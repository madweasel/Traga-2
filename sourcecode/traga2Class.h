/*********************************************************************\
	traga2Class.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef TRAGA2CLASS_H
#define TRAGA2CLASS_H

// includes
#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <functional>
#include <map>
#include <vector>
#include "varCheck.h"
#include <windows.h>

// compiling options
//#define CHECK_VARPARAM_DETAILS

using namespace std;

// globals vars & namespaces
namespace traga2classes
{
	#ifdef CHECK_VARPARAM_DETAILS

		typedef	varCheckClass<unsigned int>					uintStoneType;
		typedef	varCheckClass<unsigned int>					uintStoneId;
		typedef	varCheckClass<unsigned int>					uintStonePos;
		typedef	varCheckClass<unsigned int>					uintDirection;
		typedef	varCheckClass<unsigned int>					uintStepId;

		typedef	varCheckTypeCaster<unsigned int>			uint;

		// structures
		struct stoneDesc
		{
			uintStonePos	width, height;
		};

		typedef varCheckArray<uintStoneType, uintStonePos >	arrayStoneType_StonePos;
		typedef varCheckArray<uintStoneId  , uintStonePos >	arrayStoneId_StonePos;
		typedef varCheckArray<uintStonePos , uintStoneId  >	arrayStonePos_StoneId; 
		typedef varCheckArray<uintStonePos , uintStonePos >	arrayStonePos_StonePos; 
		typedef varCheckArray<uintStonePos , uintStepId   >	arrayStonePos_StepId;
		typedef varCheckArray<unsigned int , uintStoneType>	arrayUInt_StoneType;
		typedef varCheckArray<unsigned int , uintStonePos>	arrayUInt_StonePos;
		typedef varCheckArray<stoneDesc    , uintStoneType>	arrayStoneDesc_StoneType;
		typedef varCheckArray<stoneDesc    , uintStoneId>	arrayStoneDesc_StoneId;
		typedef varCheckArray<uintStoneId  , uintStoneType>	arrayStoneId_StoneType;
		typedef varCheckArray<uintDirection, uintStepId   >	arrayDirection_StepId;
	#else
		typedef unsigned int								uintStoneType;
		typedef unsigned int								uintStoneId;
		typedef unsigned int								uintStonePos;
		typedef unsigned int								uintDirection;
		typedef unsigned int								uintStepId;
		typedef unsigned int								uint;

		// structures
		struct stoneDesc
		{
			uintStonePos	width, height;
		};

		typedef vector<unsigned int>						arrayStonePos_StonePos;
		typedef vector<unsigned int>						arrayStoneType_StonePos;
		typedef vector<unsigned int>						arrayStoneId_StonePos;
		typedef vector<unsigned int>						arrayStoneId_StoneType;
		typedef vector<unsigned int>						arrayStonePos_StoneId;  
		typedef vector<unsigned int>						arrayStonePos_StepId;  
		typedef vector<unsigned int>						arrayDirection_StepId;  
		typedef vector<unsigned int>						arrayUInt_StoneType;
		typedef vector<unsigned int>						arrayUInt_StonePos;
		typedef vector<stoneDesc>							arrayStoneDesc_StoneId;
		typedef vector<stoneDesc>							arrayStoneDesc_StoneType; 
	#endif
}

using namespace traga2classes;

// constants
#define	WRITE_AND_CHECK(hFile,pVar,numBytes,pBytesWritten)		{ WriteFile((hFile), (pVar), (numBytes), (pBytesWritten), NULL); if ((*(pBytesWritten)) != (numBytes)) { CloseHandle(hFile); return false; } }
#define	READ_AND_CHECK(hFile,pVar,numBytes,pBytesRead)  		{ ReadFile ((hFile), (pVar), (numBytes), (pBytesRead),    NULL); if ((*(pBytesRead   )) != (numBytes)) { CloseHandle(hFile); return false; } }

#define	ST_FREESQUARE				((uintStoneType) 0)			// that's a reserved stone type: a free square of size 1x1
#define ST_FIXEDSTONE				((uintStoneType) 1)			// a 1x1 stone that won't be moved
#define ST_NUM_DEF					((uintStoneType) 2)

#define	MV_UP						0							// move direction
#define	MV_DOWN						1
#define	MV_LEFT						2
#define	MV_RIGHT					3
#define	MV_NUM_DIRECTIONS			4

#define	KS_FINISHED					1							// knot state
#define	KS_TOUCHED					2
#define	KS_VIRGIN					3

// classes
class traga2Class
{

protected:

	// structures
	class fieldState
	{
	public:
									fieldState					(uintStonePos fieldSizeX, uintStonePos fieldSizeY);
									~fieldState					();

		arrayStoneType_StonePos		stoneType;												// [numSquaresInField]		contains the stone type  of each square of the field
		arrayStoneId_StonePos 		stoneId;												// [numSquaresInField]		contains the stone index of each square of the field
		arrayStonePos_StoneId 		stonePos;												// [numStones]				contains the field index of the upper left corner of each stone

		// solver variables
		arrayStonePos_StonePos		freeSquareIndex;										// [numFreeSquares]			contains the field index of each free square
		arrayUInt_StoneType			restingStones;											// [numStoneTypes]			contains the number of unused stones of each stone type
		unsigned int				numStonesSet;											//							number of used stones / number of stones laying on the field

		bool						isTarget					(fieldState& targetState,  unsigned int fieldSize);
		bool						hasEqualStoneType			(fieldState& compareState, unsigned int fieldSize);
		bool						hasEqualStoneId				(fieldState& compareState, unsigned int fieldSize);
		void						copyFieldTo					(fieldState *to, uintStonePos fieldSize);
	};

	class fieldStateListClass
	{
	public:
		unsigned int				index;
		unsigned int				cost;
		unsigned int				knotState;
		fieldStateListClass			*predecessor;
		fieldStateListClass			*successor;
		fieldStateListClass			*connectedStates;
		fieldStateListClass			*shortestWayOrigin;
		fieldState					*state;

		fieldStateListClass();
		~fieldStateListClass();
	};

	class solutionPathListClass
	{
	private:
		struct solutionPath
		{
			uintStonePos			position;
			uintDirection			direction;
		};

	public:
		// Variables
		list<solutionPath>  		solPath;

		// functions
		void						addStep					(					   uintStonePos          position, uintDirection          direction);
		void						addStepsAndDeleteArrays	(uintStepId  numSteps, arrayStonePos_StepId  position, arrayDirection_StepId  direction);
		void						extract					(uintStepId &numSteps, arrayStonePos_StepId &position, arrayDirection_StepId &direction);
	};

	// globals
	uintStonePos					fieldSizeX, fieldSizeY;
	uintStonePos					numSquaresInField;										// = fieldSizeX * fieldSizeY
	uintStoneType					numStoneTypes;
	uintStoneId						numStones;
	uintStonePos					numFreeSquares;
	bool							isFinalState;
	arrayUInt_StoneType				numStonesOfThisType;									// [numStoneTypes]			number of stones of this type
	arrayStoneDesc_StoneType		stoneTypes;												// [numStoneTypes]			contains width and height of each stone type
	arrayStoneDesc_StoneId			stones;													// [numStones]				contains width and height of each stone
	fieldState						*curState						= nullptr;				// current state

	// solver vars
	fieldState						*targetState					= nullptr;				// target state (only stoneId, stoneType and stonePos are valid)
	arrayStoneId_StoneType			stoneTypeToId					;						// [stoneTypes][numStones]
	fieldStateListClass				*firstStateListItem				= nullptr;
	fieldStateListClass				*lastStateListItem				= nullptr;
	fieldStateListClass				*sortedTouchedStatesList		= nullptr;
	std::map<arrayStoneType_StonePos, fieldStateListClass*>			stateListMap;
	function<bool(int, int, wchar_t*)>	showProgress				= nullptr;				// parameters are (int numPossibleStates, int numProcessedStates);
	
	// Helper Functions
	void							updateStoneTypeToId				(fieldState *state);
	void							updateFreeSquareIndex			(fieldState *state);
	int								square							(int a) {return a*a;};

	template <typename varType> 	void redimensionArrayAndAddItem	(varType *&oldArray, unsigned int arraySize, varType *itemToAdd);

	// Solver Functions
	bool							prepareCurAndTargetState		(fieldState *theTargetState);
	bool							findAllStates					();
	bool							tryStone						(uintStonePos posIndex, fieldState *field, fieldStateListClass* &curStateListItem);
	bool							calcConnections					();
	fieldStateListClass *			findStateInList					(fieldState *state);
	bool							findShortestWay					(fieldStateListClass *start);
	bool							isTarget						(fieldState *state);
	bool							calcPath						(uintStepId &numSteps, arrayStonePos_StepId &stonePos, arrayDirection_StepId &direction);
	bool							calcMove						(fieldState *stateFrom, fieldState *stateTo, uintStonePos &stonePos, uintDirection &direction);
	bool							oneTimesOneAlgorithmn			(traga2Class *target, uintStepId &numSteps, arrayStonePos_StepId &stonePos, arrayDirection_StepId &direction);
	unsigned int					countUnsolvedNeighbours			(uintStonePos targetPos, bool positiveDirection, arrayStoneType_StonePos stoneType, bool horizontalNotVertical);

public:

	// Functions
									traga2Class						(uintStonePos sizeX, uintStonePos sizeY);
									~traga2Class					();

	bool							save							(wstring& filename);
	bool							load							(wstring& fileName);
	bool							loadDefaultState				();
	bool							checkIntegrity					();
	bool							copyFieldState					(traga2Class *sourceState);
	bool							copyFixedStones					(traga2Class *sourceState);
	bool							addStone						(uintStonePos position,		uintStonePos width,		uintStonePos height,		bool fixed);
	bool							addStone						(traga2Class *sourceField,	uintStoneId stoneId,	uintStonePos destPosition);
	bool							removeStone						(traga2Class *sourceField,							uintStonePos destPosition);
	bool							removeStone						(uintStonePos position);
	void							removeAllStones					();
	bool							moveStone						(fieldState *state, uintStoneId stoneId, uintDirection movingDirection, bool checkMove);
	bool							isMoveAllowed					(fieldState *state, uintStoneId stoneId, uintDirection movingDirection);
	bool							moveStone						(					uintStoneId stoneId, uintDirection movingDirection, bool checkMove);
	bool							isMoveAllowed					(					uintStoneId stoneId, uintDirection movingDirection);

	// solver
	bool							solveProblem					(traga2Class *target, uintStepId &numSteps, arrayStonePos_StepId &stonePos, arrayDirection_StepId &direction);

	// setter
	void							setShowProgressFunc				(function<bool(int, int, wchar_t*)>  showProgressFunc) { showProgress = showProgressFunc; };

	// getter
	uintDirection					getInverseDirection				(uintDirection	direction);
	uintStonePos					getStonePosAfterMove			(uintDirection direction, uintStonePos position);
	uintStonePos					getStonePos						(uintStoneId stoneId)					{ return (stoneId < numStones)?curState->stonePos[stoneId]:fieldSizeX*fieldSizeY; };
	uintStonePos					getFieldWidth					()										{ return fieldSizeX;		};
	uintStonePos					getFieldHeight					()										{ return fieldSizeY;		};
	uintStonePos					getNumStones					()										{ return numStones;			};
	uintStonePos					getNumFreeSquares				()										{ return numFreeSquares;	};
	uintStoneId						getStoneId						(uintStonePos position) 				{ return (position<fieldSizeX*fieldSizeY)?curState->stoneId  [position]:(uintStoneId)   (fieldSizeX*fieldSizeY); };
	uintStoneType					getStoneType					(uintStonePos position) 				{ return (position<fieldSizeX*fieldSizeY)?curState->stoneType[position]:(uintStoneType) (fieldSizeX*fieldSizeY); };
	uintStonePos					getCoordX						(uintStonePos position) 				{ return position % fieldSizeX; };
	uintStonePos					getCoordY						(uintStonePos position) 				{ return position / fieldSizeX; };
	uintStonePos					getPosition						(uintStonePos xPos, uintStonePos yPos)	{ return yPos * fieldSizeX + xPos; };
	uintStonePos					getStoneWidth					(uintStoneType stoneType)				{ return (stoneType<numStoneTypes)?stoneTypes[stoneType].width :0;};
	uintStonePos					getStoneHeight					(uintStoneType stoneType)				{ return (stoneType<numStoneTypes)?stoneTypes[stoneType].height:0;};
};

#endif
