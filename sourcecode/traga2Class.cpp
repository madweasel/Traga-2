/*********************************************************************
	traga2Class.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "traga2Class.h"

//-----------------------------------------------------------------------------
// Name: fieldState()
// Desc: fieldState class constructor
//-----------------------------------------------------------------------------
traga2Class::fieldState::fieldState(uintStonePos fieldSizeX, uintStonePos fieldSizeY)
{
	stoneType			.resize(fieldSizeX * fieldSizeY				);		
	stoneId				.resize(fieldSizeX * fieldSizeY				);				
	stonePos			.resize(fieldSizeX * fieldSizeY				);					// really needed: [numStones];			
	freeSquareIndex		.resize(fieldSizeX * fieldSizeY				);					// really needed: [numFreeSquares];	
	restingStones		.resize(fieldSizeX * fieldSizeY + ST_NUM_DEF);					// really needed: [numStoneTypes];
	numStonesSet		= 0;

	for (uintStonePos i=0; i<fieldSizeX*fieldSizeY; i++) {
		stoneType			[i]	= (uintStoneType) ST_FREESQUARE;
		stoneId				[i]	= (uintStoneId)   i;
		stonePos			[i]	= (uintStonePos)  i;
		restingStones		[i]	= 0;
		freeSquareIndex		[i]	= 0;
	}
}

//-----------------------------------------------------------------------------
// Name: ~fieldState()
// Desc: fieldState class destructor
//-----------------------------------------------------------------------------
traga2Class::fieldState::~fieldState()
{
	stoneType			.clear();
	stoneId				.clear();
	stonePos			.clear();
	freeSquareIndex		.clear();
	restingStones		.clear();
}

//-----------------------------------------------------------------------------
// Name: fieldStateListClass()
// Desc: fieldStateListClass class constructor
//-----------------------------------------------------------------------------
traga2Class::fieldStateListClass::fieldStateListClass()
{
	knotState			= KS_VIRGIN;
	index				= 0;
	cost				= 0;
	predecessor			= NULL;
	successor			= NULL;
	state				= NULL;
	connectedStates		= NULL;
	shortestWayOrigin	= NULL;
}

//-----------------------------------------------------------------------------
// Name: ~fieldStateListClass()
// Desc: fieldStateListClass class destructor
//-----------------------------------------------------------------------------
traga2Class::fieldStateListClass::~fieldStateListClass()
{
	if (state !=NULL) delete state;		state = nullptr;
}

//-----------------------------------------------------------------------------
// Name: traga2Class()
// Desc: traga2Class class constructor
//-----------------------------------------------------------------------------
traga2Class::traga2Class(uintStonePos sizeX, uintStonePos sizeY)
{
	// copy parameters
	isFinalState		= false;
	fieldSizeX			= sizeX;
	fieldSizeY			= sizeY;
	numSquaresInField	= sizeX * sizeY;
	curState			= new fieldState(fieldSizeX, fieldSizeY);
	targetState			= new fieldState(fieldSizeX, fieldSizeY);
	stoneTypes			.resize(numSquaresInField + ST_NUM_DEF										, {0, 0});		// really needed: [numStoneTypes];	
	numStonesOfThisType	.resize(numSquaresInField + ST_NUM_DEF										, 0);		// really needed: [numStoneTypes];	
	stones				.resize(numSquaresInField													, {0, 0});		// really needed: [numStones];
	stoneTypeToId		.resize((numSquaresInField + ST_NUM_DEF)*(numSquaresInField + ST_NUM_DEF)	, 0);
	
	removeAllStones();
}

//-----------------------------------------------------------------------------
// Name: ~traga2Class()
// Desc: traga2Class class destructor
//-----------------------------------------------------------------------------
traga2Class::~traga2Class()
{
	stoneTypes			.clear(); 
	stoneTypeToId		.clear(); 
	numStonesOfThisType	.clear(); 
	stones				.clear(); 
	if (curState			!= NULL ) { delete    curState;					curState			= NULL;  } 
	if (targetState			!= NULL ) { delete    targetState;				targetState			= NULL;  } 
}

//-----------------------------------------------------------------------------
// Name: loadDefaultState()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::loadDefaultState()
{
	fieldSizeX			= 4;
	fieldSizeY			= 5;
	numSquaresInField	= fieldSizeX * fieldSizeY;
	numStones			= 12;
	numFreeSquares		= 2;
	numStoneTypes		= 6;

	uintStoneType ST_VERTICAL	= 2;
	uintStoneType ST_BIG_ONE	= 3;
	uintStoneType ST_SMALL_ONE	= 4;
	uintStoneType ST_HORIZONTAL	= 5;

	stoneTypes[ST_FREESQUARE].width	= 1;	stoneTypes[ST_FREESQUARE].height	= 1;		
	stoneTypes[ST_FIXEDSTONE].width	= 1;	stoneTypes[ST_FIXEDSTONE].height	= 1;		
	stoneTypes[ST_VERTICAL	].width	= 1;	stoneTypes[ST_VERTICAL	].height	= 2;		
	stoneTypes[ST_BIG_ONE	].width	= 2;	stoneTypes[ST_BIG_ONE	].height	= 2;		
	stoneTypes[ST_SMALL_ONE	].width	= 1;	stoneTypes[ST_SMALL_ONE	].height	= 1;		
	stoneTypes[ST_HORIZONTAL].width	= 2;	stoneTypes[ST_HORIZONTAL].height	= 1;		

	// set stones
	stones[ 0] = stoneTypes[ST_VERTICAL];	stoneTypeToId[ST_VERTICAL	 * numStones + (uintStoneId) 0]     = 0;
	stones[ 1] = stoneTypes[ST_BIG_ONE];	stoneTypeToId[ST_BIG_ONE	 * numStones + (uintStoneId) 0]		= 1;
	stones[ 2] = stoneTypes[ST_VERTICAL];	stoneTypeToId[ST_VERTICAL	 * numStones + (uintStoneId) 1]		= 2;
	stones[ 3] = stoneTypes[ST_VERTICAL];	stoneTypeToId[ST_VERTICAL	 * numStones + (uintStoneId) 2]		= 3;
	stones[ 4] = stoneTypes[ST_HORIZONTAL];	stoneTypeToId[ST_HORIZONTAL	 * numStones + (uintStoneId) 0]		= 4;
	stones[ 5] = stoneTypes[ST_VERTICAL];	stoneTypeToId[ST_VERTICAL	 * numStones + (uintStoneId) 3]		= 5;
	stones[ 6] = stoneTypes[ST_SMALL_ONE];	stoneTypeToId[ST_SMALL_ONE	 * numStones + (uintStoneId) 0]		= 6;
	stones[ 7] = stoneTypes[ST_SMALL_ONE];	stoneTypeToId[ST_SMALL_ONE	 * numStones + (uintStoneId) 1]		= 7;
	stones[ 8] = stoneTypes[ST_SMALL_ONE];	stoneTypeToId[ST_SMALL_ONE	 * numStones + (uintStoneId) 2]		= 8;
	stones[ 9] = stoneTypes[ST_SMALL_ONE];	stoneTypeToId[ST_SMALL_ONE	 * numStones + (uintStoneId) 3]		= 9;
	stones[10] = stoneTypes[ST_FREESQUARE];	stoneTypeToId[ST_FREESQUARE	 * numStones + (uintStoneId) 0]		= 10;
	stones[11] = stoneTypes[ST_FREESQUARE];	stoneTypeToId[ST_FREESQUARE	 * numStones + (uintStoneId) 1]		= 11;

	// set
	numStonesOfThisType[ST_BIG_ONE]		= 1;
	numStonesOfThisType[ST_SMALL_ONE]	= 4;
	numStonesOfThisType[ST_HORIZONTAL]	= 1;
	numStonesOfThisType[ST_VERTICAL]	= 4;
	numStonesOfThisType[ST_FREESQUARE]	= 2;
	numStonesOfThisType[ST_FIXEDSTONE]	= 0;

	// starting state
	fieldState *startingState	= new fieldState(fieldSizeX, fieldSizeY);
	startingState->stoneType[ 0]= ST_VERTICAL;	startingState->stoneType[ 1]= ST_BIG_ONE;		startingState->stoneType[ 2]= ST_BIG_ONE;		startingState->stoneType[ 3]= ST_VERTICAL;	
	startingState->stoneType[ 4]= ST_VERTICAL;	startingState->stoneType[ 5]= ST_BIG_ONE;		startingState->stoneType[ 6]= ST_BIG_ONE;		startingState->stoneType[ 7]= ST_VERTICAL;	
	startingState->stoneType[ 8]= ST_VERTICAL;	startingState->stoneType[ 9]= ST_HORIZONTAL;	startingState->stoneType[10]= ST_HORIZONTAL;	startingState->stoneType[11]= ST_VERTICAL;	
	startingState->stoneType[12]= ST_VERTICAL;	startingState->stoneType[13]= ST_SMALL_ONE;		startingState->stoneType[14]= ST_SMALL_ONE;		startingState->stoneType[15]= ST_VERTICAL;	
	startingState->stoneType[16]= ST_SMALL_ONE;	startingState->stoneType[17]= ST_FREESQUARE;	startingState->stoneType[18]= ST_FREESQUARE;	startingState->stoneType[19]= ST_SMALL_ONE;	

	startingState->stoneId[ 0]= 0;	startingState->stoneId[ 1]= 1;	startingState->stoneId[ 2]= 1;	startingState->stoneId[ 3]= 3;	
	startingState->stoneId[ 4]= 0;	startingState->stoneId[ 5]= 1;	startingState->stoneId[ 6]= 1;	startingState->stoneId[ 7]= 3;	
	startingState->stoneId[ 8]= 3;	startingState->stoneId[ 9]= 4;	startingState->stoneId[10]= 4;	startingState->stoneId[11]= 5;	
	startingState->stoneId[12]= 3;	startingState->stoneId[13]= 6;	startingState->stoneId[14]= 7;	startingState->stoneId[15]= 5;	
	startingState->stoneId[16]= 8;	startingState->stoneId[17]= 10;	startingState->stoneId[18]= 11;	startingState->stoneId[19]= 9;	

	startingState->stonePos[ 0] = 0;
	startingState->stonePos[ 1] = 1;
	startingState->stonePos[ 2] = 3;
	startingState->stonePos[ 3] = 8;
	startingState->stonePos[ 4] = 9;
	startingState->stonePos[ 5] = 11;
	startingState->stonePos[ 6] = 13;
	startingState->stonePos[ 7] = 14;
	startingState->stonePos[ 8] = 16;
	startingState->stonePos[ 9] = 19;
	startingState->stonePos[10] = 17;
	startingState->stonePos[11] = 18;

	updateFreeSquareIndex(startingState);

	delete curState;
	curState = startingState;

	return true;
}

//-----------------------------------------------------------------------------
// Name: prepareCurAndTargetState()
// Desc:
//-----------------------------------------------------------------------------
bool traga2Class::prepareCurAndTargetState(fieldState *theTargetState)
{
	// locals
	uintStoneType	oldType, newType;
	uintStonePos	curPosInTargetState, curPosInCurState, x, y, fieldIndex;
	
	// copy target state
	theTargetState->copyFieldTo(targetState, numSquaresInField);
	
	// each stone in theTargetState shall get an own stoneType
	for (curPosInTargetState=0; curPosInTargetState<numSquaresInField; curPosInTargetState++) {

		// if the type of a stone in the target state is not unique than assign it a new stone type
		if (targetState->stoneType[curPosInTargetState] != ST_FREESQUARE
		&&	targetState->stoneType[curPosInTargetState] != ST_FIXEDSTONE) {
			if (numStonesOfThisType[targetState->stoneType[curPosInTargetState]] > 1) {

				oldType							= targetState->stoneType[curPosInTargetState];
				newType							= numStoneTypes;
				numStonesOfThisType[newType]	= 1;
				stoneTypes[newType]				= stoneTypes[oldType];
				curPosInCurState				= curState->stonePos[targetState->stoneId[curPosInTargetState]];
				numStonesOfThisType[oldType]--;
				numStoneTypes++;
				
				for (x=0; x<stoneTypes[oldType].width; x++) {
					for (y=0; y<stoneTypes[oldType].height; y++) {
						fieldIndex							= curPosInTargetState + y * fieldSizeX + x;
						targetState->stoneType[fieldIndex]	= newType;
						fieldIndex							= curPosInCurState + y * fieldSizeX + x;
						curState->stoneType[fieldIndex]		= newType;
					}
				}				
			}
		}
	}

	return true;
}
	
//-----------------------------------------------------------------------------
// Name: findAllStates()
// Desc: Returns all possible states (with doubles).
//-----------------------------------------------------------------------------
bool traga2Class::findAllStates()
{
	// locals
	uintStoneType	curType;
	uintStonePos	curPos;
	fieldState		*emptyField = new fieldState(fieldSizeX, fieldSizeY);

	// init
	emptyField->numStonesSet	= 0;
	for (curType=0; curType<numStoneTypes; curType++) {
		emptyField->restingStones[curType]	= numStonesOfThisType[curType];
	}

	// handle fixed stones
	for (curPos=0; curPos<numSquaresInField; curPos++) {

		// preset fixed stones
		if (targetState->stoneType[curPos] == uintStoneType(ST_FIXEDSTONE)) {
			emptyField->stoneId[curPos]			= targetState->stoneId[curPos];
			emptyField->stoneType[curPos]		= ST_FIXEDSTONE;
			emptyField->numStonesSet++;
		
		// all other squares shall be unused
		} else {
			emptyField->stoneId[curPos]			= numStones;
			emptyField->stoneType[curPos]		= numStoneTypes;
		}
	}

	// 
	fieldStateListClass* curStateListItem = firstStateListItem;
	stateListMap.clear();

	// try first square
	tryStone(0, emptyField, curStateListItem);

	// no states found
	if (lastStateListItem == nullptr) return false;

	// finish list and quit
	lastStateListItem				= lastStateListItem->predecessor;
	lastStateListItem->successor	= nullptr;
	delete emptyField;	
	emptyField = nullptr;
	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::fieldState::copyFieldTo()
// Desc: 
//-----------------------------------------------------------------------------
void traga2Class::fieldState::copyFieldTo(fieldState *to, uintStonePos fieldSize)
{
	to->stoneType			= this->stoneType;
	to->stoneId				= this->stoneId;
	to->stonePos			= this->stonePos;
	to->freeSquareIndex		= this->freeSquareIndex;
	to->restingStones		= this->restingStones;
	to->numStonesSet		= this->numStonesSet;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::tryStone()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::tryStone(uintStonePos posIndex, fieldState *field, fieldStateListClass* &curStateListItem)
{
	// locals
	bool				squaresIsOccupied;
	uintStonePos		x, y;
	uintStoneType		stoneType;
	uintStoneId			stoneId;
	uintStonePos		posX			= posIndex % fieldSizeX;
	uintStonePos		posY			= posIndex / fieldSizeX;
	fieldStateListClass	*nextListItem;

	// posIndex ok ?
	if (posIndex >= numSquaresInField) {
		return false;
	}

	// is square free ?
	if (field->stoneType[posIndex] >= numStoneTypes) {
	
		// try to put each stone type
		for (stoneType = 0; stoneType < numStoneTypes; stoneType++) {

			// skip fixed stone type
			if (stoneType == ST_FIXEDSTONE) continue;

			// a stone of this type available ?
			if (field->restingStones[stoneType]) {

				// does stone fit ?
				if ((posX + stoneTypes[stoneType].width  <= fieldSizeX) 
				 && (posY + stoneTypes[stoneType].height <= fieldSizeY)) {

					 // square free ?
					 squaresIsOccupied = false;
					 for (x=0; x<stoneTypes[stoneType].width; x++) { for (y=0; y<stoneTypes[stoneType].height; y++) {
						 if (field->stoneType[posIndex + y * fieldSizeX + x] < numStoneTypes) {
							 squaresIsOccupied = true;
					 }}}
					 
					 if (!squaresIsOccupied) {

						 // set stone
						 stoneId = stoneTypeToId[stoneType * numSquaresInField + (uintStoneId) field->restingStones[stoneType] - (uintStoneId) 1];
						 field->numStonesSet++;
						 field->restingStones[stoneType]--;
						 for (x=0; x<stoneTypes[stoneType].width;  x++) { for (y=0; y<stoneTypes[stoneType].height; y++) {
					 		field->stoneType[posIndex + y * fieldSizeX + x]				= stoneType;
							field->stoneId  [posIndex + y * fieldSizeX + x]				= stoneId;
						 }}
						 field->stonePos[stoneId]										= posIndex;
						 if (stoneType == ST_FREESQUARE)  {
							 field->freeSquareIndex[(uintStonePos) field->restingStones[stoneType]]	= posIndex;
						 }

						 // target achieved ?
						 if ((uint) field->numStonesSet == numStones) {

							// save current field in list
							curStateListItem->state = new fieldState(fieldSizeX, fieldSizeY);
							field->copyFieldTo(curStateListItem->state, numSquaresInField);
						
							stateListMap.insert(pair<arrayStoneType_StonePos, fieldStateListClass*>(field->stoneType, curStateListItem));
							
							// create new list item
							nextListItem					= new fieldStateListClass();
							nextListItem->index				= curStateListItem->index + 1;
							nextListItem->predecessor		= curStateListItem;
							curStateListItem->successor		= nextListItem;
							curStateListItem				= nextListItem;
							lastStateListItem				= curStateListItem;
							
							// show progress
							if (showProgress && (stateListMap.size() % 1000) == 0) {
								if (!showProgress(stateListMap.size(), 0, L"Finding all states...")) {
									return false;
								}
							}

						 // try next Stone
						 } else {
							 tryStone(posIndex + (uintStonePos) 1, field, curStateListItem);
						 }

						 // remove stone
						 if (stoneType == ST_FREESQUARE) {
							 field->freeSquareIndex[(uintStonePos) field->restingStones[stoneType]]	= numSquaresInField;
						 }
						 field->numStonesSet--;
						 field->restingStones[stoneType]++;
						 for (x=0; x<stoneTypes[stoneType].width;  x++) { for (y=0; y<stoneTypes[stoneType].height; y++) {
					 		field->stoneType[posIndex + y * fieldSizeX + x]				= numStoneTypes;
							field->stoneId  [posIndex + y * fieldSizeX + x]				= numStones;
						 }}
						 field->stonePos[stoneId]										= numSquaresInField;

					 }
				}
			}
		}
	// try next field
	} else {
		tryStone(posIndex + (uintStonePos) 1, field, curStateListItem);
	}

	// everything is ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::calcConnections()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::calcConnections()
{
	// locals
	arrayUInt_StonePos		skipHoleTo			(numFreeSquares);
	arrayUInt_StonePos		skipHoleFrom		(numFreeSquares);
	arrayUInt_StonePos		toHoleHasPartner	(numFreeSquares);
	arrayUInt_StonePos		fromHoleHasPartner	(numFreeSquares);
	unsigned int			numItemsInList			= lastStateListItem->index + 1;
	fieldStateListClass		*from, *to, *tmp;
	unsigned int			numConnections			= 0;
	fieldState				myState(fieldSizeX, fieldSizeY);

	// test each possible connection
	for (from=firstStateListItem; from != nullptr && from->index < numItemsInList; from = from->successor) { 

		// show progress
		if (showProgress && (from->index % 100) == 0) {
			if (!showProgress((int) numItemsInList, (int) from->index, L"Calc connections...")) {
				return false;
			}
		}

		// try to move each stone
		uintStoneId curStoneId;
		for (curStoneId=0; curStoneId<numStones; curStoneId++) {

			// try to move the considered stone into each direction
			uintDirection curDirection;
			for (curDirection=0; curDirection<MV_NUM_DIRECTIONS; curDirection++) {

				// move possible?
				if (isMoveAllowed(from->state, curStoneId, curDirection)) {

					// consider current state
					from->state->copyFieldTo(&myState, fieldSizeX*fieldSizeY);
					moveStone(&myState, curStoneId, curDirection, false);
					to = findStateInList(&myState);
					if (to == nullptr) {
						continue;	// ... must not happen !
					}

					// move is possible
					numConnections++;
			
					// from
					tmp											= new fieldStateListClass();
					tmp->connectedStates						= to;
					if (from->connectedStates != NULL) {
						from->connectedStates->successor		= tmp;
						tmp->index								= from->connectedStates->index + 1;
						tmp->predecessor						= from->connectedStates;
					}
					from->connectedStates						= tmp;

					// to
					tmp											= new fieldStateListClass();
					tmp->connectedStates						= from;
					if (to->connectedStates != NULL) {
						to->connectedStates->successor			= tmp;
						tmp->index								= to->connectedStates->index + 1;
						tmp->predecessor						= to->connectedStates;
					}
					to->connectedStates							= tmp;		
				}
			}
		}
	}

	// go back to first list item
	for (from=firstStateListItem; from != NULL; from = from->successor) { 
		for (; from->connectedStates != NULL && from->connectedStates->predecessor != NULL; from->connectedStates = from->connectedStates->predecessor) { }
	}

	// show progress
	showProgress(numItemsInList, numItemsInList, L"Connections calculated");

	// everything is ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::findStateInList()
// Desc: 
//-----------------------------------------------------------------------------
traga2Class::fieldStateListClass *traga2Class::findStateInList(fieldState *state)
{
	auto it = stateListMap.find(state->stoneType);

	if (it != stateListMap.end()) {
		return it->second;
	} else {
		return nullptr;
	}
}

//-----------------------------------------------------------------------------
// Name: traga2Class::isTarget()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::isTarget(fieldState *state)
{
	return state->isTarget(*targetState, fieldSizeX*fieldSizeY);
}

//-----------------------------------------------------------------------------
// Name: traga2Class::fieldState::hasEqualStoneType()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::fieldState::hasEqualStoneType(fieldState& compareState, unsigned int fieldSize)
{
	uintStonePos		curPos;

	for (curPos=0; curPos<fieldSize; curPos++) {
		if (compareState.stoneType[curPos] != stoneType[curPos]) {
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::fieldState::hasEqualStoneId()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::fieldState::hasEqualStoneId(fieldState& compareState, unsigned int fieldSize)
{
	uintStonePos		curPos;

	for (curPos=0; curPos<fieldSize; curPos++) {
		if (compareState.stoneId[curPos] != stoneId[curPos]) {
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// Name: traga2Class::isTarget()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::fieldState::isTarget(fieldState& targetState, unsigned int fieldSize)
{
	// locals
	uintStonePos position;

	for (position=0; position<fieldSize; position++) {
		if (targetState.stoneType[position] != ST_FREESQUARE &&  targetState.stoneType[position] != ST_FIXEDSTONE) {
			if (stoneId[position] != targetState.stoneId[position]) {
				break;
			}
		}
	}

	// are all target stones on their final position?
	if (position == fieldSize) {
		return true;
	} else {
		return false;
	}
}

//-----------------------------------------------------------------------------
// Name: traga2Class::countUnsolvedNeighbours()
// Desc: 
//-----------------------------------------------------------------------------
unsigned int traga2Class::countUnsolvedNeighbours(uintStonePos targetPos, bool positiveDirection, arrayStoneType_StonePos stoneType, bool horizontalNotVertical)
{
	// locals
	int			 curCoord;
	uintStonePos curPos;
	uintStonePos targetCoord;
	unsigned int numUnsolvedStones;
	uintStonePos fieldSize;

	if (horizontalNotVertical) {
		fieldSize	= fieldSizeX;
		targetCoord = getCoordX(targetPos);
	} else {
		fieldSize	= fieldSizeY;
		targetCoord = getCoordY(targetPos);
	}

	for (numUnsolvedStones=1; numUnsolvedStones < (uint) fieldSize; numUnsolvedStones++) {

		curCoord = (int) ((uint) targetCoord) + (positiveDirection?1:-1) * numUnsolvedStones; 
		
		// outside the field?
		if (curCoord <  0 || curCoord >= (int) ((uint) fieldSize))	break;

		// fixed stone?
		if (horizontalNotVertical) {
			curPos = getPosition(curCoord, getCoordY(targetPos));
		} else {
			curPos = getPosition(getCoordX(targetPos), curCoord);
		}
		if (stoneType[curPos] == ST_FIXEDSTONE) break;
	}

	return numUnsolvedStones - 1;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::oneTimesOneAlgorithmn()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::oneTimesOneAlgorithmn(traga2Class *target, uintStepId &numSteps, arrayStonePos_StepId &stonePos, arrayDirection_StepId &direction)
{
	// locals
	uintStonePos			targetPos[2];							// current position of considered stone
	uintStoneType			targetStoneType[2];						// current type of considered stone
	unsigned int			numStonesToSolve;						// current number of stones to be solved at once
	uintStonePos			posOfFreeSquare;						// position of a free square
	solutionPathListClass	solPath;								// solution
	uintStepId				curStep;								// counter vars
	uintStonePos			curPos;									//   ''
	unsigned int 			curStone;								//   ''
	bool					everythingOk		= true;				// return value
	traga2Class	*			myInitialState;							// own object for calculation of sub problems
	traga2Class	*			myFinalState;							// ''
	uintStepId				curSolNumSteps;							// solution of currently calculated sub problem
	arrayStonePos_StepId	curSolStonePos;							// ''
	arrayDirection_StepId	curSolDirection;						// ''
	float					maxDistance, curDistance;				// 
	unsigned int			stonesSolved		= 0;				// number of stones already solved
	unsigned int			left, right, up, down;					//

	// init temperary classes
	myInitialState = new traga2Class(fieldSizeX, fieldSizeY);
	myFinalState   = new traga2Class(fieldSizeX, fieldSizeY);
	myInitialState->copyFieldState(this);

	// search free square
	for (posOfFreeSquare=0; posOfFreeSquare<numSquaresInField; posOfFreeSquare++) {
		if (target->curState->stoneType[posOfFreeSquare] == ST_FREESQUARE) break;
	}
	
	// use current showProgress pointer
	myInitialState->setShowProgressFunc(this->showProgress);

	// process each stone in target state
	do {

		// calc distance to target position of free square
		for (maxDistance=0.0, curPos=0; curPos<numSquaresInField; curPos++) {
			curDistance = sqrt( (float) square((int) (uint) getCoordX(curPos) - (int) (uint) getCoordX(posOfFreeSquare)) + 
								(float) square((int) (uint) getCoordY(curPos) - (int) (uint) getCoordY(posOfFreeSquare)));
			if (maxDistance < curDistance && myInitialState->curState->stoneType[curPos] != ST_FIXEDSTONE) {
				maxDistance		= curDistance;
				targetPos[0]	= curPos;
			}
		}

		// if only one unsolved neighbour stone in this row/column than solve both stones
		left	= countUnsolvedNeighbours(targetPos[0], false, myInitialState->curState->stoneType, true);
		right	= countUnsolvedNeighbours(targetPos[0],  true, myInitialState->curState->stoneType, true);
		up		= countUnsolvedNeighbours(targetPos[0], false, myInitialState->curState->stoneType, false);
		down	= countUnsolvedNeighbours(targetPos[0],  true, myInitialState->curState->stoneType, false);

		if (left + right <= 1) {
			numStonesToSolve	= 2;
			targetPos[1]		= (right>0)?targetPos[0]+(uintStonePos)1:targetPos[0]-(uintStonePos)1;
		} else if (up + down <= 1) {
			numStonesToSolve	= 2;
			targetPos[1]		= (down>0)?targetPos[0]+fieldSizeX:targetPos[0]-fieldSizeX;
		} else {
			numStonesToSolve = 1;
		}

		for (curStone=0; curStone<numStonesToSolve; curStone++) {
			targetStoneType[curStone] = target->getStoneType(targetPos[curStone]);

			// should not happen
			if (targetStoneType[curStone] == ST_FIXEDSTONE) 
				return false;

			// skip free squares
			if (targetStoneType[curStone] == ST_FREESQUARE) 
				continue;

			// add stone
			myFinalState->addStone(myInitialState, target->getStoneId(targetPos[curStone]), targetPos[curStone]);
		}

		// solve sub problem
		everythingOk = myInitialState->solveProblem(myFinalState, curSolNumSteps, curSolStonePos, curSolDirection);

		// no solution found, so quit
		if (everythingOk) { 

			// perform moves on own copy of field state
			for (curStep=0; curStep<curSolNumSteps; curStep++) {
				if (!moveStone(myInitialState->curState, myInitialState->curState->stoneId[curSolStonePos[curStep]], curSolDirection[curStep], true)) return false;
			}
			solPath.addStepsAndDeleteArrays(curSolNumSteps, curSolStonePos, curSolDirection);

			// set solved stone as fixed
			for (curStone=0; curStone<numStonesToSolve; curStone++) {
				myInitialState->numStonesOfThisType[myInitialState->curState->stoneType[targetPos[curStone]]]--;
				myInitialState->numStoneTypes--;

				// should not happen
				if (myInitialState->numStonesOfThisType[myInitialState->curState->stoneType[targetPos[curStone]]] != 0) 
					return false;

				myInitialState->curState->stoneType[targetPos[curStone]] = ST_FIXEDSTONE;
				myInitialState->numStonesOfThisType[ST_FIXEDSTONE]++;
				myFinalState  ->copyFixedStones(myInitialState);
			}

			// update curState->freeSquareIndex[]
			updateFreeSquareIndex(myInitialState->curState);

			// next stone
			stonesSolved += numStonesToSolve;
		}

	} while (everythingOk && stonesSolved < (uint) numSquaresInField);

	// extract accumulated solution steps
	solPath.extract(numSteps, stonePos, direction);

	// delete temporary classes
	delete myInitialState;	myInitialState	= nullptr;
	delete myFinalState;	myFinalState	= nullptr;
	return everythingOk;
}

/*-----------------------------------------------------------------------------
// Name: traga2Class::oneTimesOneAlgorithmn()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::oneTimesOneAlgorithmn(traga2Class *target, unsigned int &numSteps, unsigned int *&stonePos, unsigned int *&direction)
{
	// locals
	unsigned int			targetX, targetY;
	fieldState 				myCurState(fieldSizeX, fieldSizeY);
	solutionPathListClass	solPath;

	// make my own instance of the field state
	copyField(curState, &myCurState);

	// process each stone in target state, beginning in the upper left corner
	for (targetY=0; targetY<fieldSizeY-2; targetY++) {
		for (targetX=0; targetX<fieldSizeX; targetX++) {
			if (!solveSingleStone(target, targetX, targetY, myCurState, solPath, false)) return false;
		}
	}

	// calc last two rows
	for (targetX=0; targetX<fieldSizeX; targetX++) {
		for (targetY=fieldSizeY-2; targetY<fieldSizeY; targetY++) {
			if (!solveSingleStone(target, targetX, targetY, myCurState, solPath, true)) return false;
		}
	}

	// extract accumulated solution steps
	solPath.extract(numSteps, stonePos, direction);

	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::solveSingleStone()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::solveSingleStone(traga2Class *target, unsigned int targetX, unsigned int targetY, fieldState &myCurState, solutionPathListClass &solPath, bool solvingLastTwoRows)
{
	// locals
	unsigned int	fromPos, toPos;												// considered rect for subproblem
	unsigned int	targetPos;													// current considered position in the target state
	unsigned int	initialX, initialY, initialPos;								// current position of the stone, which is going to be solved next
	unsigned int    freeSquarePos;												// position of a free square
	unsigned int	curSolNumSteps, *curSolStonePos, *curSolDirection;			// solution of currently calculated sub problem
	unsigned int	curStep;													// 
	unsigned int	numStonesInTarget, targetStoneId[2], targetDestPosition[2];	// passed variables for function solveSubProblem()

	// skip if it's a free square
	targetPos = getPosition(targetX, targetY);
	if (target->getStoneType(targetPos) == ST_FREESQUARE) return true;
	if (target->getStoneType(targetPos) == ST_FIXEDSTONE) return true;

	// search target stone in initial state
	initialPos	= myCurState.stonePos[target->getStoneId(targetPos)];
	initialX	= getCoordX(initialPos);
	initialY	= getCoordY(initialPos);

	// skip if stone is already on the final position
	if (initialPos == targetPos) return true;

	// move each free square down which has the y-coordinate equal targetY
	if (!solvingLastTwoRows) {
		for (freeSquarePos=targetPos; freeSquarePos<targetPos + (fieldSizeX - targetX); freeSquarePos++) { 
			if (myCurState.stoneType[freeSquarePos] == ST_FREESQUARE) {
				if (!moveStone(&myCurState, myCurState.stoneId[freeSquarePos+fieldSizeX], MV_UP, true)) return false;
				solPath.addStep(freeSquarePos+fieldSizeX, MV_UP);
			}
		}

		// search target stone in initial state
		initialPos	= myCurState.stonePos[target->getStoneId(targetPos)];
		initialX	= getCoordX(initialPos);
		initialY	= getCoordY(initialPos);

		// skip if stone is already on the final position
		if (initialPos == targetPos) return true;

		// move the considered stone inside the rect spanned from targetPos to the lower right corner
		if (initialX < targetX) {
			numStonesInTarget		= 1;
			targetStoneId[0]		= target->getStoneId(targetPos);
			targetDestPosition[0]	= getPosition(targetX, initialY);
			fromPos					= getPosition(0, targetY+1);
			toPos					= numSquaresInField-1;
			if (!solveSubProblem(&myCurState, numStonesInTarget, targetStoneId, targetDestPosition, fromPos, toPos, curSolNumSteps, curSolStonePos, curSolDirection)) return false;
			
			// perform moves on own copy of field state
			for (curStep=0; curStep<curSolNumSteps; curStep++) {
				if (!moveStone(&myCurState, myCurState.stoneId[curSolStonePos[curStep]], curSolDirection[curStep], true)) return false;
			}
			solPath.addStepsAndDeleteArrays(curSolNumSteps, curSolStonePos, curSolDirection);
		} 

		// search target stone in initial state
		initialPos	= myCurState.stonePos[target->getStoneId(targetPos)];
		initialX	= getCoordX(initialPos);
		initialY	= getCoordY(initialPos);

		// skip if stone is already on the final position
		if (initialPos == targetPos) return true;

		// move free squares inside the rect spanned from targetPos to the lower right corner
		for (freeSquarePos=((targetX != fieldSizeX-1)?targetPos:targetPos-1); freeSquarePos<numSquaresInField; freeSquarePos++) {
			if (getCoordX(freeSquarePos) < ((targetX != fieldSizeX-1)?targetX:targetX-1)) continue;
			if (myCurState.stoneType[freeSquarePos] == ST_FREESQUARE) break;
		}
		if (freeSquarePos == numSquaresInField) {

			// search a free square
			for (freeSquarePos=getPosition(0, targetY+1); freeSquarePos<numSquaresInField-1; freeSquarePos++) { 
				if (getCoordX(freeSquarePos) >= targetX) continue;
				if (myCurState.stoneType[freeSquarePos] == ST_FREESQUARE) {
					break;
				}
			}

			// move free square to other row than considered stone
			if (getCoordY(freeSquarePos) == initialY) {

				// move free suqare up
				if (getCoordY(freeSquarePos) == fieldSizeY - 1) {
					if (!moveStone(&myCurState, myCurState.stoneId[freeSquarePos - fieldSizeX], MV_DOWN, true)) return false;
					solPath.addStep(freeSquarePos - fieldSizeX, MV_DOWN);
					freeSquarePos -= fieldSizeX;
				// move free suqare down
				} else {
					if (!moveStone(&myCurState, myCurState.stoneId[freeSquarePos + fieldSizeX], MV_UP,   true)) return false;
					solPath.addStep(freeSquarePos + fieldSizeX, MV_UP);
					freeSquarePos += fieldSizeX;
				}
			}

			// move free square to the right
			do {
				if (!moveStone(&myCurState, myCurState.stoneId[freeSquarePos + 1], MV_LEFT, true)) return false;
				solPath.addStep(freeSquarePos + 1, MV_LEFT);
				freeSquarePos++;
			} while (getCoordX(freeSquarePos) < targetX);
		}
	}

	// move considered stone to target position
	if ((!solvingLastTwoRows && targetX != fieldSizeX-1) || (solvingLastTwoRows && targetY != fieldSizeY-1)) {
		numStonesInTarget		= 1;
		targetDestPosition[0]	= targetPos;
		targetStoneId[0]		= target->getStoneId(targetPos);
		fromPos					= targetPos;
		toPos					= numSquaresInField-1;
	} else {
		numStonesInTarget		= 2;
		targetDestPosition[0]	= targetPos;
		targetStoneId[0]		= target->getStoneId(targetPos);
		targetDestPosition[1]	= (!solvingLastTwoRows)?targetPos-1:targetPos-fieldSizeX;
		targetStoneId[1]		= target->getStoneId(targetDestPosition[1]);
		fromPos					= targetDestPosition[1];
		toPos					= numSquaresInField-1;
	}
	if (fromPos != numSquaresInField - 1 - fieldSizeX) {
		if (!solveSubProblem(&myCurState, numStonesInTarget, targetStoneId, targetDestPosition, fromPos, toPos, curSolNumSteps, curSolStonePos, curSolDirection)) return false;
	} else if (initialPos != targetPos) {
		curSolNumSteps		= 1;
		curSolStonePos		= new unsigned int[1];
		curSolDirection		= new unsigned int[1];
		curSolStonePos[0]	= initialPos;
		curSolDirection[0]	= MV_UP;
	}
	
	// perform moves on own copy of field state
	for (curStep=0; curStep<curSolNumSteps; curStep++) {
		if (!moveStone(&myCurState, myCurState.stoneId[curSolStonePos[curStep]], curSolDirection[curStep], true)) return false;
	}
	solPath.addStepsAndDeleteArrays(curSolNumSteps, curSolStonePos, curSolDirection);

	// everything was fine
	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::solveSubProblem()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::solveSubProblem(fieldState *stateToConsider, unsigned int numStonesInTarget, unsigned int targetStoneId[], unsigned int targetDestPosition[], unsigned int fromPos, unsigned int toPos, unsigned int &numSteps, unsigned int *&stonePos, unsigned int *&direction)
{
	// locals
	unsigned int	curStep;								// 
	unsigned int	curPos, curStoneType, curStone;			//
	unsigned int	curSubX, curSubY;						//	
	unsigned int	curSubStoneId, destSubPosition;			//
	bool			everythingOk		= true;				//
	traga2Class	*	myInitialState;							//
	traga2Class	*	myFinalState;							//
	unsigned int	*curSolStonePos;						//

	// swap if necessary
	if (fromPos > toPos) {
		curPos	= toPos;
		toPos	= fromPos;
		fromPos = curPos;
	} else if (fromPos == toPos) {
		return false;
	}

	// init temperary classes
	myInitialState = new traga2Class(getCoordX(toPos) - getCoordX(fromPos) + 1, getCoordY(toPos) - getCoordY(fromPos) + 1);
	myFinalState   = new traga2Class(getCoordX(toPos) - getCoordX(fromPos) + 1, getCoordY(toPos) - getCoordY(fromPos) + 1);

	// copy initial state
	for (curStone=0;curStone<numStones; curStone++) {

		// translate to sub-problem-position
		curPos			= stateToConsider->stonePos[curStone];
		curStoneType	= stateToConsider->stoneType[curPos];
		curSubX			= getCoordX(curPos) - getCoordX(fromPos);
		curSubY			= getCoordY(curPos) - getCoordY(fromPos);

		// skip if it's a free square
		if (curStoneType == ST_FREESQUARE) continue;

		// skip if stone position is outside
		if (getCoordX(curPos) < getCoordX(fromPos) || getCoordX(curPos) + stoneTypes[curStoneType].width  - 1 > getCoordX(toPos)) continue;
		if (getCoordY(curPos) < getCoordY(fromPos) || getCoordY(curPos) + stoneTypes[curStoneType].height - 1 > getCoordY(toPos)) continue;

		// add stone
		myInitialState->addStone(myInitialState->getPosition(curSubX, curSubY), stoneTypes[curStoneType].width, stoneTypes[curStoneType].height, false);
	}

	// prepare final state
	for (curStone=0;curStone<numStonesInTarget; curStone++) {

		// calc curSubStoneId
		curSubX			= getCoordX(stateToConsider->stonePos[targetStoneId[curStone]]) - getCoordX(fromPos);
		curSubY			= getCoordY(stateToConsider->stonePos[targetStoneId[curStone]]) - getCoordY(fromPos);
		curSubStoneId	= myInitialState->getStoneId(myInitialState->getPosition(curSubX, curSubY));

		// calc destSubPosition
		curSubX			= getCoordX(targetDestPosition[curStone]) - getCoordX(fromPos);
		curSubY			= getCoordY(targetDestPosition[curStone]) - getCoordY(fromPos);
		destSubPosition	= myInitialState->getPosition(curSubX, curSubY);

		// add stone
		myFinalState->addStone(myInitialState, curSubStoneId, destSubPosition);
	}

	// solve sub problem
	everythingOk = myInitialState->solveProblem(myFinalState, numSteps, curSolStonePos, direction);

	// translate movements
	if (everythingOk) {
		stonePos	= new unsigned int[numSteps];
		for (curStep=0; curStep<numSteps; curStep++) {
			stonePos[curStep] = getPosition(myInitialState->getCoordX(curSolStonePos[curStep]) + getCoordX(fromPos), myInitialState->getCoordY(curSolStonePos[curStep]) + getCoordY(fromPos));
		}
	} else {
		numSteps = 0;
	}

	// delete temporary classes
	if (numSteps) delete [] curSolStonePos;
	delete myInitialState;
	delete myFinalState;
	return everythingOk;
}*/

