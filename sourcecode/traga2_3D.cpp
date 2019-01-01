/*********************************************************************
	traga2_3D.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "traga2_3D.h"

#pragma region traga2_3D_stoneCluster
//-----------------------------------------------------------------------------
// Name: traga2_3D_stoneCluster()
// Desc: 
//-----------------------------------------------------------------------------
traga2_3D_stoneCluster::traga2_3D_stoneCluster(wildWeasel::masterMind* ww)
{
	wildWeasel::guiElemCluster3D::create(ww);
	this->ww = ww;
}

//-----------------------------------------------------------------------------
// Name: createNewStone()
// Desc: 
//-----------------------------------------------------------------------------
traga2_3D_stone* traga2_3D_stoneCluster::createNewStone()
{
	auto newButton = new traga2_3D_stone();
	newButton->cubicButton::create(ww);
	newButton->stoneCluster	= this;
	guiElemCluster3D::addItem(newButton);
	return newButton;
}
#pragma endregion

#pragma region woodboxClass
//-----------------------------------------------------------------------------
// Name: woodboxClass()
// Desc: 
//-----------------------------------------------------------------------------
woodboxClass::woodboxClass(wildWeasel::masterMind* ww)
{
	wildWeasel::guiElemCluster3D::create(ww);
	auto	newButtons		= new wildWeasel::cubicButton*[numButtons];	

	for (unsigned int i=0; i<numButtons; i++) {
		newButtons[i] = new wildWeasel::cubicButton();
		newButtons[i]->create(ww);
		newButtons[i]->setPositioningMode(wildWeasel::matrixControl3D::matControlMode::matrix);
		addItem(newButtons[i]);
	}
}

//-----------------------------------------------------------------------------
// Name: setDimension()
// Desc: 
//-----------------------------------------------------------------------------
void woodboxClass::setDimension(float edgeLength, float stoneDepth, uintStonePos iFieldWidth, uintStonePos iFieldHeight)
{
	// locals
	float	beamThickness	= 0.3f * edgeLength;
	float	beamDepth		= 1.3f * stoneDepth;
	float	fieldWidth		= (float) iFieldWidth  * edgeLength;
	float	fieldHeight		= (float) iFieldHeight * edgeLength;
	float	bottomHeight	= 0.5f * (beamDepth - stoneDepth);	
	auto    itr				= items.begin();
	wildWeasel::guiElement3D* myElem;
	
	myElem	= (wildWeasel::guiElement3D*) (*itr);	myElem->setScaleRotPos(fieldWidth,						beamThickness, beamDepth, 						0.0f, 0.0f, 0.0f, 		0.0f,  0.5f * (fieldHeight + beamThickness), 0.0f);		itr++; // upper beam
	myElem	= (wildWeasel::guiElement3D*) (*itr);	myElem->setScaleRotPos(fieldWidth,						beamThickness, beamDepth, 						0.0f, 0.0f, 0.0f, 		0.0f, -0.5f * (fieldHeight + beamThickness), 0.0f);		itr++; // lower beam
	myElem	= (wildWeasel::guiElement3D*) (*itr);	myElem->setScaleRotPos(fieldHeight + 2*beamThickness,	beamThickness, beamDepth, 						0.0f, 0.0f, wwc::PI/2, -0.5f * (fieldWidth + beamThickness), 0.0f, 0.0f);		itr++; // left beam
	myElem	= (wildWeasel::guiElement3D*) (*itr);	myElem->setScaleRotPos(fieldHeight + 2*beamThickness,	beamThickness, beamDepth, 						0.0f, 0.0f, wwc::PI/2, 	0.5f * (fieldWidth + beamThickness), 0.0f, 0.0f);		itr++; // right beam
	myElem	= (wildWeasel::guiElement3D*) (*itr);	myElem->setScaleRotPos(fieldWidth  + 2*beamThickness,	fieldHeight  + 2*beamThickness, bottomHeight, 	0.0f, 0.0f, 0.0f, 		0.0f, 0.0f, -0.5f * (stoneDepth + bottomHeight));		itr++; // bottom
}

//-----------------------------------------------------------------------------
// Name: woodboxClass()
// Desc: 
//-----------------------------------------------------------------------------
woodboxClass::~woodboxClass()
{
	deleteAllItems();
}
#pragma endregion

#pragma region traga2_3D_field
//-----------------------------------------------------------------------------
// Name: traga2_3D()
// Desc: 
//-----------------------------------------------------------------------------
traga2_3D_field::traga2_3D_field(uintStonePos newEdgeSizeX, uintStonePos newEdgeSizeY, wildWeasel::masterMind *ww, wildWeasel::texture* theTexture, wildWeasel::font3D* d3dFont3D) : traga2Class(newEdgeSizeX, newEdgeSizeY), traga2_3D_stoneCluster(ww), wildWeasel::clusterAniExplosion(ww, items)
{
	this->stoneIdToButtonMap	= new traga2_3D_stone*[newEdgeSizeX*newEdgeSizeY];
	this->woodBox				= new woodboxClass(ww);
	this->ww					= ww;
	this->field					= this;

	this->setPositioningMode(wildWeasel::matrixControl3D::matControlMode::posRotSca);

	// create a 3D-button for each possible stone
	for (uintStoneId curStone=0; curStone<newEdgeSizeX*newEdgeSizeY; curStone++) {

		// create button 
		auto newButton = new traga2_3D_stone();
		newButton->cubicButton::create(ww);

		// set button properties
		newButton->setTextState (wildWeasel::guiElemState::VISIBLE);
		newButton->setTextColor	(wildWeasel::color::white);
		newButton->setFont		(d3dFont3D);
		newButton->setTexture	(theTexture);
		newButton->setPositioningMode(wildWeasel::matrixControl3D::matControlMode::posRotSca);
		
		// may be newButton should be of type traga2_3D_stone
		newButton->field				= this;
		newButton->stoneCluster			= this;
		newButton->stoneId				= curStone;
		stoneIdToButtonMap[curStone]	= newButton;

		// cluster containing all buttons
		allButtons.push_back(newButton);
	}
}

//-----------------------------------------------------------------------------
// Name: traga2_3D()
// Desc: 
//-----------------------------------------------------------------------------
traga2_3D_field::~traga2_3D_field()
{
	delete [] stoneIdToButtonMap;		stoneIdToButtonMap	= nullptr;
	delete woodBox;						woodBox				= nullptr;
	
	for (auto curItem : allButtons) {
		auto curStone = (traga2_3D_stone*) curItem;
		delete curStone; curStone = nullptr;
	}
	allButtons.clear();
}


//-----------------------------------------------------------------------------
// Name: loadState()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2_3D_field::isTargetReached(traga2_3D_field& target)
{
	return field->curState->isTarget(*target.field->curState, field->fieldSizeX * field->fieldSizeY);
}

//-----------------------------------------------------------------------------
// Name: loadState()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2_3D_field::loadState(wstring& strDatabaseDir, wstring& fileName)
{
	// locals
	wstringstream pathName;

	// set default file name
	pathName << strDatabaseDir << "\\" << fileName << ".sli" << getFieldWidth() << "x" << getFieldHeight();

	// load file
	if (!load(pathName.str())) {
		MessageBoxA(ww->getHwnd(), "Could not load default file.", "Error", MB_OK | MB_ICONSTOP | MB_APPLMODAL);
		return false;
	}

	// update
	adaptButtonsOnSystem();
	return true;
}

//-----------------------------------------------------------------------------
// Name: traga2_3D()
// Desc: 
//-----------------------------------------------------------------------------
void traga2_3D_field::setDimensions(float newEdgeLength, float newStoneDepth, wildWeasel::vector2 maxBoardSize)
{
	edgeLength	= newEdgeLength;
	edgeLength	= std::clamp(edgeLength, 1.0f, maxBoardSize.x / fieldSizeX);
	edgeLength	= std::clamp(edgeLength, 1.0f, maxBoardSize.y / fieldSizeY);
	stoneDepth	= newStoneDepth;
	updateMatrixOfAllButtons();
	woodBox->setDimension(edgeLength, stoneDepth, getFieldWidth(), getFieldHeight());
}

//-----------------------------------------------------------------------------
// Name: adaptButtonsOnSystem()
// Desc: 
//-----------------------------------------------------------------------------
void traga2_3D_field::adaptButtonsOnSystem()
{
	// locals
	wchar_t				str[10];
	uintStoneId			stoneId;
	uintStoneType		stoneType;

	// hide all buttons/items 
	setState(wildWeasel::guiElemState::HIDDEN);
	this->clearItemList();

	// create one 3D-button for each stone, which is not a "free-suare-stone"
	for (auto curButton : allButtons) {
		
		auto curStone = (traga2_3D_stone*) curButton;
		stoneId = curStone->stoneId;

		// unused buttons aren't visible
		if (stoneId >= numStones) {
			curStone->setState(wildWeasel::guiElemState::HIDDEN);
			continue;
		}

		// 
		if (curState->stonePos[stoneId] >= numSquaresInField) continue;

		// free squares aren't visible
		stoneType = curState->stoneType[curState->stonePos[stoneId]];
		if (curState->stonePos[stoneId] >= numSquaresInField || stoneType == ST_FREESQUARE) {
			curStone->setState(wildWeasel::guiElemState::HIDDEN);
			continue;
		}

		// add the relevant stones to the cluster item list
		this->guiElemCluster3D::addItem(curStone);

		// create button 
		wsprintf(str, L"%d", stoneId + (uintStoneId) 1);

		// set button properties
		curStone->setTextState   (wildWeasel::guiElemState::VISIBLE);
		curStone->setTextColor	 (wildWeasel::color::white);
		curStone->setText		 (wstring(str));
	}

	updateMatrixOfAllButtons();
}

//-----------------------------------------------------------------------------
// Name: moveStone()
// Desc: 
//-----------------------------------------------------------------------------
bool traga2_3D_field::moveStone(uintStoneId idStone, uintDirection movingDirection, unsigned int numSteps, DWORD duration, traga2_3D_field* targetState)
{
	// move allowed?
	if (moveStoneVars.curStep>0) stopStoneMovement();
	if (!isMoveAllowed(curState, idStone, movingDirection)) return false;

	moveStoneVars.curStep			= 0;
	moveStoneVars.idStone			= idStone;
	moveStoneVars.movingDirection	= movingDirection;
	moveStoneVars.numSteps			= numSteps;
	moveStoneVars.targetState		= targetState;

	// start moving stone by calling draw positionning function repeatedly
	// ... try matAnimation implementation
	SetTimer(ww->getHwnd(), (UINT_PTR) this, duration / numSteps, &traga2_3D_field::moveStoneEvent);

	return true;
}

//-----------------------------------------------------------------------------
// Name: stopStoneMovement()
// Desc: 
//-----------------------------------------------------------------------------
void traga2_3D_field::stopStoneMovement()
{
	// locals
	traga2_3D_stone*		myButton	= stoneIdToButtonMap[moveStoneVars.idStone];

	// perform move & set old cuberotation
	KillTimer(ww->getHwnd(), (UINT_PTR) this);
	traga2Class::moveStone(curState, moveStoneVars.idStone, moveStoneVars.movingDirection, false);
	calcButtonMatrix(myButton);
	myButton->setDirty();
	moveStoneVars.curStep = 0;

	if (moveStoneVars.targetState != nullptr && field->isTargetReached(*(moveStoneVars.targetState))) {
		myButton->blinkVisibility(3.0f, 10, wildWeasel::guiElemState::DRAWED);
		myButton->setColor(wildWeasel::color::red);
		ww->showMessageBox(L"Finish", L"You reached the final state", MB_OK);
	}
}

//-----------------------------------------------------------------------------
// Name: moveStoneEvent()
// Desc: 
//-----------------------------------------------------------------------------
VOID CALLBACK traga2_3D_field::moveStoneEvent(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	// locals
	traga2_3D_field*		pThis		= (traga2_3D_field*) idEvent;
	traga2_3D_stone*		myButton	= pThis->stoneIdToButtonMap[pThis->moveStoneVars.idStone];
	wildWeasel::vector3		curPosition;

	if (pThis->moveStoneVars.curStep < pThis->moveStoneVars.numSteps) {

		// calc new stone position
		myButton->getPosition(curPosition);
		switch (pThis->moveStoneVars.movingDirection)
		{
		case MV_LEFT:	curPosition.x -= pThis->edgeLength / pThis->moveStoneVars.numSteps; break;
		case MV_RIGHT:	curPosition.x += pThis->edgeLength / pThis->moveStoneVars.numSteps; break;
		case MV_UP:		curPosition.y += pThis->edgeLength / pThis->moveStoneVars.numSteps; break;
		case MV_DOWN:	curPosition.y -= pThis->edgeLength / pThis->moveStoneVars.numSteps; break;
		}

		// apply new matrizes
		myButton->setPosition(&curPosition, false, true);

		// show to user
		pThis->moveStoneVars.curStep++;

	} else {
		pThis->stopStoneMovement();
	}
}

//-----------------------------------------------------------------------------
// Name: calcButtonMatrix()
// Desc: 
//-----------------------------------------------------------------------------
void traga2_3D_field::calcButtonMatrix(traga2_3D_stone* button)
{
	// locals
	wildWeasel::vector3 scale;
	wildWeasel::vector3 rotation;
	wildWeasel::vector3 position;
	wildWeasel::matrix  myMat;
	uintStonePos			stonePos = curState->stonePos[button->stoneId];
	
	// parameters ok?
	if (button == NULL) return;

	// calc position
	position.x = ( 1.0f * (float) (stonePos % fieldSizeX) - (float) fieldSizeX / 2.0f + (float) stones[button->stoneId].width  / 2.0f) * edgeLength;
	position.y = (-1.0f * (float) (stonePos / fieldSizeX) + (float) fieldSizeY / 2.0f - (float) stones[button->stoneId].height / 2.0f) * edgeLength;
	position.z = 0;

	// calc rotation
	rotation.x = rotation.y = rotation.z = 0;

	// calc size
	scale.x = (float) stones[button->stoneId].width  * edgeLength * 0.90f; 
	scale.y = (float) stones[button->stoneId].height * edgeLength * 0.90f; 
	scale.z = stoneDepth;
	
	// scale, rotate, translate
	button->setPosition(&position, false, false);
	button->setRotation(&rotation, false, false);
	button->setScale   (&scale,    false, true);
}

//-----------------------------------------------------------------------------
// Name: updateMatrixOfAllButtons()
// Desc: 
//-----------------------------------------------------------------------------
void traga2_3D_field::updateMatrixOfAllButtons()
{
	for (auto curButton : items) {
		calcButtonMatrix((traga2_3D_stone*) curButton);
	}
}

//-----------------------------------------------------------------------------
// Name: setColor()
// Desc: 
//-----------------------------------------------------------------------------
void traga2_3D_field::setColor(uintStonePos stonePos, wildWeasel::color newColor)
{
	stoneIdToButtonMap[getStoneId(stonePos)]->setColor(newColor);
}
#pragma endregion
