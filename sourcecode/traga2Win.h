/*********************************************************************
	Traga2Win.h													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

//#define DH_USE_DEBUG_HELPER
//#define CHECK_VARPARAM_DETAILS

// own header files
#include "wildWeasel\\wildWeasel.h"
#include "resource.h"
#include "traga2Class.h"
#include "traga2_3D.h"
#include "xml.h"

// Constants
#define										MAX_FIELD_WIDTH						  5								// maximum allowable width of the field
#define										MAX_FIELD_HEIGHT					  5								// maximum allowable height of the field
#define										MAX_STONE_EDGE_LENGTH				  2								// maximum edge length of a stone

// gui variables
wildWeasel::masterMind *					ww									= nullptr;						// contains all the winapi GUII stuff

// global variables
bool										showDisclaimerOnStart				= true;							// when the apüplication is started the first time, then the disclaimer dialog shall be shown
wstring										strDatabaseDir;														// directory path of the database
stoneDesc *									depotPartType						= nullptr;						// types of the depot parts
uintStonePos								defaultEdgeSizeX					= 4;							// default field width
uintStonePos								defaultEdgeSizeY					= 5;							// default field height
traga2_3D_field *							curState							= nullptr;						// current state
traga2_3D_field *							initialState						= nullptr;						// initial state used when starting the search for a solution
traga2_3D_field *							finalState							= nullptr;						// final state which shall be achieved
traga2_3D_field *							targetState							= nullptr;						// target where parts are put, when setting initial/final state
traga2_3D_stoneCluster *					depotParts							= nullptr;						// parts from which the user can select when setting the initial state
traga2_3D_stoneCluster *					targetGrid							= nullptr;						// the parts on the targetState can't be selected by the user directly, but indirectly with this grid instead
traga2_3D_stoneCluster *					selState							= nullptr;						// the field on which the user has clicked and selected one part from
traga2_3D_stoneCluster *					cursorOverState						= nullptr;						// the cursor is currently over this field
traga2_3D_stone *							selStone							= nullptr;						// stone selected by by the user via mouse click 
traga2_3D_stone *							cursorOverStone						= nullptr;						// stone over which the is over
traga2_3D_stone *							stoneAddedAtTargetGridPos			= nullptr;						// the target grid is made of invisible stone/buttons. here the button is stored, at which a real stone was added/removed

// positions & dimensions
wildWeasel::alignment						amupSetStateButtons					= { wildWeasel::alignmentTypeX::BORDER_LEFT, 14.0f, wildWeasel::alignmentTypeY::BORDER_BOTTOM,- 44, wildWeasel::alignmentTypeX::PIXEL_WIDTH, 110, wildWeasel::alignmentTypeY::PIXEL_HEIGHT, 30, wildWeasel::alignmentTypeX::PIXEL_WIDTH, 14, wildWeasel::alignmentTypeY::PIXEL_HEIGHT,  10, 8};
wildWeasel::alignment						amupSolveButtons					= { wildWeasel::alignmentTypeX::BORDER_LEFT, 14.0f, wildWeasel::alignmentTypeY::BORDER_BOTTOM,- 44, wildWeasel::alignmentTypeX::PIXEL_WIDTH, 110, wildWeasel::alignmentTypeY::PIXEL_HEIGHT, 30, wildWeasel::alignmentTypeX::PIXEL_WIDTH, 14, wildWeasel::alignmentTypeY::PIXEL_HEIGHT,  10, 8};
wildWeasel::alignment						amupSolveLabels						= { wildWeasel::alignmentTypeX::BORDER_LEFT, 14.0f, wildWeasel::alignmentTypeY::BORDER_BOTTOM,-134, wildWeasel::alignmentTypeX::PIXEL_WIDTH, 410, wildWeasel::alignmentTypeY::PIXEL_HEIGHT, 30, wildWeasel::alignmentTypeX::PIXEL_WIDTH, 14, wildWeasel::alignmentTypeY::PIXEL_HEIGHT,   0, 1};
wildWeasel::alignment						amupBottomText						= { wildWeasel::alignmentTypeX::BORDER_LEFT, 14.0f, wildWeasel::alignmentTypeY::BORDER_BOTTOM,-134, wildWeasel::alignmentTypeX::PIXEL_WIDTH, 410, wildWeasel::alignmentTypeY::PIXEL_HEIGHT, 20, wildWeasel::alignmentTypeX::PIXEL_WIDTH, 14, wildWeasel::alignmentTypeY::PIXEL_HEIGHT,   5, 1};
POINT										minimumWindowSize					= {1050,  730};					// minimum value
POINT										defaultWindowSize					= {1000, 1000};					// default value
float										cStoneDepth							=  5.0f;						// depth of a stone in space units
wildWeasel::vector2							cMaxBoardSize						= {60.0f, 80.0f};				// width of the board in space units
float										cMaxStoneEdgeLength					= 10.0f;						// the edge length of a stone is calculated upon the field width. this is the maximum size in space units.
vector<float>								loadingScreenFractions;												// fraction of the loading time between each 


// color values (ARGB)
wildWeasel::color							colWindowBackground					= wildWeasel::color::white;
wildWeasel::color							colBottomTextForeground				= wildWeasel::color{164, 164, 220};
wildWeasel::color							colBottomTextBackground				= wildWeasel::color{ 64,  64,  64};
wildWeasel::color							colCurRect							= wildWeasel::color::green;
wildWeasel::color							colNormalRect						= wildWeasel::color::white;
wildWeasel::color							colWoodbox							= wildWeasel::color{200, 200, 200};
wildWeasel::color							colSelRect							= wildWeasel::color::red;
wildWeasel::color							colGray								= wildWeasel::color::blue;

// control handles
wildWeasel::buttonImageFiles				buttonLoadImages					= { L"buttonLoad_normal.png",    1, 0, L"buttonLoad_mouseOver.png",    10, 100, L"buttonLoad_mouseLeave.png",    10, 100, L"buttonLoad_pressed.png",    10, 100, L"buttonLoad_grayedOut.png",    1, 0};
wildWeasel::buttonImageFiles				buttonRand1x1Images					= { L"buttonRand1x1_normal.png", 1, 0, L"buttonRand1x1_mouseOver.png", 10, 100, L"buttonRand1x1_mouseLeave.png", 10, 100, L"buttonRand1x1_pressed.png", 10, 100, L"buttonRand1x1_grayedOut.png", 1, 0};
wildWeasel::buttonImageFiles				buttonSaveImages					= { L"buttonSave_normal.png",    1, 0, L"buttonSave_mouseOver.png",    10, 100, L"buttonSave_mouseLeave.png",    10, 100, L"buttonSave_pressed.png",    10, 100, L"buttonSave_grayedOut.png",    1, 0};
wildWeasel::buttonImageFiles				buttonOkImages						= { L"buttonOk_normal.png",      1, 0, L"buttonOk_mouseOver.png",      10, 100, L"buttonOk_mouseLeave.png",      10, 100, L"buttonOk_pressed.png",      10, 100, L"buttonOk_grayedOut.png",      1, 0};
wildWeasel::buttonImageFiles				buttonCancelImages					= { L"buttonCancel_normal.png",  1, 0, L"buttonCancel_mouseOver.png",  10, 100, L"buttonCancel_mouseLeave.png",  10, 100, L"buttonCancel_pressed.png",  10, 100, L"buttonCancel_grayedOut.png",  1, 0};
wildWeasel::buttonImageFiles				buttonResetImages					= { L"buttonReset_normal.png",   1, 0, L"buttonReset_mouseOver.png",   10, 100, L"buttonReset_mouseLeave.png",   10, 100, L"buttonReset_pressed.png",   10, 100, L"buttonReset_grayedOut.png",   1, 0};
wildWeasel::buttonImageFiles				buttonRandomImages					= { L"buttonRandom_normal.png",  1, 0, L"buttonRandom_mouseOver.png",  10, 100, L"buttonRandom_mouseLeave.png",  10, 100, L"buttonRandom_pressed.png",  10, 100, L"buttonRandom_grayedOut.png",  1, 0};
wildWeasel::buttonImageFiles				buttonCommonImages					= { L"button_Common_normal.png",  1, 0, L"button_Common_mouseOver.png",  10, 100, L"button_Common_mouseLeave.png",  10, 100, L"button_Common_pressed.png",  10, 100, L"button_Common_grayedOut.png",  1, 0};

wildWeasel::buttonImageFiles				buttonSolveQuitImages				= { L"buttonSolveQuit_normal.png",     1, 0, L"buttonSolveQuit_mouseOver.png",     10, 100, L"buttonSolveQuit_mouseLeave.png",     10, 100, L"buttonSolveQuit_pressed.png",     10, 100, L"buttonSolveQuit_grayedOut.png",     1, 0};
wildWeasel::buttonImageFiles				buttonSolvePlayImages				= { L"buttonSolvePlay_normal.png",     1, 0, L"buttonSolvePlay_mouseOver.png",     10, 100, L"buttonSolvePlay_mouseLeave.png",     10, 100, L"buttonSolvePlay_pressed.png",     10, 100, L"buttonSolvePlay_grayedOut.png",     1, 0};
wildWeasel::buttonImageFiles				buttonSolvePauseImages				= { L"buttonSolvePause_normal.png",    1, 0, L"buttonSolvePause_mouseOver.png",    10, 100, L"buttonSolvePause_mouseLeave.png",    10, 100, L"buttonSolvePause_pressed.png",    10, 100, L"buttonSolvePause_grayedOut.png",    1, 0};
wildWeasel::buttonImageFiles				buttonSolveForwardImages			= { L"buttonSolveForward_normal.png",  1, 0, L"buttonSolveForward_mouseOver.png",  10, 100, L"buttonSolveForward_mouseLeave.png",  10, 100, L"buttonSolveForward_pressed.png",  10, 100, L"buttonSolveForward_grayedOut.png",  1, 0};
wildWeasel::buttonImageFiles				buttonSolveBackwardImages			= { L"buttonSolveBackward_normal.png", 1, 0, L"buttonSolveBackward_mouseOver.png", 10, 100, L"buttonSolveBackward_mouseLeave.png", 10, 100, L"buttonSolveBackward_pressed.png", 10, 100, L"buttonSolveBackward_grayedOut.png", 1, 0};
wildWeasel::buttonImageFiles				buttonSolveStopCalcImages			= { L"buttonSolveStopCalc_normal.png", 1, 0, L"buttonSolveStopCalc_mouseOver.png", 10, 100, L"buttonSolveStopCalc_mouseLeave.png", 10, 100, L"buttonSolveStopCalc_pressed.png", 10, 100, L"buttonSolveStopCalc_grayedOut.png", 1, 0};
wildWeasel::buttonImageFiles				buttonSolveFasterImages				= { L"buttonSolveFaster_normal.png",   1, 0, L"buttonSolveFaster_mouseOver.png",   10, 100, L"buttonSolveFaster_mouseLeave.png",   10, 100, L"buttonSolveFaster_pressed.png",   10, 100, L"buttonSolveFaster_grayedOut.png",   1, 0};
wildWeasel::buttonImageFiles				buttonSolveSlowerImages				= { L"buttonSolveSlower_normal.png",   1, 0, L"buttonSolveSlower_mouseOver.png",   10, 100, L"buttonSolveSlower_mouseLeave.png",   10, 100, L"buttonSolveSlower_pressed.png",   10, 100, L"buttonSolveSlower_grayedOut.png",   1, 0};
wildWeasel::buttonImageFiles				buttonSolveRestartImages			= { L"buttonSolveRestart_normal.png",  1, 0, L"buttonSolveRestart_mouseOver.png",  10, 100, L"buttonSolveRestart_mouseLeave.png",  10, 100, L"buttonSolveRestart_pressed.png",  10, 100, L"buttonSolveRestart_grayedOut.png",  1, 0};

wildWeasel::textLabel2D						textStepInTotal, textAnimationSpeed, textNumProcStates, textInfoLevel1, textInfoLevel2, textInfoLevel3;
wildWeasel::plainButton2D					buttonLoad, buttonRand1x1, buttonSave, buttonOk, buttonCancel, buttonReset, buttonRandom, buttonCommon;
wildWeasel::plainButton2D					buttonSolvePlay, buttonSolvePause, buttonSolveForward, buttonSolveBackward, buttonSolveStopCalc, buttonSolveFaster, buttonSolveSlower, buttonSolveRestart;

// textures & bitmaps
wstring										strFontFilename						= L"SegoeUI_18.spritefont";
wstring										strTexturePath						= L"textures";
wstring										strBackgroundFilename				= L"background.jpg";
wstring										strWoodFilename						= L"wood.jpg";
wstring										strWhiteDummyFilename				= L"whiteDummy.bmp";
wildWeasel::texture							textureBackground;
wildWeasel::texture							textureLoading;
wildWeasel::texture							textureWood;
wildWeasel::texture							textureStone;
wildWeasel::font2D							d3dFont2D;
wildWeasel::font3D							d3dFont3D;					
wildWeasel::camera							mainCamera;

// winapi functions
LRESULT CALLBACK							WndProc								(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK							AboutWndProc						(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK							SetFieldSizeWndProc					(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK							SetDirectoryWndProc					(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// init functions
void										loadDefaultSettings					(void);
void										saveDefaultSettings					(void);
void										mainInitFunc						(void* pUser);

// default state
class scenarioSharedObject : public wildWeasel::guiScenario, public wildWeasel::eventFollower
{
public:
	void									init								() override;
	void									release								() override;
	void									activate							() override;
	void									deactivate							() override;
	void									createControls						();
	void									recreateTargetGrid					(traga2_3D_field *state);
	void									recreatePartsDepot					();
	void									loadTextures						();
	void									keyDown								(int keyCode) {};
} scnSharedObjects;

// play game functions
class scenarioPlayingGame : public wildWeasel::guiScenario, public wildWeasel::eventFollower
{
public:
	void									init								() override {};
	void									activate							() override;
	void									deactivate							() override;
	void									keyDown								(int keyCode) override;

private:
	void									cursorHoverStone					(wildWeasel::guiElemEvFol* item, void* pUser);
	void									cursorleftStone						(wildWeasel::guiElemEvFol* item, void* pUser);
} scnPlayGame;

// finishing animation (maybe this should be part of traga2_3D_field)
class scenarioFinishAnimation : public wildWeasel::guiScenario, public wildWeasel::eventFollower
{
public:
	void									init								() override;
	void									activate							() override;
	void									deactivate							() override;
	void									leftMouseButtonPressed				(int xPos, int yPos);
} scnFinishAnimation;

// set state functions
class scenarioSetState : public wildWeasel::guiScenario
{
private:
	void									checkIfSettingStateReady			();

public:
	enum class								modes								{ none, initial, final, unused };

	modes									curMode								= modes::unused;

	//
	void									init								() override {};
	void									activate							() override;
	void									deactivate							() override;

	// helper functions
	void									showSetStateControls				(bool visible);
	void									loadOrSave							(bool dontLoadButSave);
	void									encolorUsedStones					(traga2_3D_stoneCluster* theCluster);
	void									setRandomState						(bool only1x1Stones);

	// button functions	
	void									buttonFuncSetRandomState1x1			(void* pUser);
	void									buttonFuncLoad						(void* pUser);
	void									buttonFuncSave						(void* pUser);
	void									buttonFuncSetRandomState			(void* pUser);
	void									buttonFuncFinishSettingState		(void* pUser);
	void									buttonFuncCancelSettingState		(void* pUser);
	void									buttonFuncResetSettingState			(void* pUser);
	void									buttonFuncSetCommonState			(void* pUser);

	void									hoverTargetGrid						(wildWeasel::guiElemEvFol* item, void* pUser);
	void									leftTargetGrid						(wildWeasel::guiElemEvFol* item, void* pUser);
	void									rightClickOnTargetGrid				(wildWeasel::guiElemEvFol* item, void* pUser);
	void									leftClickOnTargetGrid				(wildWeasel::guiElemEvFol* item, void* pUser);
	void									hoverDepotOrInitialState			(wildWeasel::guiElemEvFol* item, void* pUser);
	void									leftDepotOrInitialState				(wildWeasel::guiElemEvFol* item, void* pUser);
	void									leftClickOnDepotOrInitialState		(wildWeasel::guiElemEvFol* item, void* pUser);

} scnSetState;

// solve functions
class scenarioSolve : public wildWeasel::guiScenario
{
public:
	void									activate							() override;
	void									deactivate							() override;
	void									init								() override;

	// variables
	wildWeasel::timer						timerPlay							;
	DWORD									solutionAnimationSleepDuration		= 0;							// duration in milliseconds between each frame when showing a move of the solution
	DWORD									dwWaitTimeForThreadTermination		= 15000;						// time out in milliseconds until solution search thread is killed, when it does not respond
	DWORD									dwWaitTimeForUnlockSolVars			= 100;
	uintStepId								curShowedStep						= 0;							// current showed move from all solVars.totalNumSteps
	bool									autoForward							= false;						// true if a timer shall call periodically buttonFuncSolveForward()
	HANDLE									hThreadSolve						= nullptr;						// handle of the solving thread
	
	// event handles
	wildWeasel::threadEvent					hEventLockSolutionVars;
	HANDLE									hEventSolutionVarsLocked			= nullptr;
	HANDLE									hEventUnlockSolutionVars			= nullptr;
	HANDLE									hEventTerminateThreadSolve			= nullptr;
	HANDLE									hEventThreadSolveTerminated			= nullptr;

	// solution variables
	struct solutionVariablesStruct																				// this struct is only used for the purpose of grouping variables
	{																											// it contains information of the currently calculated solution
		wstring								strStatus;															// description of current calculation status
		uintStepId							totalNumSteps						= 0;							// total number of moves needed
		unsigned int						numPossibleStates					= 0;							// number of field states
		unsigned int						numProcessedStates					= 0;							// number of already considered states
		arrayStonePos_StepId				solPathStonePos;													// position  of the stone to move within the solution
		arrayDirection_StepId				solPathDirection;													// direction of the stone to move within the solution
		scenarioSolve*						parent								= nullptr;
	} solVars;	

	// helper functions
	static DWORD WINAPI						ThreadProcSolveStatic				(LPVOID lpParameter);
	DWORD WINAPI							ThreadProcSolve						();
	static void								TimerProcPlay						(void* pUser);
	bool									showProgress						(int numPossibleStates, int numProcessedStates, wchar_t* strStatus);
	void									solveCurCtate						();
	void									showSolveControls					(bool visible);
	void									updateShowedSolution				(void* pUser);
	void									terminateSolveThread				();
	void									updateLabels						();

	// button functions
	void									buttonFuncSolvePlay					(void* pUser);
	void									buttonFuncSolvePause				(void* pUser);
	void									buttonFuncSolveForward				(void* pUser);
	void									buttonFuncSolveBackward				(void* pUser);
	void									buttonFuncSolveRestart				(void* pUser);
	void									buttonFuncSolveStopCalc				(void* pUser);
	void									buttonFuncSolveFaster				(void* pUser);
	void									buttonFuncSolveSlower				(void* pUser);	
} scnSolve;