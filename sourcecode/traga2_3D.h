/*********************************************************************\
	traga2_3D.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/
#ifndef TRAGA2_3D_H
#define TRAGA2_3D_H

#include "wildWeasel\\wildWeasel.h"
#include "traga2Class.h"

// predefinition
class traga2_3D_stone;
class traga2_3D_stoneCluster;
class traga2_3D_field;

// // variable type checking
#ifdef CHECK_VARPARAM_DETAILS
	typedef varCheckArray<traga2_3D_stone, uintStoneId>	arrayButton_StoneId;
#else
	typedef traga2_3D_stone **			arrayButton_StoneId;
#endif

// cubic buttons around the stones forming the box around
class woodboxClass : public wildWeasel::guiElemCluster3D
{
public:
	const unsigned int					numButtons						= 5;
										woodboxClass					(wildWeasel::masterMind* ww);
										~woodboxClass					();
	void								setDimension					(float edgeLength, float stoneDepth, uintStonePos iFieldWidth, uintStonePos iFieldHeight);
};

// a single stone
class traga2_3D_stone : public wildWeasel::cubicButton		
{
public:
	uintStoneId							stoneId							= 0;						// id of the stone corresponding to id system in traga2Class
	uintStonePos						stonePos						= 0;						// position of the stone corresponding to id system in traga2Class
	traga2_3D_field *					field							= nullptr;					// the field to which the stone belongs
	traga2_3D_stoneCluster *			stoneCluster					= nullptr;					// != NULL if it belongs to  cluster
	wildWeasel::vector3					vecCurrentVelocity;											// for explosion animation
};

// a group of stones, which are not necessarily a whole field
class traga2_3D_stoneCluster : public wildWeasel::guiElemCluster3D
{
protected:
	wildWeasel::masterMind*				ww								= nullptr;					// used engine providing 3D buttons and corresponding graphics

public:
	traga2_3D_field *					field							= nullptr;					// the field to which the stone belongs

										traga2_3D_stoneCluster			(wildWeasel::masterMind* ww);
	traga2_3D_stone *					createNewStone					();
};

// a whole game consisting of woodbox, stones and underlying traga2Class system
class traga2_3D_field : public traga2Class, public traga2_3D_stoneCluster, public wildWeasel::clusterAniExplosion
{
private:
	struct moveStoneVarsStruct
	{
		uintStoneId						idStone							= 0;
		uintDirection					movingDirection					= 0;
		unsigned int					numSteps						= 0;
		unsigned int					curStep							= 0;
		traga2_3D_field*				targetState						= nullptr;
	} moveStoneVars;

	void								stopStoneMovement				();
	static VOID CALLBACK				moveStoneEvent					(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

public:
	// constructor/destructor
										traga2_3D_field					(uintStonePos newEdgeSizeX, uintStonePos newEdgeSizeY, wildWeasel::masterMind* ww, wildWeasel::texture* theTexture, wildWeasel::font3D* d3dFont3D);
										~traga2_3D_field				();

	// additional variables
	float								edgeLength						= 5;						// length of an edge in space units
	float								stoneDepth						= 2;						// depth in space units
	arrayButton_StoneId					stoneIdToButtonMap				= nullptr;					// 
	woodboxClass*						woodBox							= nullptr;					// graphical opject to render the stone/woodbox
	list<wildWeasel::guiElement*>		allButtons;

	// additional functions
	bool								isTargetReached					(traga2_3D_field& target);
	bool								loadState						(wstring& strDatabaseDir, wstring& fileName);
	bool								moveStone						(uintStoneId idStone, uintDirection movingDirection, unsigned int numSteps, DWORD duration, traga2_3D_field* targetState);
	void								setDimensions					(float newEdgeLength, float newStoneDepth, wildWeasel::vector2 maxBoardSize);
	void								adaptButtonsOnSystem			();
	void								setColor						(uintStonePos stonePos, wildWeasel::color newColor);
	void								calcButtonMatrix				(traga2_3D_stone* button);
	void								updateMatrixOfAllButtons		();
};

#endif