//-----------------------------------------------------------------------------
// Name: traga2Class::solveProblem()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::solveProblem(traga2Class *target, uintStepId &numSteps, arrayStonePos_StepId &stonePos, arrayDirection_StepId &direction)
{
	// locals
	uintStoneId			curStone;
	uintStonePos		curPos;
	uintStonePos		numFixedStones;
	bool				allStonesOf1x1Type	= true;
	bool				everythingOk		= true;
	fieldStateListClass	*fromState, *tmpState;

	// parameters ok?
	if (target == NULL)							return false;
	if (!target->isFinalState)					return false;
	if (target->fieldSizeX != this->fieldSizeX) return false;
	if (target->fieldSizeY != this->fieldSizeY) return false;

	// check if fixed stones are same on target and initial state
	for (numFixedStones=0, curPos=0; curPos<numSquaresInField; curPos++) {
		if (this->curState->stoneType[curPos] == ST_FIXEDSTONE && target->curState->stoneType[curPos] != ST_FIXEDSTONE
		||  this->curState->stoneType[curPos] != ST_FIXEDSTONE && target->curState->stoneType[curPos] == ST_FIXEDSTONE) {
			return false;
		}
		if (this->curState->stoneType[curPos] == ST_FIXEDSTONE) numFixedStones++;
	}

	// check if all stones are of 1x1-type
	for (curStone=0; curStone<this->numStones; curStone++) {
		if (stones[curStone].height != (uintStonePos) 1 || stones[curStone].width != (uintStonePos) 1) {
			allStonesOf1x1Type = false;
			break;
		}
	}

	// use special algorithmn, when only 1x1-stones are used
	if ((this->fieldSizeX > (uintStonePos) 2 || this->fieldSizeY > (uintStonePos) 2) && allStonesOf1x1Type && target->numFreeSquares == (uintStonePos) 1 && (target->numSquaresInField - target->numFreeSquares - numFixedStones) > (uintStonePos) 2) {

		oneTimesOneAlgorithmn(target, numSteps, stonePos, direction);
		
	// standard algorithmn
	} else {
		// state list
		firstStateListItem						= new fieldStateListClass();
		lastStateListItem						= firstStateListItem;

		// all stones of the final stones shall each have an own stoneType
		if (!prepareCurAndTargetState(target->curState)) everythingOk = false;

		// recalc array stoneTypeToId[]
		updateStoneTypeToId(curState);

		// find all possible field states
		if (everythingOk && !findAllStates()) everythingOk = false;

		// calc connections between states
		if (everythingOk && !calcConnections()) everythingOk = false;

		// Find Starting point
		if (everythingOk && ((fromState = findStateInList(curState)) == NULL)) everythingOk = false;

		// Calculate shortest path (using Dijkstra-Algorithm) to each state 
		if (everythingOk && !findShortestWay(fromState)) everythingOk = false;

		// Calculate shortest path to final state
		if (everythingOk && !calcPath(numSteps, stonePos, direction)) everythingOk = false;

		// free mem
		fieldStateListClass *curStateListItem;
		for (curStateListItem=firstStateListItem; curStateListItem!=NULL; curStateListItem=tmpState) {
			for (fromState=curStateListItem->connectedStates; fromState!=NULL; fromState=tmpState) {
				tmpState = fromState->successor;
				delete fromState; fromState = NULL;
			}
			tmpState = curStateListItem->successor;
			delete curStateListItem; curStateListItem = NULL;
		}
	}
	
	// finished
	return everythingOk;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::getStonePosAfterMove()
// Desc: 
//-----------------------------------------------------------------------------
uintStonePos traga2Class::getStonePosAfterMove(uintDirection direction, uintStonePos position)
{
	switch ((uint) direction)
	{
	case MV_UP:		return position - fieldSizeX;
	case MV_DOWN:	return position + fieldSizeX;
	case MV_LEFT:	return position - (uintStonePos) 1;
	case MV_RIGHT:	return position + (uintStonePos) 1;
	default:		return numSquaresInField;
	};
}

//-----------------------------------------------------------------------------
// Name: traga2Class::isMoveAllowed()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::isMoveAllowed(uintStoneId stoneId, uintDirection movingDirection)
{
	return isMoveAllowed(curState, stoneId, movingDirection);
}

//-----------------------------------------------------------------------------
// Name: traga2Class::isMoveAllowed()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::isMoveAllowed(fieldState *state, uintStoneId stoneId, uintDirection movingDirection)
{
	// locals
	uintStonePos x, y, fieldIndex; 

	// current position of upper left corner of stone
	x			= state->stonePos[stoneId] % fieldSizeX;
	y			= state->stonePos[stoneId] / fieldSizeX;

	// don't move fixed stones
	if (state->stoneType[state->stonePos[stoneId]] == ST_FIXEDSTONE) return false;

	// do not move free squares
	if (state->stoneType[state->stonePos[stoneId]] == ST_FREESQUARE) return false;

	// can stone be moved?
	switch ((uint) movingDirection) {
		case MV_UP:
			if (y == (uintStonePos) 0) {
				return false;
			}
			for (x=(uintStonePos) 0; x<stones[stoneId].width; x++) {
				fieldIndex = state->stonePos[stoneId] - fieldSizeX + x;
				if (state->stoneType[fieldIndex] != ST_FREESQUARE) {
					return false;
				}
			}
			break;

		case MV_DOWN:
			if (y + stones[stoneId].height == fieldSizeY) {
				return false;
			}
			for (x=0; x<stones[stoneId].width; x++) {
				fieldIndex = state->stonePos[stoneId] + stones[stoneId].height * fieldSizeX + x;
				if (state->stoneType[fieldIndex] != ST_FREESQUARE) {
					return false;
				}
			}
			break;

		case MV_LEFT:
			if (x == (uintStonePos) 0) {
				return false;
			}
			for (y=(uintStonePos) 0; y<stones[stoneId].height; y++) {
				fieldIndex = state->stonePos[stoneId] - (uintStonePos) 1 + y * fieldSizeX;
				if (state->stoneType[fieldIndex] != ST_FREESQUARE) {
					return false;
				}
			}
			break;

		case MV_RIGHT:
			if (x + stones[stoneId].width == fieldSizeX) {
				return false;
			}
			for (y=(uintStonePos) 0; y<stones[stoneId].height; y++) {
				fieldIndex = state->stonePos[stoneId] + stones[stoneId].width + y * fieldSizeX;
				if (state->stoneType[fieldIndex] != ST_FREESQUARE) {
					return false;
				}
			}
			break;

		default: 
			return false;
	}
	
	// move is allowed
	return true;
}


//-----------------------------------------------------------------------------
// Name: traga2Class::moveStone()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::moveStone(uintStoneId stoneId, uintDirection movingDirection, bool checkMove)
{
	return moveStone(curState, stoneId, movingDirection, checkMove);
}


//-----------------------------------------------------------------------------
// Name: traga2Class::moveStone()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::moveStone(fieldState *state, uintStoneId stoneId, uintDirection movingDirection, bool checkMove)
{
	// locals
	uintStonePos	x, y, fieldIndexMovingFrom, fieldIndexMovingTo;
	uintStoneType	stoneType; 

	// current position of upper left corner of stone
	x			= state->stonePos[stoneId] % fieldSizeX;
	y			= state->stonePos[stoneId] / fieldSizeX;
	stoneType	= state->stoneType[y * fieldSizeX + x];

	// check move before?
	if (checkMove) {
		if (!isMoveAllowed(state, stoneId, movingDirection)) {
			return false;
		}
	}

	// move stone
	switch ((uint) movingDirection) {
		case MV_UP:

			// update stoneId and stoneType
			for (x=(uintStonePos) 0; x<this->stones[stoneId].width; x++) {
				fieldIndexMovingTo										= state->stonePos[stoneId] - fieldSizeX + x;
				fieldIndexMovingFrom									= state->stonePos[stoneId] + fieldSizeX * (this->stones[stoneId].height - (uintStonePos) 1) + x;
				state->stonePos[state->stoneId[fieldIndexMovingTo]]		= fieldIndexMovingFrom;
				state->stoneType[fieldIndexMovingFrom]					= ST_FREESQUARE;
				state->stoneId[fieldIndexMovingFrom]					= state->stoneId[fieldIndexMovingTo];
				state->stoneType[fieldIndexMovingTo]					= stoneType;
				state->stoneId[fieldIndexMovingTo]						= stoneId;
			}

			// update stonePos[]
			state->stonePos[stoneId] -= fieldSizeX;
			break;

		case MV_DOWN:

			// update stoneId and stoneType
			for (x=(uintStonePos) 0; x<this->stones[stoneId].width; x++) {
				fieldIndexMovingFrom									= state->stonePos[stoneId] + x;
				fieldIndexMovingTo										= state->stonePos[stoneId] + fieldSizeX * this->stones[stoneId].height + x;
				state->stonePos[state->stoneId[fieldIndexMovingTo]]		= fieldIndexMovingFrom;
				state->stoneType[fieldIndexMovingFrom]					= ST_FREESQUARE;
				state->stoneId[fieldIndexMovingFrom]					= state->stoneId[fieldIndexMovingTo];
				state->stoneType[fieldIndexMovingTo]					= stoneType;
				state->stoneId[fieldIndexMovingTo]						= stoneId;
			}

			// update stonePos[]
			state->stonePos[stoneId] += fieldSizeX;
			break;

		case MV_LEFT:

			// update stoneId and stoneType
			for (y=(uintStonePos) 0; y<this->stones[stoneId].height; y++) {
				fieldIndexMovingFrom									= state->stonePos[stoneId] + (this->stones[stoneId].width - (uintStonePos) 1) + y * fieldSizeX;
				fieldIndexMovingTo										= state->stonePos[stoneId] - (uintStonePos) 1 + y * fieldSizeX;
				state->stonePos[state->stoneId[fieldIndexMovingTo]]		= fieldIndexMovingFrom;
				state->stoneType[fieldIndexMovingFrom]					= ST_FREESQUARE;
				state->stoneId[fieldIndexMovingFrom]					= state->stoneId[fieldIndexMovingTo];
				state->stoneType[fieldIndexMovingTo]					= stoneType;
				state->stoneId[fieldIndexMovingTo]						= stoneId;
			}

			// update stonePos[]
			state->stonePos[stoneId] -= 1;
			break;

		case MV_RIGHT:

			// update stoneId and stoneType
			for (y=(uintStonePos) 0; y<this->stones[stoneId].height; y++) {
				fieldIndexMovingFrom									= state->stonePos[stoneId] + y * fieldSizeX;
				fieldIndexMovingTo										= state->stonePos[stoneId] + this->stones[stoneId].width + y * fieldSizeX;
				state->stonePos[state->stoneId[fieldIndexMovingTo]]		= fieldIndexMovingFrom;
				state->stoneType[fieldIndexMovingFrom]					= ST_FREESQUARE;
				state->stoneId[fieldIndexMovingFrom]					= state->stoneId[fieldIndexMovingTo];
				state->stoneType[fieldIndexMovingTo]					= stoneType;
				state->stoneId[fieldIndexMovingTo]						= stoneId;
			}

			// update stonePos[]
			state->stonePos[stoneId] += (uintStonePos) 1;
			break;
	}

	// update freeSquareIndex[]
	updateFreeSquareIndex(state);

	// everythink ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::updateFreeSquareIndex()
// Desc: Recalculates the array freeSquareIndex[] from the array stoneType[]
//-----------------------------------------------------------------------------
void traga2Class::updateFreeSquareIndex(fieldState *state)
{
	uintStonePos fieldIndex, freeSquareId;

	for (fieldIndex=0, freeSquareId=0; fieldIndex<fieldSizeX*fieldSizeY; fieldIndex++) {
		if (state->stoneType[fieldIndex] == ST_FREESQUARE) {
			state->freeSquareIndex[freeSquareId] = fieldIndex;
			freeSquareId++;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: traga2Class::updateStoneTypeToId()
// Desc: Recalculates the array stoneTypeToId[] from the arrays stoneType[], stonePos[]
//-----------------------------------------------------------------------------
void traga2Class::updateStoneTypeToId(fieldState *state)
{
	// locals
	uintStoneType		curStoneType;
	uintStoneId			curStoneId;
	arrayUInt_StoneType	stoneCounter(numStoneTypes);

	for (curStoneType=0; curStoneType<numStoneTypes; curStoneType++) {
		stoneCounter[curStoneType] = 0;
	}

	for (curStoneId=0; curStoneId<numStones; curStoneId++) {
		curStoneType = state->stoneType[state->stonePos[curStoneId]];
		stoneTypeToId[curStoneType * numSquaresInField + stoneCounter[curStoneType]] = curStoneId;
		stoneCounter[curStoneType]++;
	}
}

//-----------------------------------------------------------------------------
// Name: traga2Class::redimensionArrayAndAddItem()
// Desc: 
//-----------------------------------------------------------------------------
template <typename varType> void traga2Class::redimensionArrayAndAddItem(varType *&oldArray, unsigned int oldArraySize, varType *itemToAdd)
{
	// locals
	varType *newArray = new varType[oldArraySize+1];
	if (oldArray != NULL) {
		memcpy(newArray, oldArray, sizeof(varType) * oldArraySize);
		delete [] oldArray;	oldArray = nullptr;
	}
	memcpy(&newArray[oldArraySize], itemToAdd, sizeof(varType));
	oldArray = newArray;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::addStone()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::addStone(uintStonePos position, uintStonePos width, uintStonePos height, bool fixed)
{
	// locals
	uintStonePos	x, y, fieldIndex;
	uintStoneId		i, curStoneId, newStoneId;
	uintStoneType	curStoneType;

	// does stone fit?
	for (x=0; x<width; x++) { for (y=0; y<height; y++) {
		fieldIndex = position + y * fieldSizeX + x;
		if (fieldIndex >= curState->stoneType.size() || curState->stoneType[fieldIndex] != ST_FREESQUARE) {
			return false;
		}
	}}
	if (fieldSizeX<(position%fieldSizeX)+width ) return false;
	if (fieldSizeY<(position/fieldSizeX)+height) return false;

	// add fixed stone
	if (fixed) {
	
		numFreeSquares						-=         width*height;
		numStonesOfThisType[ST_FIXEDSTONE]	+= (uint) (width*height);
		numStonesOfThisType[ST_FREESQUARE]  -= (uint) (width*height);

		for (x=0; x<width; x++) { for (y=0; y<height; y++) {
			fieldIndex = position + y * fieldSizeX + x;
			curState->stoneType[fieldIndex] = ST_FIXEDSTONE;
		}}

	// add moveable stone
	} else {
		
		// stonetype already existend?
		for (curStoneType=ST_NUM_DEF; curStoneType<numStoneTypes; curStoneType++) {
			if (width==stoneTypes[curStoneType].width && height==stoneTypes[curStoneType].height) {
				break;
			}
		}

		// stonetype not yet in list? so add it!
		if (curStoneType==numStoneTypes) {
			stoneTypes[numStoneTypes].width		= width;
			stoneTypes[numStoneTypes].height	= height;
			numStonesOfThisType[numStoneTypes]	= 1;
			numStonesOfThisType[ST_FREESQUARE] -= (uint) (width*height);
			numStoneTypes++;
		} else {
			numStonesOfThisType[curStoneType]++;
			numStonesOfThisType[ST_FREESQUARE] -= (uint) (width*height);
		}

		// calc new stoneId
		for (i=0; i<numStones; i++) {
			if (curState->stoneType[curState->stonePos[i]] == ST_FREESQUARE) {
				newStoneId = i;
				break;
			}
		}

		// zero free squares
		for (fieldIndex=0; fieldIndex<fieldSizeX*fieldSizeY; fieldIndex++) {
			if (curState->stoneType[fieldIndex] == ST_FREESQUARE) {
				curState->stonePos[curState->stoneId[fieldIndex]]	=			    fieldSizeX*fieldSizeY;
				curState->stoneId[fieldIndex]						= (uintStoneId) fieldSizeX*fieldSizeY;
			}
		}

		// insert new stone
		numStones						= numStones - width*height + (uintStoneId) 1;
		numFreeSquares					= numFreeSquares - width*height;
		stones[newStoneId].width		= width;
		stones[newStoneId].height		= height;
		curState->stonePos[newStoneId]	= position;

		for (x=0; x<width; x++) { for (y=0; y<height; y++) {
			fieldIndex = position + y * fieldSizeX + x;
			curState->stoneId[fieldIndex]	= newStoneId;
			curState->stoneType[fieldIndex] = curStoneType;
		}}

		// reassign stone id's if there are any higher than number of stones on the board
		for (fieldIndex=0; fieldIndex<fieldSizeX*fieldSizeY; fieldIndex++) {
			if (curState->stoneId[fieldIndex] >= numStones && curState->stoneId[fieldIndex] != fieldSizeX*fieldSizeY) {

				// search appropriate stoneId for this stone
				for (curStoneId=0; curStoneId<numStones; curStoneId++) {
					if (curState->stonePos[curStoneId] == fieldSizeX*fieldSizeY) break;
				}
				
				curState->stonePos[curStoneId]	= fieldIndex;
				stones			  [curStoneId]	= stones[curState->stoneId[fieldIndex]];

				for (x=0; x<stones[curStoneId].width; x++) { for (y=0; y<stones[curStoneId].height; y++) {
					curState->stoneId  [fieldIndex + y * fieldSizeX + x] = curStoneId;

				}}
			}
		}

		// fill up rest with free squares
		for (fieldIndex=0; fieldIndex<fieldSizeX*fieldSizeY; fieldIndex++) {
			if (curState->stoneType[fieldIndex] == ST_FREESQUARE) {
				
				// search appropriate stoneId for free square
				for (curStoneId=0; curStoneId<numStones; curStoneId++) {
					if (curState->stonePos[curStoneId] == fieldSizeX*fieldSizeY) break;
				}
				curState->stonePos[curStoneId]	= fieldIndex;
				curState->stoneId[fieldIndex]	= curStoneId;
			}
		}
	}

	// update curState->freeSquareIndex[]
	updateFreeSquareIndex(curState);

	// every was ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::removeAllStones()
// Desc: 
//-----------------------------------------------------------------------------
void traga2Class::removeAllStones()
{
	// locals
	uintStonePos i;

	numStoneTypes						= ST_NUM_DEF;
	numStones							= (uintStoneId) numSquaresInField;
	numFreeSquares						= numSquaresInField;
	stoneTypes[ST_FREESQUARE].width		= 1;
	stoneTypes[ST_FREESQUARE].height	= 1;
	numStonesOfThisType[ST_FREESQUARE]	= (uint) numSquaresInField;
	stoneTypes[ST_FIXEDSTONE].width		= 1;
	stoneTypes[ST_FIXEDSTONE].height	= 1;
	numStonesOfThisType[ST_FIXEDSTONE]	= 0;

	for (i=0; i<numSquaresInField; i++) {
		stones[i].width		= 1;
		stones[i].height	= 1;
	}
	
	for (i=0; i<fieldSizeX*fieldSizeY; i++) {
		curState->stoneType			[i]	= ST_FREESQUARE;
		curState->stoneId			[i]	= i;
		curState->stonePos			[i]	= i;
		curState->restingStones		[i]	= 0;
	}

	updateFreeSquareIndex(curState);
}

//-----------------------------------------------------------------------------
// Name: traga2Class::removeStone()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::removeStone(uintStonePos position)
{
	// locals
	uintStonePos	x, y, fieldIndex, width, height;
	uintStoneType	curStoneType;
	uintStoneId		stoneId;

	// is there a stone which can be removed?
	if (curState->stoneType[position] == ST_FREESQUARE) return false;

	// adjust position, so that it is the upper left corner of the stone
	position		= curState->stonePos[curState->stoneId[position]];
	curStoneType	= curState->stoneType[position];
	stoneId			= curState->stoneId[position];
	width			= stoneTypes[curStoneType].width;
	height			= stoneTypes[curStoneType].height;

	// if last stone of this type, then remove it
	if (numStonesOfThisType[curStoneType] == 1 && curStoneType != ST_FIXEDSTONE) {
		stoneTypes[curStoneType]			= stoneTypes[numStoneTypes - (uintStoneType) 1];
		numStonesOfThisType[curStoneType]	= numStonesOfThisType[numStoneTypes - (uintStoneType) 1];
		numStonesOfThisType[ST_FREESQUARE] += (uint) (width*height);
		numStoneTypes--;

		// update curState->stoneType[]
		for (fieldIndex=0; fieldIndex<numSquaresInField; fieldIndex++) {
			if (curState->stoneType[fieldIndex] == numStoneTypes) {
				curState->stoneType[fieldIndex] = curStoneType;
			}
		}
	} else {
		numStonesOfThisType[curStoneType]--;
		numStonesOfThisType[ST_FREESQUARE] += (uint) (width*height);
	}

	// update curState->stoneId[], curState->stoneType[], stones,  
	for (x=0; x<width; x++) { for (y=0; y<height; y++) {

		fieldIndex						= position + y * fieldSizeX + x;
		stones[stoneId].width			= 1;
		stones[stoneId].height			= 1;
		curState->stonePos[stoneId]		= fieldIndex;
		curState->stoneId[fieldIndex]	= stoneId;
		curState->stoneType[fieldIndex] = ST_FREESQUARE;

		// select appropriate stoneId (for x==0 and y==0 it's the stoneId of the removed stone)
		if (x==(uintStoneType)0 && y==(uintStoneType)0) {
			stoneId = numStones;
		} else {
			stoneId++; 
		}
	}}

	numStones						+= width*height - (uintStoneType) 1;
	numFreeSquares					+= width*height;

	// update curState->freeSquareIndex[]
	updateFreeSquareIndex(curState);

	// every was ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::copyFieldState()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::copyFieldState(traga2Class *sourceState)
{
	// parameter ok?
	if (this->fieldSizeX != sourceState->fieldSizeX) return false;
	if (this->fieldSizeY != sourceState->fieldSizeY) return false;

	this->numStoneTypes			= sourceState->numStoneTypes;
	this->numStones				= sourceState->numStones;
	this->numFreeSquares		= sourceState->numFreeSquares;
	this->isFinalState			= sourceState->isFinalState;

	this->stoneTypeToId			= sourceState->stoneTypeToId;
	this->numStonesOfThisType	= sourceState->numStonesOfThisType;
	this->stoneTypes			= sourceState->stoneTypes;
	this->stones				= sourceState->stones;

	sourceState->curState->copyFieldTo(this->curState, numSquaresInField);

	// every was ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::copyFixedStones()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::copyFixedStones(traga2Class *sourceState)
{
	// parameter ok?
	if (this->fieldSizeX != sourceState->fieldSizeX) return false;
	if (this->fieldSizeY != sourceState->fieldSizeY) return false;

	// locals
	uintStonePos curPosition;
		
	// remove all stones
	removeAllStones();

	// add fixed stones
	for (curPosition=0; curPosition<numSquaresInField; curPosition++) {
		if (sourceState->curState->stoneType[curPosition] == ST_FIXEDSTONE) {
			addStone(sourceState, sourceState->curState->stoneId[curPosition], curPosition);
		}
	}

	// every was ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: load()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::load(wstring& filename)
{
	// locals
	HANDLE			hFile		= NULL;
	DWORD			bytesRead	= 0;
	uintStonePos	fileFieldSizeX, fileFieldSizeY;

	// parameters ok?
	if (filename.length() == 0) return false;
	if (this			  == NULL) return false;

	// open file
	hFile = CreateFile(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// handle valid ?
	if (hFile == INVALID_HANDLE_VALUE) return false;

	// read header 
	READ_AND_CHECK(hFile, &fileFieldSizeX,					sizeof(unsigned int),								&bytesRead)
	READ_AND_CHECK(hFile, &fileFieldSizeY,					sizeof(unsigned int), 								&bytesRead)

	// correct size ?
	if (fileFieldSizeX != fieldSizeX || fileFieldSizeY != fieldSizeY) {
		CloseHandle(hFile);
		return false;
	}

	READ_AND_CHECK(hFile, &numSquaresInField,				sizeof(unsigned int), 								&bytesRead)
	READ_AND_CHECK(hFile, &numStoneTypes,					sizeof(unsigned int), 								&bytesRead)
	READ_AND_CHECK(hFile, &numStones,						sizeof(unsigned int), 								&bytesRead)
	READ_AND_CHECK(hFile, &numFreeSquares,					sizeof(unsigned int), 								&bytesRead)
	READ_AND_CHECK(hFile, &isFinalState,					sizeof(bool),										&bytesRead)

	// Hier muss Wert der Variable "p" stehen, nicht der Zeiger auf die Klasse.
	READ_AND_CHECK(hFile, &numStonesOfThisType[0],			sizeof(unsigned int) * (uint) numSquaresInField,	&bytesRead)
	READ_AND_CHECK(hFile, &stoneTypes[0],					sizeof(stoneDesc   ) * (uint) numSquaresInField,	&bytesRead)
	READ_AND_CHECK(hFile, &stones[0],						sizeof(stoneDesc   ) * (uint) numSquaresInField,	&bytesRead)
	
	READ_AND_CHECK(hFile, &curState->numStonesSet,			sizeof(unsigned int), 								&bytesRead)
	READ_AND_CHECK(hFile, &curState->stoneType[0],			sizeof(unsigned int) * (uint) numSquaresInField,	&bytesRead)
	READ_AND_CHECK(hFile, &curState->stoneId[0],			sizeof(unsigned int) * (uint) numSquaresInField,	&bytesRead)
	READ_AND_CHECK(hFile, &curState->stonePos[0],			sizeof(unsigned int) * (uint) numSquaresInField,	&bytesRead)
	READ_AND_CHECK(hFile, &curState->freeSquareIndex[0],	sizeof(unsigned int) * (uint) numSquaresInField,	&bytesRead)
	READ_AND_CHECK(hFile, &curState->restingStones[0],		sizeof(unsigned int) * (uint) numSquaresInField,	&bytesRead)

	// close file
	CloseHandle(hFile);

	return true;
}

//-----------------------------------------------------------------------------
// Name: save()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::save(wstring& filename)
{
	// locals
	HANDLE	hFile		= NULL;
	DWORD	bytesWritten= 0;

	// parameters ok?
	if (filename.length() == NULL)	return false;
	if (this			  == NULL)	return false;
				
	// open file
	hFile = CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// handle valid ?
	if (hFile == INVALID_HANDLE_VALUE) return false;

	// save header 
	WRITE_AND_CHECK(hFile, &fieldSizeX,							sizeof(unsigned int),								&bytesWritten)
	WRITE_AND_CHECK(hFile, &fieldSizeY,							sizeof(unsigned int), 								&bytesWritten)
	WRITE_AND_CHECK(hFile, &numSquaresInField,					sizeof(unsigned int), 								&bytesWritten)
	WRITE_AND_CHECK(hFile, &numStoneTypes,						sizeof(unsigned int), 								&bytesWritten)
	WRITE_AND_CHECK(hFile, &numStones,							sizeof(unsigned int), 								&bytesWritten)
	WRITE_AND_CHECK(hFile, &numFreeSquares,						sizeof(unsigned int), 								&bytesWritten)
	WRITE_AND_CHECK(hFile, &isFinalState,						sizeof(bool),										&bytesWritten)
	WRITE_AND_CHECK(hFile, &numStonesOfThisType[0],				sizeof(unsigned int) *(uint)  numSquaresInField,	&bytesWritten)
	WRITE_AND_CHECK(hFile, &stoneTypes[0],						sizeof(stoneDesc   ) *(uint)  numSquaresInField,	&bytesWritten)
	WRITE_AND_CHECK(hFile, &stones[0],							sizeof(stoneDesc   ) *(uint)  numSquaresInField,	&bytesWritten)
																				  
	WRITE_AND_CHECK(hFile, &curState->numStonesSet,				sizeof(unsigned int), 								&bytesWritten)
	WRITE_AND_CHECK(hFile, &curState->stoneType[0],				sizeof(unsigned int) *(uint)  numSquaresInField,	&bytesWritten)
	WRITE_AND_CHECK(hFile, &curState->stoneId[0],				sizeof(unsigned int) *(uint)  numSquaresInField,	&bytesWritten)
	WRITE_AND_CHECK(hFile, &curState->stonePos[0],				sizeof(unsigned int) *(uint)  numSquaresInField,	&bytesWritten)
	WRITE_AND_CHECK(hFile, &curState->freeSquareIndex[0],		sizeof(unsigned int) *(uint)  numSquaresInField,	&bytesWritten)
	WRITE_AND_CHECK(hFile, &curState->restingStones[0],			sizeof(unsigned int) *(uint)  numSquaresInField,	&bytesWritten)

	// close file
	CloseHandle(hFile);

	return true;
}

//-----------------------------------------------------------------------------
// Name: addStone()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::addStone(traga2Class *sourceField, uintStoneId stoneId, uintStonePos destPosition)
{
	// locals
	uintStonePos	i, x, y, fieldIndex;
	uintStonePos	sourcePos		= sourceField->curState->stonePos[stoneId];
	uintStoneType	curStoneType	= sourceField->curState->stoneType[sourcePos];
	uintStonePos	stoneWidth		= sourceField->stones[stoneId].width;
	uintStonePos	stoneHeight		= sourceField->stones[stoneId].height;

	// does sourceField fit?
	if (this->fieldSizeX != sourceField->fieldSizeX) return false;
	if (this->fieldSizeY != sourceField->fieldSizeY) return false;

	// does stone fit?
	for (x=0; x<stoneWidth; x++) { for (y=0; y<stoneHeight; y++) {
		fieldIndex = destPosition + y * fieldSizeX + x;
		if (fieldIndex >= fieldSizeX*fieldSizeY || curState->stoneType[fieldIndex] != ST_FREESQUARE) {
			return false;
		}
	}}
	if (fieldSizeX<(destPosition%fieldSizeX)+stoneWidth ) return false;
	if (fieldSizeY<(destPosition/fieldSizeX)+stoneHeight) return false;

	// is stone already set?
	for (i=0; i<numSquaresInField; i++) {
		if (this->curState->stoneType[i] != ST_FREESQUARE 
		&&  this->curState->stoneId[i] == stoneId) return false;
	}

	// copying field properties for the first time?
	if (this->numStoneTypes == ST_NUM_DEF) {
		this->numStoneTypes		 = sourceField->numStoneTypes;
		for (i=0; i<numSquaresInField; i++) {
			this->numStonesOfThisType	[(uintStoneType) i]	= sourceField->numStonesOfThisType	[(uintStoneType) i];
			this->stoneTypes			[(uintStoneType) i] = sourceField->stoneTypes			[(uintStoneType) i];
			this->stones				[(uintStoneId)   i] = sourceField->stones				[(uintStoneId)   i];
			this->curState->stoneType	[                i]	=				ST_FREESQUARE;
			this->curState->stoneId		[                i]	= (uintStoneId) numSquaresInField;
			this->curState->stonePos	[(uintStoneId)   i]	=				numSquaresInField;
		}
	}

	this->numFreeSquares	-=				 (stoneWidth * stoneHeight);
	this->numStones			-= (uintStoneId) (stoneWidth * stoneHeight - (uintStonePos) 1);
	
	// curState
	for (x=0; x<stoneWidth; x++) { for (y=0; y<stoneHeight; y++) {
		fieldIndex = destPosition + y * fieldSizeX + x;
		this->curState->stoneType[fieldIndex]	= curStoneType;
		this->curState->stoneId  [fieldIndex]	= stoneId;
	}}
	this->curState->stonePos[stoneId]	= destPosition;

	// everything ok
	isFinalState = true;
	return true;
}

//-----------------------------------------------------------------------------
// Name: removeStone()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::removeStone(traga2Class *sourceField, uintStonePos destPosition)
{
	// locals
	uintStonePos	i, x, y, fieldIndex;
	uintStoneId		stoneId			= this->curState->stoneId[destPosition];
	uintStonePos	sourcePos		= sourceField->curState->stonePos[stoneId];
	uintStoneType	curStoneType	= sourceField->curState->stoneType[sourcePos];
	uintStonePos	stoneWidth		= sourceField->stones[stoneId].width;
	uintStonePos	stoneHeight		= sourceField->stones[stoneId].height;

	// adjust position, so that it is the upper left corner of the stone
	destPosition	= curState->stonePos[curState->stoneId[destPosition]];

	// is there a stone which can be removed?
	if (curState->stoneType[destPosition] == ST_FREESQUARE)		return false;

	// copy source field properties
	this->numStoneTypes		 = sourceField->numStoneTypes;
	for (i=0; i<numSquaresInField; i++) {
		this->numStonesOfThisType	[(uintStoneType) i]	= sourceField->numStonesOfThisType	[(uintStoneType) i];
		this->stoneTypes			[(uintStoneType) i]	= sourceField->stoneTypes			[(uintStoneType) i];
		this->stones				[(uintStoneId)   i]	= sourceField->stones				[(uintStoneId)   i];
	}

	// curState
	for (x=0; x<stoneWidth; x++) { for (y=0; y<stoneHeight; y++) {
		fieldIndex = destPosition + y * fieldSizeX + x;
		this->curState->stoneType[fieldIndex]	=			    ST_FREESQUARE;
		this->curState->stoneId  [fieldIndex]	= (uintStoneId) numSquaresInField;
	}}
	this->curState->stonePos[stoneId]	 = numSquaresInField;
	this->numFreeSquares				+=				 (stoneWidth * stoneHeight);
	this->numStones						+= (uintStoneId) (stoneWidth * stoneHeight - (uintStonePos) 1);

	// everything ok 
	isFinalState = true;
	return true;
}

//-----------------------------------------------------------------------------
// Name: checkIntegrity()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::checkIntegrity()
{
	// ...

	// integrity is ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::calcPath()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::calcPath(uintStepId &numSteps, arrayStonePos_StepId &stonePos, arrayDirection_StepId &direction)
{
	// locals
	fieldStateListClass	*myState;
	fieldStateListClass *cheapestEndState = NULL;
	fieldStateListClass *path, *tmpPath, *curPath;
	unsigned int		costOfcheapestPath = 1000000000;

	// search cheapest end state
	for (myState=firstStateListItem; myState != NULL; myState = myState->successor) {
		
		// target cheaper ?
		if ((costOfcheapestPath > myState->cost)&&(myState->cost != 0)) {
		
			// target state ?
			if (isTarget(myState->state)) {
				costOfcheapestPath	= myState->cost;
				cheapestEndState	= myState;
			}
		}
	}

	// quit if target wasn't found
	if (cheapestEndState == NULL)
		return false;

	// make path
	for (tmpPath=NULL,myState=cheapestEndState; myState != NULL; myState=myState->shortestWayOrigin) {
		
		curPath						= new fieldStateListClass();
		curPath->connectedStates	= myState;
		if (tmpPath == NULL) {
			curPath->index			= 0;
			curPath->predecessor	= NULL;
		} else {
			curPath->index			= tmpPath->index + 1;
			curPath->predecessor	= tmpPath;
			tmpPath->successor		= curPath;
		}
		tmpPath						= curPath;
	}

	// translate
	path		= curPath;
	numSteps	= curPath->index;
	stonePos	.resize(numSteps);
	direction	.resize(numSteps);

	// print path in reverse way
	for (myState=path, numSteps=0; myState->predecessor != NULL; myState=myState->predecessor) {
		calcMove(myState->connectedStates->state, myState->predecessor->connectedStates->state, stonePos[numSteps], direction[numSteps]);
		numSteps++;
	}
	
	// delete list backwards
	for (curPath=path; curPath!=NULL; curPath = tmpPath) {
		tmpPath = curPath->predecessor;
		delete curPath;
		curPath = nullptr;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::calcMove()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::calcMove(fieldState *stateFrom, fieldState *stateTo, uintStonePos &stonePos, uintDirection &direction)
{
	// locals
	uintStonePos stoneIdFrom = numSquaresInField, stoneIdTo = numSquaresInField, fieldIndex;
	uintStonePos xPosFrom, yPosFrom, xPosTo, yPosTo;
	
	for (fieldIndex=0; fieldIndex<numSquaresInField; fieldIndex++) {

		if (stateFrom->stoneType[fieldIndex] != ST_FREESQUARE && stateTo->stoneType[fieldIndex] == ST_FREESQUARE) {
			if (stoneIdFrom == numSquaresInField) {
				stoneIdFrom = stateFrom->stoneId[fieldIndex];
			} else if (stoneIdFrom != stateFrom->stoneId[fieldIndex]) {
				return false;
			}
		}

		if (stateFrom->stoneType[fieldIndex] == ST_FREESQUARE && stateTo->stoneType[fieldIndex] != ST_FREESQUARE) {
			if (stoneIdTo == numSquaresInField) {
				stoneIdTo = stateTo->stoneId[fieldIndex];
			} else if (stoneIdTo != stateTo->stoneId[fieldIndex]) {
				return false;
			}
		}
	}

	stonePos = stateFrom->stonePos[stoneIdFrom];

	xPosFrom = stateFrom->stonePos[stoneIdFrom] % fieldSizeX;
	yPosFrom = stateFrom->stonePos[stoneIdFrom] / fieldSizeX;
	xPosTo   = stateTo  ->stonePos[stoneIdTo  ] % fieldSizeX;
	yPosTo   = stateTo  ->stonePos[stoneIdTo  ] / fieldSizeX;

	if (xPosFrom == xPosTo && yPosFrom > yPosTo) {
		direction = MV_UP;
	} else if (xPosFrom == xPosTo && yPosFrom < yPosTo) {
		direction = MV_DOWN;
	} else if (yPosFrom == yPosTo && xPosFrom > xPosTo) {
		direction = MV_LEFT;
	} else if (yPosFrom == yPosTo && xPosFrom < xPosTo) {
		direction = MV_RIGHT;
	} else {
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2Class::findShortestWay()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2Class::findShortestWay(fieldStateListClass *start)
{
	// locals
	fieldStateListClass	*tmpKnot, *connectedKnot, *newKnot, *forSearchKnot, *lastItemInList, *knot;

	// init
	sortedTouchedStatesList		= NULL;
	start->cost					= 0;
	start->shortestWayOrigin	= NULL;
	start->knotState			= KS_VIRGIN;
	knot						= start;

	do {

		// skip when already finished
		if (knot->knotState != KS_FINISHED) {

			// for this knot the cheapest way was found
			knot->knotState = KS_FINISHED;

			// calc costs to connected knots
			for (tmpKnot = knot->connectedStates; tmpKnot != NULL; tmpKnot = tmpKnot->successor) {
				
				// current connected knot
				connectedKnot = tmpKnot->connectedStates;

				// ignore finished knots
				if (connectedKnot->knotState == KS_FINISHED) continue;
				
				// is knot already touched or still virgin ?
				if (((connectedKnot->knotState == KS_TOUCHED)&&(connectedKnot->cost > knot->cost + 1))
				   ||(connectedKnot->knotState == KS_VIRGIN)){

					// save knot in sorted list
					connectedKnot->cost					= knot->cost + 1;
					connectedKnot->knotState			= KS_TOUCHED;
					connectedKnot->shortestWayOrigin	= knot;

					newKnot								= new fieldStateListClass();
					newKnot->connectedStates			= connectedKnot;
					if (sortedTouchedStatesList == NULL) {
						sortedTouchedStatesList			= newKnot;
					} else {

						// search position in sorted list
						for (forSearchKnot=sortedTouchedStatesList; forSearchKnot != NULL; forSearchKnot = forSearchKnot->successor) {

							// stop when at least an equal expensive knot was found
							if (newKnot->connectedStates->cost <= forSearchKnot->connectedStates->cost) break;
						}

						// insert newKnot in list
						if (forSearchKnot == NULL) {
							for (lastItemInList=sortedTouchedStatesList; lastItemInList->successor != NULL; lastItemInList = lastItemInList->successor) {}
							lastItemInList->successor				= newKnot;
							newKnot->predecessor					= lastItemInList;
						} else if (forSearchKnot->predecessor == NULL) {
							forSearchKnot->predecessor				= newKnot;
							newKnot->successor						= forSearchKnot;
							sortedTouchedStatesList					= newKnot;
						} else {
							newKnot->predecessor					= forSearchKnot->predecessor;
							newKnot->successor						= forSearchKnot;
							forSearchKnot->predecessor->successor	= newKnot;
							forSearchKnot->predecessor				= newKnot;
						}
					}
				}
			}
		}

		// delete cheapest knot from list
		if (sortedTouchedStatesList != NULL) {			
			tmpKnot									= sortedTouchedStatesList->connectedStates;
			sortedTouchedStatesList					= sortedTouchedStatesList->successor;
			if ((sortedTouchedStatesList != NULL)&&(sortedTouchedStatesList->predecessor != NULL)) {
				delete sortedTouchedStatesList->predecessor;
				sortedTouchedStatesList->predecessor	= NULL;
			}
			
			// process the cheapest knot
			knot = tmpKnot;

		// all knot were already processed
		} else {
			knot = NULL;
		}

	} while (knot != NULL);

	// everything is ok
	return true;
}

//-----------------------------------------------------------------------------
// Name: solutionPathListClass::addStep()
// Desc: 
//-----------------------------------------------------------------------------
void traga2Class::solutionPathListClass::addStep(uintStonePos position, uintDirection direction)
{
	solutionPath myPath;
	myPath.direction	= direction;
	myPath.position		= position;
	solPath.push_back(myPath);
}

//-----------------------------------------------------------------------------
// Name: solutionPathListClass::addStep()
// Desc: 
//-----------------------------------------------------------------------------
void traga2Class::solutionPathListClass::addStepsAndDeleteArrays(uintStepId numSteps, arrayStonePos_StepId position, arrayDirection_StepId direction)
{
	solutionPath myPath;
	uintStepId   curStep;

	for (curStep=0;curStep<numSteps; curStep++) {
		myPath.direction	= direction[curStep];
		myPath.position		= position [curStep];
		solPath.push_back(myPath);
	}
	position.clear();
	direction.clear();
}

//-----------------------------------------------------------------------------
// Name: solutionPathListClass::addStep()
// Desc: 
//-----------------------------------------------------------------------------
void traga2Class::solutionPathListClass::extract(uintStepId &numSteps, arrayStonePos_StepId &position, arrayDirection_StepId &direction)
{
	list<solutionPath>::iterator	itr;
	uintStepId curStep;

	numSteps  = solPath.size();
	direction .resize(numSteps);
	position  .resize(numSteps);

	for (curStep=0,itr=solPath.begin(); itr!=solPath.end(); itr++,curStep++) {
		direction[curStep] = (*itr).direction;
		position [curStep] = (*itr).position;
	}
}

//-----------------------------------------------------------------------------
// Name: solutionPathListClass::addStep()
// Desc: 
//-----------------------------------------------------------------------------
uintDirection traga2Class::getInverseDirection(uintDirection direction)
{
	switch ((uint) direction) 
	{
	case MV_UP:		return MV_DOWN;
	case MV_DOWN:	return MV_UP;
	case MV_LEFT:	return MV_RIGHT;
	case MV_RIGHT:	return MV_LEFT;
	default:		return MV_NUM_DIRECTIONS;
	}
}