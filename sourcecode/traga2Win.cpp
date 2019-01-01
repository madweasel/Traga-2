/*********************************************************************
	Traga2Win.cpp													  
 	Copyright (c) Thomas Weber. All rights reserved.				
	Licensed under the MIT License.
	https://github.com/madweasel/madweasels-cpp
\*********************************************************************/

#include "traga2Win.h"

#pragma region mainAndInit
//-----------------------------------------------------------------------------
// Name: _tWinMain()
// Desc: entry function
//-----------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// load default settings
	loadDefaultSettings();

	// create window with wrapper class
	ww = new wildWeasel::masterMind(hInstance, WndProc, IDS_APP_TITLE, IDC_Traga2Win, IDI_Traga2Win, IDC_ARROW, IDC_Traga2Win, IDI_SMALL, IDC_Traga2Win);
	ww->createWindow(defaultWindowSize.x, defaultWindowSize.y, nCmdShow, 0, NULL);

	// show disclaimer
	if (showDisclaimerOnStart) {
		if (IDOK != ww->showMessageBox(L"Disclaimer", L"Please read the legal notice in the file 'LICENSE.txt' distributed with this software. Press 'OK' to agree.", MB_OKCANCEL)) {
			ww->exitProgram();
			return 0;
		}
	}

	// the loading of ressources is done in a seperate thread
	ww->setTexturesPath(strTexturePath);
	d3dFont2D.loadFontFile(ww, strFontFilename);
	d3dFont3D.loadFontFile(ww, strFontFilename);
	textureLoading.loadFile(ww, strWoodFilename);
	textureBackground.loadFile(ww, strBackgroundFilename);
	ww->setMinimumWindowSize(minimumWindowSize.x, minimumWindowSize.y);
	ww->setDummyWhiteTexture(strWhiteDummyFilename);
	if (loadingScreenFractions.size()) ww->mainLoadingScreen.setMeasuredFractions(loadingScreenFractions);
	ww->mainLoadingScreen.setTextures(&textureBackground, &textureLoading, &d3dFont2D);
	ww->mainLoadingScreen.show(mainInitFunc, NULL);
	ww->screenInfo.showFramesPerSecond(false /*true*/, &d3dFont2D);

	// Main message loop:
	ww->goIntoMainLoop();
		
	// release
	saveDefaultSettings();
	delete ww;
	return 0;
}

//-----------------------------------------------------------------------------
// Name: loadDefaultSettings()
// Desc: 
//-----------------------------------------------------------------------------
void loadDefaultSettings(void)
{
	// locals
	xmlNode	*	rootNode	= nullptr;
	xmlClass	xml(string("settings.xml"), rootNode);

	// file loaded?
	if (rootNode != nullptr) {

		// <database directory="">
		strDatabaseDir		= rootNode->node("database")->attribute("directory")->valueW();
	
		// <firstRun showDisclaimer="true">
		showDisclaimerOnStart	= (bool) !(rootNode->node("firstRun")->attribute("showDisclaimer")->valueW().compare(L"false") == 0);

		// <edgeSize x="4" y="5">
		if (rootNode->node("edgeSize")->attribute("x")->exists()) defaultEdgeSizeX	= (uintStonePos) atoi(rootNode->node("edgeSize")->attribute("x")->valueA().c_str());
		if (rootNode->node("edgeSize")->attribute("y")->exists()) defaultEdgeSizeY	= (uintStonePos) atoi(rootNode->node("edgeSize")->attribute("y")->valueA().c_str());

		// <windowSize x="800" y="600"></>
		defaultWindowSize.x = (int) atoi(rootNode->node("windowSize")->attribute("x")->valueA().c_str());
		defaultWindowSize.y = (int) atoi(rootNode->node("windowSize")->attribute("y")->valueA().c_str());

		// loading screen progress fractions
		if (rootNode->node("loadingScreenProgress")->attribute("n")->exists()) {
			loadingScreenFractions.resize((int) atoi(rootNode->node("loadingScreenProgress")->attribute("n")->valueA().c_str()));
			wstringstream wss		(rootNode->node("loadingScreenProgress")->valueW());
			for (auto& curFrac : loadingScreenFractions) {
				wss >> curFrac;
				if (wss.peek() == L';') wss.ignore();
			}
		}
	} else {
		strDatabaseDir		= L"database";
	}
}

//-----------------------------------------------------------------------------
// Name: saveDefaultSettings()
// Desc: 
//-----------------------------------------------------------------------------
void saveDefaultSettings(void)
{
	// locals
	xmlClass		xml;
	xmlNode	*		rootNode		= xml.getRootNode();
	wstring			strRelative;	

	// <database directory="">
	ww->makeRelativePathToExe(strDatabaseDir, strRelative);
	rootNode->addSubNodeWithAttribute(L"database", L"directory", strRelative.c_str());

	// <firstRun showDisclaimer="true">
	rootNode->addSubNodeWithAttribute(L"firstRun", L"showDisclaimer", L"false");
	
	// <edgeSize x="4" y="5">
	rootNode->addSubNodeWithAttribute(L"edgeSize", L"x", curState->getFieldWidth());
	rootNode->addSubNodeWithAttribute(L"edgeSize", L"y", curState->getFieldHeight());

	// <windowSize x="800" y="600"></>
	rootNode->addSubNodeWithAttribute(L"windowSize", L"x", ww->getWindowSizeX());
	rootNode->addSubNodeWithAttribute(L"windowSize", L"y", ww->getWindowSizeY());

	// <loadingScreenProgress n="999">0.1;0.2;1.0;</> 			 (loading screen progress fractions)
	vector<float> fractions;
	wstringstream wss;
	ww->mainLoadingScreen.getMeasuredFractions(fractions);
	for (auto& curFrac : fractions) {
		wss << setprecision(2) << curFrac << L";";
	}
	rootNode->addSubNodeWithAttribute(L"loadingScreenProgress", L"n", fractions.size());
	rootNode->node(L"loadingScreenProgress")->setValue(wss.str().c_str());

	// write to file
	xml.writeFile(string("settings.xml"));
}

//-----------------------------------------------------------------------------
// Name: mainInitFunc()
// Desc: 
//-----------------------------------------------------------------------------
void mainInitFunc(void* pUser)
{
	// default ww stuff
	ww->mainLoadingScreen.setCompletionFraction(0);
	ww->mainLoadingScreen.progress();
	ww->setBackground(strBackgroundFilename);
	ww->cursor3D.init(ww);

	// current set database directory ok?
	ww->checkPathAndModify(strDatabaseDir, wstring(L"databases\\default"));

	// setup
	scnSharedObjects	.init();								ww->mainLoadingScreen.progress();
	scnPlayGame			.init();								ww->mainLoadingScreen.progress();
	scnFinishAnimation	.init();								ww->mainLoadingScreen.progress();
	scnSetState			.init();								ww->mainLoadingScreen.progress();
	scnSolve			.init();								ww->mainLoadingScreen.progress();
	ww->scenarioManager.setActiveScenario(scnSharedObjects);
	
	// debug stuff
	mainCamera.create			(ww);
	mainCamera.setPosition		({  0,  0, 100});
	mainCamera.setLookAt		({  0,  0,   0});
	mainCamera.setFieldOfViewX	(wwc::PI / 2);
	mainCamera.setAsActiveCamera();
	scnSharedObjects.loadTextures();

	// upload resources
	ww->graphicManager.performResourceUpload();
	
	// debug information
	#ifdef DH_USE_DEBUG_HELPER
	RECT rcDH = { 10, 10, 400, 1000 };
	curState->dh = new debugHelper(hWnd, hInst, &rcDH);
	targetState->dh->addArray(0, targetState->curState->stoneType,	DHC_UINT,  1,  1, targetState->getFieldWidth(), targetState->getFieldHeight()*targetState->getFieldWidth(), "curState->stoneType[]", 2);
	targetState->dh->addArray(1, targetState->curState->stoneId,	DHC_UINT,  1, 15, targetState->getFieldWidth(), targetState->getFieldHeight()*targetState->getFieldWidth(), "curState->stoneId[]",   2);
	targetState->dh->addArray(3, targetState->curState->stonePos,	DHC_UINT,  30,15, targetState->getFieldWidth(), targetState->getFieldHeight()*targetState->getFieldWidth(), "curState->stonePos[]",  2);
	targetState->dh->updateDebugHelper();
	curState->dh->addArray(0, curState->curState->stoneType, DHC_UINT,  1,  1, curState->getFieldWidth(), curState->getFieldHeight()*curState->getFieldWidth(), "stoneType[]", 2);
	curState->dh->addArray(1, curState->curState->stoneId,   DHC_UINT,  1, 10, curState->getFieldWidth(), curState->getFieldHeight()*curState->getFieldWidth(), "stoneId[]",   2);
	#endif

	// show render rect
	ww->mainLoadingScreen.setCompletionFraction(3);
}

//-----------------------------------------------------------------------------
// Name: WndProc()
// Desc: Processes messages for the main window.
//-----------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// locals
	int wmId, wmEvent;

	wstring strReason;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(ww->getHinst(), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutWndProc);
			break;
		case IDM_PRIVACY:		ww->showMessageBox(L"Privacy", L"This application does not store nor send personal data.", MB_OK);			break;		
		case IDM_LICENSE:		ww->showMessageBox(L"License", L"\
                               The MIT License (MIT)\n\
\n\
Copyright (c) 2019 Thomas Weber\n\
\n\
Permission is hereby granted, free of charge, to any person obtaining a copy of this\
software and associated documentation files (the 'Software'), to deal in the Software\
without restriction, including without limitation the rights to use, copy, modify,\
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to\
permit persons to whom the Software is furnished to do so, subject to the following\
conditions:\n\
\n\
The above copyright notice and this permission notice shall be included in all copies\
or substantial portions of the Software.\n\
\n\
THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,\
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A\
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT\
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF\
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE\
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.", MB_OK); break;
		case ID_SET_SETBOARDSIZE:
			DialogBox(ww->getHinst(), MAKEINTRESOURCE(IDD_FIELDSIZE), hWnd, SetFieldSizeWndProc);
			break;
		case ID_SET_SETDATABASEDIRECTORY:
			DialogBox(ww->getHinst(), MAKEINTRESOURCE(IDD_DIRECTORY), hWnd, SetDirectoryWndProc);
			break;
		case ID_GAME_PLAY:		
			ww->scenarioManager.setActiveScenario(scnPlayGame);
			break;
		case ID_GAME_STARTFINISHANIMATION:
			ww->scenarioManager.setActiveScenario(scnFinishAnimation);
			break;
		case ID_SET_SETINITIALSTATE:
			scnSetState.curMode = scenarioSetState::modes::initial;
			ww->scenarioManager.setActiveScenario(scnSetState);
			break;
		case ID_SET_SETFINALSTATE:
			scnSetState.curMode = scenarioSetState::modes::final;
			ww->scenarioManager.setActiveScenario(scnSetState);
			break;
		case ID_GAME_SOLVE:
			ww->scenarioManager.setActiveScenario(scnSolve);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			break;
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Name: SetDirectoryWndProc()
// Desc: Message handler for dialog box.
//-----------------------------------------------------------------------------
INT_PTR CALLBACK SetDirectoryWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// locals
	UNREFERENCED_PARAMETER(lParam);
	WCHAR					tmpStr[MAX_PATH];
	wstring					folderPath;
	
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT_DIRECTORY, strDatabaseDir.c_str());
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_DIRECTORY:
			if (ww->browseForFolder(folderPath, L"Select database folder...")) {
				SetDlgItemText(hDlg, IDC_EDIT_DIRECTORY, folderPath.c_str());
			}
			break;
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT_DIRECTORY, tmpStr, MAX_PATH-1);
			if (ww->isFolderPathValid(tmpStr)) {
				strDatabaseDir.assign(tmpStr);
			} else {
				ww->showMessageBox(L"ERROR", L"The selected path is invalid.", MB_OK);
				break;
			}
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		}
	}
	return (INT_PTR)FALSE;
}

//-----------------------------------------------------------------------------
// Name: SetFieldSizeWndProc()
// Desc: Message handler for dialog box.
//-----------------------------------------------------------------------------
INT_PTR CALLBACK SetFieldSizeWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// locals
	UNREFERENCED_PARAMETER(lParam);
	char	tmpStr1[MAX_PATH], tmpStr2[MAX_PATH];
	int		newWidth, newHeight;
	
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemTextA(hDlg, IDC_EDIT_WIDTH,  "4");
		SetDlgItemTextA(hDlg, IDC_EDIT_HEIGHT, "5");
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemTextA(hDlg, IDC_EDIT_WIDTH,  tmpStr1, MAX_PATH-1);
			GetDlgItemTextA(hDlg, IDC_EDIT_HEIGHT, tmpStr2, MAX_PATH-1);
			newWidth  = atoi(tmpStr1);
			newHeight = atoi(tmpStr2);
			if (newWidth  >= 2 && newWidth  <= MAX_FIELD_WIDTH
			&&  newHeight >= 2 && newHeight <= MAX_FIELD_HEIGHT) {
				// ... könnte durch liste (add, delete) besser gelöst werden!
				delete initialState;	initialState= new traga2_3D_field(newWidth, newHeight, ww, &textureStone, &d3dFont3D);
				delete finalState;		finalState  = new traga2_3D_field(newWidth, newHeight, ww, &textureStone, &d3dFont3D);
				delete curState;		curState	= new traga2_3D_field(newWidth, newHeight, ww, &textureStone, &d3dFont3D);
				delete targetState;		targetState	= new traga2_3D_field(newWidth, newHeight, ww, &textureStone, &d3dFont3D);
				initialState->loadState(strDatabaseDir, wstring(L"\\initialState"));
				finalState  ->loadState(strDatabaseDir, wstring(L"\\finalState"));
				ww->scenarioManager.setActiveScenario(scnSharedObjects);
				scnSharedObjects.recreatePartsDepot();
				scnSharedObjects.recreateTargetGrid(curState);
				ww->graphicManager.performResourceUpload();
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			} else {
				sprintf(tmpStr1, "The width must be between 2 and %d,\nthe height between 2 and %d.", MAX_FIELD_WIDTH, MAX_FIELD_HEIGHT);
				MessageBoxA(hDlg, tmpStr1, "Error", MB_OK);
			}
			break;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		}
	}
	return (INT_PTR)FALSE;
}

//-----------------------------------------------------------------------------
// Name: WndProc()
// Desc: Message handler for about box.
//-----------------------------------------------------------------------------
INT_PTR CALLBACK AboutWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
#pragma endregion

/************************************************************************************************************************************\
  scenarioSharedObject
\************************************************************************************************************************************/

#pragma region shared objects

//-----------------------------------------------------------------------------
// Name: scenarioSharedObject::init()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSharedObject::init()
{
	loadTextures();					ww->mainLoadingScreen.progress();
	createControls();				ww->mainLoadingScreen.progress();
	activate();						ww->mainLoadingScreen.progress();
	recreatePartsDepot();			ww->mainLoadingScreen.progress();
	recreateTargetGrid(curState);	ww->mainLoadingScreen.progress();

	// Set Camera
	mainCamera.create			(ww);
	mainCamera.setPosition		({  0,  0, 100});
	mainCamera.setLookAt		({  0,  0,   0});
	mainCamera.setFieldOfViewX	(wwc::PI / 2);
	mainCamera.setAsActiveCamera();

	// Set Cursor (coordinates are relative to the camera)
	ww->cursor3D.setCursorVisibility(false);
	ww->cursor3D.setScale(1, 1, 1, false, false);
}

//-----------------------------------------------------------------------------
// Name: scenarioSharedObject::release()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSharedObject::release()
{
	delete curState, initialState, finalState, targetState;
	delete depotParts, targetGrid, depotPartType;
}

//-----------------------------------------------------------------------------
// Name: scenarioSharedObject::activate()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSharedObject::activate()
{
	cursorOverState				= nullptr;
	cursorOverStone				= nullptr;
	selState					= nullptr;
	selStone					= nullptr;
	stoneAddedAtTargetGridPos	= nullptr;

	curState	->copyFieldState(initialState);

	curState	->adaptButtonsOnSystem();
	initialState->adaptButtonsOnSystem();
	finalState	->adaptButtonsOnSystem();
	targetState	->adaptButtonsOnSystem();

	curState	->setDimensions(cMaxStoneEdgeLength, cStoneDepth, cMaxBoardSize);
	initialState->setDimensions(cMaxStoneEdgeLength, cStoneDepth, cMaxBoardSize);
	finalState	->setDimensions(cMaxStoneEdgeLength, cStoneDepth, cMaxBoardSize);
	targetState	->setDimensions(cMaxStoneEdgeLength, cStoneDepth, cMaxBoardSize);

	curState	->setTexture(&textureStone);
	initialState->setTexture(&textureStone);
	finalState	->setTexture(&textureStone);
	targetState	->setTexture(&textureStone);

	curState	->setButtonColor(colNormalRect); 
	initialState->setButtonColor(colNormalRect); 
	finalState	->setButtonColor(colNormalRect); 
	targetState	->setButtonColor(colNormalRect); 
	depotParts	->setButtonColor(colNormalRect);

	curState	->setState(wildWeasel::guiElemState::VISIBLE);
	initialState->setState(wildWeasel::guiElemState::VISIBLE);
	finalState	->setState(wildWeasel::guiElemState::VISIBLE);
	targetState	->setState(wildWeasel::guiElemState::HIDDEN);
	depotParts	->setState(wildWeasel::guiElemState::HIDDEN);
	targetGrid	->setState(wildWeasel::guiElemState::HIDDEN);

	curState	->setPosition(  0,-10, 0,	false, false);
	initialState->setPosition(-60, 30, 0,   false, false);
	finalState	->setPosition( 60, 30, 0,   false, false);

	curState	->setScale(1.0f, 1.0f, 1.0f, false, true);
	initialState->setScale(0.5f, 0.5f, 0.5f, false, true);
	finalState	->setScale(0.5f, 0.5f, 0.5f, false, true);

	curState	->woodBox->setPositionByCluster(curState);
	initialState->woodBox->setPositionByCluster(initialState);
	finalState	->woodBox->setPositionByCluster(finalState);
	
	curState	->woodBox->setScaleByCluster   (curState);
	initialState->woodBox->setScaleByCluster   (initialState);
	finalState	->woodBox->setScaleByCluster   (finalState);
		
	targetState	->woodBox->setTexture(&textureWood);
	curState	->woodBox->setTexture(&textureWood);
	initialState->woodBox->setTexture(&textureWood);
	finalState	->woodBox->setTexture(&textureWood);

	targetState	->woodBox->setButtonColor(colWoodbox);
	curState	->woodBox->setButtonColor(colWoodbox);
	initialState->woodBox->setButtonColor(colWoodbox);
	finalState	->woodBox->setButtonColor(colWoodbox);

	targetState	->woodBox->setState(wildWeasel::guiElemState::HIDDEN);
	curState	->woodBox->setState(wildWeasel::guiElemState::VISIBLE);
	initialState->woodBox->setState(wildWeasel::guiElemState::VISIBLE);
	finalState	->woodBox->setState(wildWeasel::guiElemState::VISIBLE);

	textInfoLevel1.setText(L"Move cursor over stone and press arrow keys to move it.");
	textInfoLevel1.setTextState(wildWeasel::guiElemState::DRAWED);
}

//-----------------------------------------------------------------------------
// Name: scenarioSharedObject::deactivate()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSharedObject::deactivate()
{
	textInfoLevel1.setTextState(wildWeasel::guiElemState::HIDDEN);

	cursorOverState				= nullptr;
	cursorOverStone				= nullptr;
	selState					= nullptr;
	selStone					= nullptr;
	stoneAddedAtTargetGridPos	= nullptr;
}

//-----------------------------------------------------------------------------
// Name: scenarioSharedObject::loadTextures()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSharedObject::loadTextures()
{
	textureStone.loadFile(ww, strWoodFilename);
	textureWood .loadFile(ww, strWoodFilename);
}

//-----------------------------------------------------------------------------
// Name: createControls()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSharedObject::createControls()
{
	// create principal objects
	initialState	= new traga2_3D_field(defaultEdgeSizeX, defaultEdgeSizeY, ww, &textureStone, &d3dFont3D);		ww->mainLoadingScreen.progress();
	finalState		= new traga2_3D_field(defaultEdgeSizeX, defaultEdgeSizeY, ww, &textureStone, &d3dFont3D);		ww->mainLoadingScreen.progress();
	curState		= new traga2_3D_field(defaultEdgeSizeX, defaultEdgeSizeY, ww, &textureStone, &d3dFont3D);		ww->mainLoadingScreen.progress();
	targetState		= new traga2_3D_field(defaultEdgeSizeX, defaultEdgeSizeY, ww, &textureStone, &d3dFont3D);		ww->mainLoadingScreen.progress();
	depotParts		= new traga2_3D_stoneCluster(ww);																ww->mainLoadingScreen.progress();
	targetGrid		= new traga2_3D_stoneCluster(ww);																ww->mainLoadingScreen.progress();
	
	// load default states from database
	initialState->loadState(strDatabaseDir, wstring(L"\\initialState"));											ww->mainLoadingScreen.progress();
	finalState  ->loadState(strDatabaseDir, wstring(L"\\finalState"));												ww->mainLoadingScreen.progress();
	
	// create buttons
	buttonLoad			.create(ww, buttonLoadImages			, bind(&scenarioSetState::buttonFuncLoad				, &scnSetState, placeholders::_1), nullptr, 0);
	buttonSave			.create(ww, buttonSaveImages			, bind(&scenarioSetState::buttonFuncSave				, &scnSetState, placeholders::_1), nullptr, 0);
	buttonRand1x1		.create(ww, buttonRand1x1Images			, bind(&scenarioSetState::buttonFuncSetRandomState1x1	, &scnSetState, placeholders::_1), nullptr, 0);
	buttonRandom		.create(ww, buttonRandomImages			, bind(&scenarioSetState::buttonFuncSetRandomState		, &scnSetState, placeholders::_1), nullptr, 0);
	buttonOk			.create(ww, buttonOkImages				, bind(&scenarioSetState::buttonFuncFinishSettingState	, &scnSetState, placeholders::_1), nullptr, 0);
	buttonCancel		.create(ww, buttonCancelImages			, bind(&scenarioSetState::buttonFuncCancelSettingState	, &scnSetState, placeholders::_1), nullptr, 0);
	buttonReset			.create(ww, buttonResetImages			, bind(&scenarioSetState::buttonFuncResetSettingState	, &scnSetState, placeholders::_1), nullptr, 0);
	buttonCommon		.create(ww, buttonCommonImages			, bind(&scenarioSetState::buttonFuncSetCommonState		, &scnSetState, placeholders::_1), nullptr, 0);

	buttonSolvePlay		.create(ww, buttonSolvePlayImages		, bind(&scenarioSolve::buttonFuncSolvePlay				, &scnSolve, placeholders::_1), nullptr, 0);
	buttonSolvePause	.create(ww, buttonSolvePauseImages		, bind(&scenarioSolve::buttonFuncSolvePause				, &scnSolve, placeholders::_1), nullptr, 0);
	buttonSolveForward	.create(ww, buttonSolveForwardImages	, bind(&scenarioSolve::buttonFuncSolveForward			, &scnSolve, placeholders::_1), nullptr, 0);
	buttonSolveBackward	.create(ww, buttonSolveBackwardImages	, bind(&scenarioSolve::buttonFuncSolveBackward			, &scnSolve, placeholders::_1), nullptr, 0);
	buttonSolveStopCalc	.create(ww, buttonSolveStopCalcImages	, bind(&scenarioSolve::buttonFuncSolveStopCalc			, &scnSolve, placeholders::_1), nullptr, 0);
	buttonSolveFaster	.create(ww, buttonSolveFasterImages		, bind(&scenarioSolve::buttonFuncSolveFaster			, &scnSolve, placeholders::_1), nullptr, 0);
	buttonSolveSlower	.create(ww, buttonSolveSlowerImages		, bind(&scenarioSolve::buttonFuncSolveSlower			, &scnSolve, placeholders::_1), nullptr, 0);
	buttonSolveRestart	.create(ww, buttonSolveRestartImages	, bind(&scenarioSolve::buttonFuncSolveRestart			, &scnSolve, placeholders::_1), nullptr, 0);

	// ... work around so that textures for 'buttonSolveQuitImages' are also processed in the performResourceUpload() function. This should be done automatically by the wildWeasel library.
	buttonSolveStopCalc.setImageFiles(buttonSolveQuitImages);
	buttonSolveStopCalc.setImageFiles(buttonSolveStopCalcImages);

	textStepInTotal		.create(ww, wstring(L""), &d3dFont2D, 0);
	textNumProcStates	.create(ww, wstring(L""), &d3dFont2D, 0);
	textAnimationSpeed	.create(ww, wstring(L""), &d3dFont2D, 0);
	textInfoLevel1		.create(ww, wstring(L""), &d3dFont2D, 0);
	textInfoLevel2		.create(ww, wstring(L""), &d3dFont2D, 0);
	textInfoLevel3		.create(ww, wstring(L""), &d3dFont2D, 0);
	
	textInfoLevel1		.setTextColor(colBottomTextForeground);
	textInfoLevel2		.setTextColor(colBottomTextForeground);
	textInfoLevel3		.setTextColor(colBottomTextForeground);

	buttonLoad			.setAlignment(amupSetStateButtons,	0);
	buttonSave			.setAlignment(amupSetStateButtons,	1);
	buttonRand1x1		.setAlignment(amupSetStateButtons,	2);
	buttonRandom		.setAlignment(amupSetStateButtons,	3);
	buttonOk			.setAlignment(amupSetStateButtons,	4);
	buttonCancel		.setAlignment(amupSetStateButtons,	5);
	buttonReset			.setAlignment(amupSetStateButtons,	6);
	buttonCommon		.setAlignment(amupSetStateButtons,	7);

	buttonSolvePlay		.setAlignment(amupSolveButtons,		0);
	buttonSolvePause	.setAlignment(amupSolveButtons,		1);
	buttonSolveForward	.setAlignment(amupSolveButtons,		2);
	buttonSolveBackward	.setAlignment(amupSolveButtons,		3);
	buttonSolveRestart	.setAlignment(amupSolveButtons,		4);
	buttonSolveFaster	.setAlignment(amupSolveButtons,		5);
	buttonSolveSlower	.setAlignment(amupSolveButtons,		6);
	buttonSolveStopCalc	.setAlignment(amupSolveButtons,		7);

	textStepInTotal		.setAlignment(amupSolveLabels,		0);
	textNumProcStates	.setAlignment(amupSolveLabels,		1);
	textAnimationSpeed	.setAlignment(amupSolveLabels,		2);

	textInfoLevel1		.setAlignment(amupBottomText,		0);
	textInfoLevel2		.setAlignment(amupBottomText,		1);
	textInfoLevel3		.setAlignment(amupBottomText,		2);
}

//-----------------------------------------------------------------------------
// Name: recreateTargetGrid()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSharedObject::recreateTargetGrid(traga2_3D_field *state)
{
	// locals
	unsigned int 	x, y;
	unsigned int 	stonesInX		= state->getFieldWidth();
	unsigned int 	stonesInY		= state->getFieldHeight();
	float			xPos, yPos;

	targetGrid->deleteAllItems		();
	targetGrid->setPositionByCluster(state);
	targetGrid->setViewByCluster    (state);
	targetGrid->field	 = NULL;

	for (x=0; x<stonesInX;x++) { for (y=0; y<stonesInY;y++) { 
		auto newButton = targetGrid->createNewStone();
		xPos = ( 1.0f * x - stonesInX / 2.0f + 1 / 2.0f) * state->edgeLength;
		yPos = (-1.0f * y + stonesInY / 2.0f - 1 / 2.0f) * state->edgeLength;
		newButton->setScaleRotPos(state->edgeLength, state->edgeLength, state->stoneDepth, 0.0f, 0.0f, 0.0f, xPos, yPos, 0.0f);
		newButton->stonePos	= finalState->traga2Class::getPosition(x, y);
	}}

	targetGrid->setState		(wildWeasel::guiElemState::HIDDEN);
	targetGrid->setButtonColor	(colNormalRect);
	targetGrid->setTexture		(NULL); 
}

//-----------------------------------------------------------------------------
// Name: recreatePartsDepot()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSharedObject::recreatePartsDepot()
{
	// locals
	float			edgeLength		= cMaxStoneEdgeLength;
	float			stoneDepth		= cStoneDepth;
	int				numDeposParts	= 4;

	if (depotPartType != NULL) {
		delete [] depotPartType;		
		depotPartType = nullptr;
	}
	depotParts->deleteAllItems();
	depotParts->setPosition(0, 0, 0, false, true);
	depotParts->field	 = NULL;

	depotPartType	= new stoneDesc[numDeposParts];
	auto newButtons = new traga2_3D_stone*[numDeposParts];

	for (auto i = 0; i < numDeposParts; i++) {
		newButtons[i] = depotParts->createNewStone();
		newButtons[i]->stoneId = i;
	}

	depotPartType[0].width = 1;		depotPartType[0].height = 1;
	depotPartType[1].width = 2;		depotPartType[1].height = 1;
	depotPartType[2].width = 1;		depotPartType[2].height = 2;
	depotPartType[3].width = 2;		depotPartType[3].height = 2;

	newButtons[0]->setScaleRotPos(1*edgeLength, 1*edgeLength, stoneDepth, 		0.0f, 0.0f, 0.0f, 	0.0f,  4*edgeLength, 0.0f);	
	newButtons[1]->setScaleRotPos(2*edgeLength, 1*edgeLength, stoneDepth, 		0.0f, 0.0f, 0.0f, 	0.0f,  2*edgeLength, 0.0f);	
	newButtons[2]->setScaleRotPos(1*edgeLength, 2*edgeLength, stoneDepth, 		0.0f, 0.0f, 0.0f, 	0.0f,  0*edgeLength, 0.0f);	
	newButtons[3]->setScaleRotPos(2*edgeLength, 2*edgeLength, stoneDepth, 		0.0f, 0.0f, 0.0f, 	0.0f, -3*edgeLength, 0.0f);	

	depotParts->setState		(wildWeasel::guiElemState::HIDDEN);
	depotParts->setButtonColor	(colNormalRect);
	depotParts->setTexture		(&textureWood); 

	delete newButtons;
}

#pragma endregion

/************************************************************************************************************************************\
  set state mode
\************************************************************************************************************************************/

#pragma region set state mode
//-----------------------------------------------------------------------------
// Name: activate()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::activate()
{
	// locals
	unsigned int	position, stoneType;
	bool			show1x1		= true;

	scnSharedObjects.activate();

	// set state
	if (curMode == modes::initial) {
		initialState->setState(wildWeasel::guiElemState::HIDDEN);
		initialState->woodBox->setState(wildWeasel::guiElemState::HIDDEN);
		targetState ->copyFieldState(initialState);
		depotParts	->setState(wildWeasel::guiElemState::DRAWED);
		depotParts	->setPosition(-40, 0, 10, false, true);
	} else if (curMode == modes::final) {
		initialState->woodBox->setState(wildWeasel::guiElemState::VISIBLE);
		initialState->adaptButtonsOnSystem();
		initialState->setState(wildWeasel::guiElemState::DRAWED);
		initialState->setButtonColor(colNormalRect);
		initialState->setPosition(-35,0,0, false, false);
		initialState->setScale(1.0f, 1.0f, 1.0f, false, true);
		initialState->woodBox->setPositionByCluster(initialState);
		initialState->woodBox->setScaleByCluster   (initialState);
		targetState->removeAllStones();
		targetState->copyFieldState(finalState);
		depotParts ->setState(wildWeasel::guiElemState::HIDDEN);
		encolorUsedStones(NULL);
	}
	
	// position objects
	targetState->adaptButtonsOnSystem();
	targetState->setState(wildWeasel::guiElemState::VISIBLE);
	targetState->setButtonColor(colNormalRect);
	targetState	->setPosition( 35, 0,  0, false, true);
	targetState	->woodBox->setPositionByCluster(targetState);
	targetGrid	->setPositionByCluster(targetState);
	targetGrid	->setViewByCluster	  (targetState);

	// visibility
	curState	->setState(wildWeasel::guiElemState::HIDDEN);
	finalState	->setState(wildWeasel::guiElemState::HIDDEN);
	targetState	->setState(wildWeasel::guiElemState::VISIBLE);
	targetGrid	->setState(wildWeasel::guiElemState::INVISIBLE);
	targetState	->woodBox->setState(wildWeasel::guiElemState::VISIBLE);
	curState	->woodBox->setState(wildWeasel::guiElemState::HIDDEN);
	finalState	->woodBox->setState(wildWeasel::guiElemState::HIDDEN);
	
	// redraw everything
	showSetStateControls(true);
	buttonOk	.setState(wildWeasel::guiElemState::GRAYED);
	buttonSave	.setState(wildWeasel::guiElemState::GRAYED);

	// don't show random 1x1 button if 
	if (curMode == modes::final) {
		
		for (position=0; position<initialState->getFieldHeight()*initialState->getFieldWidth(); position++) {
			stoneType = initialState->getStoneType(position);

			if ((unsigned int) initialState->getStoneWidth(stoneType) > 1 || (unsigned int) initialState->getStoneHeight(stoneType) > 1) {
				show1x1 = false;
				break;
			}
		}
	}
	buttonRand1x1.setState(show1x1 ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::HIDDEN);

	textInfoLevel1.setText(wstring(L"Press left mousebutton to select a stone type from the depot on the left side."));	textInfoLevel1.setTextState(wildWeasel::guiElemState::DRAWED);
	textInfoLevel2.setText(wstring(L"Press left mousebutton on the right wooden box to place a stone."));				textInfoLevel2.setTextState(wildWeasel::guiElemState::DRAWED);
	textInfoLevel3.setText(wstring(L"Press left mousebutton to remove a stone from the wooden box."));					textInfoLevel3.setTextState(wildWeasel::guiElemState::DRAWED);

	// catch user input
	for (auto& curItem : targetGrid->getContainer<traga2_3D_stone>()) {	 
		curItem->assignOnRightMouseButtonPressed(bind(&scenarioSetState::rightClickOnTargetGrid,		&scnSetState, placeholders::_1, placeholders::_2), this);
		curItem->assignOnLeftMouseButtonPressed (bind(&scenarioSetState::leftClickOnTargetGrid,			&scnSetState, placeholders::_1, placeholders::_2), this);
		curItem->assignOnMouseEnteredRegion     (bind(&scenarioSetState::hoverTargetGrid,				&scnSetState, placeholders::_1, placeholders::_2), this);
		curItem->assignOnMouseLeftRegion        (bind(&scenarioSetState::leftTargetGrid,				&scnSetState, placeholders::_1, placeholders::_2), this);
	}
	for (auto& curItem : depotParts->getContainer<traga2_3D_stone>()) {	 
		curItem->assignOnLeftMouseButtonPressed (bind(&scenarioSetState::leftClickOnDepotOrInitialState,&scnSetState, placeholders::_1, placeholders::_2), this);
		curItem->assignOnMouseEnteredRegion     (bind(&scenarioSetState::hoverDepotOrInitialState,		&scnSetState, placeholders::_1, placeholders::_2), this);
		curItem->assignOnMouseLeftRegion        (bind(&scenarioSetState::leftDepotOrInitialState,		&scnSetState, placeholders::_1, placeholders::_2), this);
	}
	for (auto& curItem : initialState->getContainer<traga2_3D_stone>()) {	 
		curItem->assignOnLeftMouseButtonPressed (bind(&scenarioSetState::leftClickOnDepotOrInitialState,&scnSetState, placeholders::_1, placeholders::_2), this);
		curItem->assignOnMouseEnteredRegion     (bind(&scenarioSetState::hoverDepotOrInitialState,		&scnSetState, placeholders::_1, placeholders::_2), this);
		curItem->assignOnMouseLeftRegion        (bind(&scenarioSetState::leftDepotOrInitialState,		&scnSetState, placeholders::_1, placeholders::_2), this);
	}
}

//-----------------------------------------------------------------------------
// Name: deactivate()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::deactivate()
{
	textInfoLevel1.setTextState(wildWeasel::guiElemState::HIDDEN);
	textInfoLevel2.setTextState(wildWeasel::guiElemState::HIDDEN);
	textInfoLevel3.setTextState(wildWeasel::guiElemState::HIDDEN);

	showSetStateControls(false);

	// deactivate catching user input
	for (auto& curItem : targetGrid->getContainer<traga2_3D_stone>()) {
		curItem->assignOnRightMouseButtonPressed(nullptr, this);
		curItem->assignOnLeftMouseButtonPressed (nullptr, this);
		curItem->assignOnMouseEnteredRegion     (nullptr, this);
		curItem->assignOnMouseLeftRegion        (nullptr, this);
	}
	for (auto& curItem : depotParts->getContainer<traga2_3D_stone>()) {	 
		curItem->assignOnLeftMouseButtonPressed (nullptr, this);
		curItem->assignOnMouseEnteredRegion     (nullptr, this);
		curItem->assignOnMouseLeftRegion        (nullptr, this);
	}
	for (auto& curItem : initialState->getContainer<traga2_3D_stone>()) {	 
		curItem->assignOnLeftMouseButtonPressed (nullptr, this);
		curItem->assignOnMouseEnteredRegion     (nullptr, this);
		curItem->assignOnMouseLeftRegion        (nullptr, this);
	}

	curMode			= modes::none;
}

//-----------------------------------------------------------------------------
// Name: buttonFuncCancelSettingState()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::buttonFuncCancelSettingState(void* pUser)
{
	// locals
	ww->scenarioManager.setActiveScenario(scnSharedObjects);
}

//-----------------------------------------------------------------------------
// Name: showSetStateControls()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::showSetStateControls(bool visible)
{
	// locals
	wildWeasel::guiElemState	nCmdShow = visible ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::HIDDEN;

	buttonLoad		.setState(nCmdShow);
	buttonSave		.setState(nCmdShow);
	buttonRandom	.setState(nCmdShow);
	buttonOk		.setState(nCmdShow);
	buttonCancel	.setState(nCmdShow);
	buttonReset		.setState(nCmdShow);
	buttonCommon	.setState(wildWeasel::guiElemState::HIDDEN);
	buttonRand1x1	.setState(nCmdShow);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSetRandomState1x1()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::buttonFuncSetRandomState1x1(void* pUser)	
{
	setRandomState(true);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncLoad()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::buttonFuncLoad(void* pUser)	
{	
	loadOrSave(false);			
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSave()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::buttonFuncSave(void* pUser)	
{
	loadOrSave(true);			
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSetRandomState()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::buttonFuncSetRandomState	(void* pUser)	
{	
	setRandomState(false);	
}

//-----------------------------------------------------------------------------
// Name: loadOrSave()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::loadOrSave(bool dontLoadButSave)
{
	// locals
	wstringstream	ssExtFilter;
	wstringstream	ssExt;
	wstring			strExtFilter;
	wstring			strExt;
	wstring			filePath;

	unsigned int	height = targetState->getFieldHeight();
	unsigned int	width  = targetState->getFieldWidth();
	
	// make file extension filter
	ssExt		<< "sli" << width << "x" << height;																				strExt		 = ssExt.str();
	ssExtFilter << width << "x" << height << "-sliding game file (*." << strExt << ")" << '\0' << "*." << strExt << '\0';		strExtFilter = ssExtFilter.str();

	// ... replace wrong extension
		//if (ofn.Flags & OFN_EXTENSIONDIFFERENT) {
		//	strExt.copy(&pathName[ofn.nFileExtension], strExt.length(), 0);
		//	// strcpy(&pathName[ofn.nFileExtension], strExt.c_str());
		//}

	// save
	if (dontLoadButSave) {
		if (!ww->getSaveFileName(filePath, strExtFilter, L"", L"Save state", strExt, false)) return;

		if (!targetState->save(filePath)) {
			ww->showMessageBox(L"Error", L"Could not save file.", MB_OK | MB_ICONSTOP | MB_APPLMODAL);
		}

	// load
	} else {
		if (!ww->getOpenFileName(filePath, strExtFilter, L"", L"Open state", strExt, false)) return;

		if (!targetState->load(filePath)) {
			ww->showMessageBox(L"Error", L"Could not load file.", MB_OK | MB_ICONSTOP | MB_APPLMODAL);
		}

		// update
		targetState->adaptButtonsOnSystem();
		targetState->setState(wildWeasel::guiElemState::VISIBLE);
		targetState->setButtonColor(colNormalRect);
		buttonOk	.setState(wildWeasel::guiElemState::DRAWED);
		buttonSave	.setState(wildWeasel::guiElemState::DRAWED);
		encolorUsedStones(nullptr);
	}
}		

//-----------------------------------------------------------------------------
// Name: buttonFuncResetSettingState()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::buttonFuncResetSettingState(void* pUser)
{
	// locals
	targetState->removeAllStones();
	targetState->copyFixedStones(initialState);
	targetState->adaptButtonsOnSystem();
	targetState->setState(wildWeasel::guiElemState::VISIBLE);
	targetState->setButtonColor(colNormalRect);
	buttonOk	.setState(wildWeasel::guiElemState::GRAYED);
	buttonSave	.setState(wildWeasel::guiElemState::GRAYED);
	encolorUsedStones(nullptr);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSetCommonState()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::buttonFuncSetCommonState(void* pUser)
{
	// set default file name
	if (curMode == modes::initial) {
		targetState->loadState(strDatabaseDir, wstring(L"\\initialState"));
	} else if (curMode == modes::final) {
		targetState->loadState(strDatabaseDir, wstring(L"\\finalState"));
	}

	// update
	buttonOk	.setState(wildWeasel::guiElemState::DRAWED);
	buttonSave	.setState(wildWeasel::guiElemState::DRAWED);
	encolorUsedStones(nullptr);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncFinishSettingState()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::buttonFuncFinishSettingState(void* pUser)
{
	// copy state
	if (curMode == modes::initial) {
		initialState->copyFieldState (targetState);
		finalState  ->copyFixedStones(targetState);
	} else if (curMode == modes::final) {
		finalState->copyFieldState(targetState);
	}
	
	ww->scenarioManager.setActiveScenario(scnSharedObjects);
}

//-----------------------------------------------------------------------------
// Name: setRandomState()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::setRandomState(bool only1x1Stones)
{
	// locals
	unsigned int position, stoneWidth, stoneHeight, stoneId, direction;
	unsigned int fieldHeight = targetState->getFieldHeight();
	unsigned int fieldWidth  = targetState->getFieldWidth();
	unsigned int tries=0, numTries=1000;
	
	// set random state
	targetState->removeAllStones();
	targetState->copyFixedStones(initialState);

	if (curMode == modes::initial) {
		if (only1x1Stones) {
			// set a 1x1 on all other positions
			for (position=0; position<fieldWidth * fieldHeight; position++) {
				if (fieldHeight * fieldWidth - 1 == position) continue;
				targetState->addStone(position, 1, 1, false);
			}

			// do some random moves
			for (tries=0; tries<numTries; tries++) {
				do {
					direction = rand() %  4;
					stoneId	  = rand() % (fieldHeight * fieldWidth - 2);
				} while (!targetState->isMoveAllowed(stoneId, direction));
				targetState->traga2Class::moveStone(stoneId, direction, false);
			}		
		} else {
			while (tries++<numTries) {
				// put a stone of random size on a random position
				position	=  rand() % (fieldWidth * fieldHeight);
				stoneWidth	= (rand() % MAX_STONE_EDGE_LENGTH) + 1;
				stoneHeight = (rand() % MAX_STONE_EDGE_LENGTH) + 1;
				targetState->addStone(position, stoneWidth, stoneHeight, false);

				// quit if only one or two free squares are left
				if (targetState->getNumFreeSquares() <= 2) break;
			};
		}
		finalState->removeAllStones();
		finalState->copyFixedStones(initialState);
		finalState->adaptButtonsOnSystem();
		finalState->setState(wildWeasel::guiElemState::HIDDEN);
		finalState->setButtonColor(colNormalRect);

	} else if (curMode == modes::final) {
		if (only1x1Stones) {
			for (position=0; position<fieldHeight * fieldWidth - 1; position++) {
				stoneId = position;
				targetState->addStone(initialState, stoneId, position);
			}
		} else {
			if (initialState->getNumFreeSquares() < fieldHeight * fieldWidth) {
				do {	
					do {
						stoneId		= rand() % initialState->getNumStones();
						position	= initialState->getStonePos(stoneId);
					} while (ST_FREESQUARE == initialState->getStoneType(position));
					position	= rand() % (fieldWidth * fieldHeight);
				} while (!targetState->addStone(initialState, stoneId, position));
			}
		}
	}
	
	// update
	targetState->adaptButtonsOnSystem();
	targetState->setButtonColor(colNormalRect);
	targetState->setState(wildWeasel::guiElemState::VISIBLE);
	buttonOk	.setState(wildWeasel::guiElemState::DRAWED);
	buttonSave	.setState(wildWeasel::guiElemState::DRAWED);
	encolorUsedStones(nullptr);
}

//-----------------------------------------------------------------------------
// Name: encolorUsedStones()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::encolorUsedStones(traga2_3D_stoneCluster* theCluster)
{
	// locals
	unsigned int		position, stoneType, stoneId;
	wildWeasel::color	color;

	// only if setting final state
	if (scnSetState.curMode != scenarioSetState::modes::final) return;

	// color already used stones
	for (position=0; position<initialState->getFieldHeight()*initialState->getFieldWidth(); position++) {
		
		stoneId		= initialState->getStoneId(position);
		stoneType	= targetState ->getStoneType(targetState->getStonePos(stoneId));

		// skip other stones
		if (theCluster != NULL && initialState->stoneIdToButtonMap[stoneId]->stoneCluster != theCluster) continue;

		// skip selected stone
		if (selStone != NULL && theCluster != selStone->stoneCluster && selStone == initialState->stoneIdToButtonMap[stoneId] && selState != NULL) continue;

		// calc color
		if (stoneType >= ST_NUM_DEF && stoneType != initialState->getFieldHeight()*initialState->getFieldWidth()) {
			color = colGray;
		} else {
			color = colNormalRect;
		}

		// set color
		if (initialState->getStoneType(position) >= ST_NUM_DEF) {
			initialState->stoneIdToButtonMap[stoneId]->setColor(color);
		}
	}
}

//-----------------------------------------------------------------------------
// Name: checkIfSettingStateReady()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::checkIfSettingStateReady()
{
	// target state ready?
	if ((scnSetState.curMode == scenarioSetState::modes::initial && targetState->getNumStones() > 0)
	||  (scnSetState.curMode == scenarioSetState::modes::final   && targetState->getNumFreeSquares() < targetState->getFieldHeight()*targetState->getFieldWidth())) {
		buttonOk	.setState(wildWeasel::guiElemState::DRAWED);
		buttonSave	.setState(wildWeasel::guiElemState::DRAWED);
	} else {
		buttonOk	.setState(wildWeasel::guiElemState::GRAYED);
		buttonSave	.setState(wildWeasel::guiElemState::GRAYED);
	}		
}

//-----------------------------------------------------------------------------
// Name: leftClickOnDepotOrInitialState()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::leftClickOnDepotOrInitialState(wildWeasel::guiElemEvFol* item, void* pUser)
{
	// unmark selected one
	if (selState != nullptr && selStone != nullptr) selStone->setColor(colNormalRect);

	// mark selected one in blue if already used in target state
	if (scnSetState.curMode == scenarioSetState::modes::final && selState != nullptr && selState->field == initialState && selStone != nullptr) scnSetState.encolorUsedStones(selStone->stoneCluster);

	// unselect, when already selected
	if (cursorOverState == selState && cursorOverStone == selStone) {
		selState	= nullptr;
		selStone	= nullptr;
	// mark hovered one
	} else {
		selState	= cursorOverState;
		selStone	= cursorOverStone;
		selStone->setColor(colSelRect);
	}

	checkIfSettingStateReady();
}

//-----------------------------------------------------------------------------
// Name: leftClickOnTargetGrid()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::leftClickOnTargetGrid(wildWeasel::guiElemEvFol* item, void* pUser)
{
	// since the buttonId of targetGrid is equal the position			
	uintStonePos myStonePos = cursorOverStone->stonePos; 

	// stone has been added, so unmark it
	if (stoneAddedAtTargetGridPos == cursorOverStone) {
		cursorOverStone->setColor(colNormalRect);

		// when setting final state also unmark the source stone
		if (scnSetState.curMode == scenarioSetState::modes::final) {
			selStone->setColor(colGray);
			selState		= nullptr;
			selStone		= nullptr;
			scnSetState.encolorUsedStones(nullptr);
		// when setting initial state, so reset final state
		} else if (scnSetState.curMode == scenarioSetState::modes::initial) {
			finalState->removeAllStones();
			finalState->adaptButtonsOnSystem();				
		}
		checkIfSettingStateReady();

	// when no stone could be added, since there is already one, than remove it
	} else if (targetState->getStoneId(myStonePos) < targetState->getFieldHeight()*targetState->getFieldWidth()
			&&  targetState->getStoneType(myStonePos) != ST_FREESQUARE) {
		if (scnSetState.curMode == scenarioSetState::modes::initial) {
			targetState->removeStone(myStonePos);
			finalState->removeAllStones();
			finalState->adaptButtonsOnSystem();
		} else if (scnSetState.curMode == scenarioSetState::modes::final && targetState->getStoneType(myStonePos) != ST_FIXEDSTONE) {
			targetState->removeStone(initialState, myStonePos);
			scnSetState.encolorUsedStones(nullptr);
		}
		targetState->adaptButtonsOnSystem();
		targetState->setState(wildWeasel::guiElemState::VISIBLE);
		targetState->setButtonColor(colNormalRect);
		checkIfSettingStateReady();
	}
	stoneAddedAtTargetGridPos	= nullptr;

}

//-----------------------------------------------------------------------------
// Name: rightClickOnTargetGrid()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::rightClickOnTargetGrid(wildWeasel::guiElemEvFol* item, void* pUser)
{
}

//-----------------------------------------------------------------------------
// Name: hoverOnTargetGrid()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::hoverTargetGrid(wildWeasel::guiElemEvFol* item, void* pUser)
{
	// locals
	traga2_3D_stone*		myStone		= (traga2_3D_stone*) item;
	traga2_3D_stoneCluster* myCluster	= myStone->stoneCluster;
	uintStonePos			myStonePos	= myStone->stonePos;
	
	// add stone to targetState
	cursorOverState		= myCluster;
	cursorOverStone		= myStone;

	// if there is already a stone, than mark it, so that it can be deleted on click
	if (targetState->getStoneId  (myStonePos) < targetState->getFieldHeight()*targetState->getFieldWidth()
	&&  targetState->getStoneType(myStonePos) != ST_FREESQUARE
	&&  (targetState->getStoneType(myStonePos) != ST_FIXEDSTONE || scnSetState.curMode == scenarioSetState::modes::initial)) {
		stoneAddedAtTargetGridPos = nullptr;
		targetState->setColor(myStonePos, colSelRect);
							
	// setting initial state: square is free, so add stone temporarly
	} else if (scnSetState.curMode == scenarioSetState::modes::initial 
	&&		   selState  == depotParts 
	&&		   selStone != nullptr
	&&	       targetState->addStone(myStonePos, depotPartType[selStone->stoneId].width, depotPartType[selStone->stoneId].height, false)) {
		stoneAddedAtTargetGridPos = cursorOverStone;
		targetState->adaptButtonsOnSystem();
		targetState->setState(wildWeasel::guiElemState::VISIBLE);
		targetState->setButtonColor(colNormalRect);
		targetState->setColor(myStonePos, colCurRect);
					
	// setting final state: square is free, so add stone temporarly
	} else if (scnSetState.curMode == scenarioSetState::modes::final 
	&&		   selState	!= nullptr	&&	selState->field		== initialState 
	&&		   selStone != nullptr &&	targetState->addStone(initialState, selStone->stoneId, myStonePos)) {
		stoneAddedAtTargetGridPos = cursorOverStone;
		targetState->adaptButtonsOnSystem();
		targetState->setState(wildWeasel::guiElemState::VISIBLE);
		targetState->setButtonColor(colNormalRect);
		targetState->setColor(myStonePos, colCurRect);
					
	// stone can't be added
	} else {
		stoneAddedAtTargetGridPos = nullptr;
	}
}

//-----------------------------------------------------------------------------
// Name: leftTargetGrid()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::leftTargetGrid(wildWeasel::guiElemEvFol* item, void* pUser)
{
	// since the stoneId of targetGrid is equal the position
	uintStonePos myStonePos = cursorOverStone->stonePos; 

	// remove stone from targetState?
	if (stoneAddedAtTargetGridPos == cursorOverStone) {
				
		stoneAddedAtTargetGridPos	= nullptr;

		if (scnSetState.curMode == scenarioSetState::modes::initial) {
			targetState->removeStone(myStonePos);
		} else if (scnSetState.curMode == scenarioSetState::modes::final) { 
			targetState->removeStone(initialState, myStonePos);
		}
		targetState->adaptButtonsOnSystem();
		targetState->setState(wildWeasel::guiElemState::VISIBLE);
		targetState->setButtonColor(colNormalRect);

	// unmark stone from targetState?
	} else if (targetState->getStoneId  (myStonePos) < targetState->getFieldHeight()*targetState->getFieldWidth() &&  targetState->getStoneType(myStonePos) != ST_FREESQUARE) {
		targetState->setColor(myStonePos, colNormalRect);
	}

	// unmark
	cursorOverState	= nullptr;
	cursorOverStone	= nullptr;
}

//-----------------------------------------------------------------------------
// Name: hoverDepotOrInitialState()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::hoverDepotOrInitialState(wildWeasel::guiElemEvFol* item, void* pUser)
{
	// since the stoneId of targetGrid is equal the position
	traga2_3D_stone*		myStone		= (traga2_3D_stone*) item;
	traga2_3D_stoneCluster* myCluster	= myStone->stoneCluster;

	// don't mark fixed stones
	if (!(scnSetState.curMode == scenarioSetState::modes::final && myCluster->field == initialState && initialState->getStoneType(initialState->getStonePos(myStone->stoneId)) == ST_FIXEDSTONE)) {

		// ???
		if (cursorOverState != nullptr && cursorOverStone != nullptr && cursorOverStone != selStone) { 
							
			// unmark selected one
			cursorOverStone->setColor(colNormalRect);
		
			// mark selected one in blue if already used in target state
			if (scnSetState.curMode == scenarioSetState::modes::final && cursorOverState->field == initialState && cursorOverStone != nullptr) scnSetState.encolorUsedStones(cursorOverStone->stoneCluster);
		}

		// mark new one, if not a fixed stone
		cursorOverState	= myCluster;
		cursorOverStone	= myStone;
		cursorOverStone->setColor(colCurRect);
	}
}

//-----------------------------------------------------------------------------
// Name: leftDepotOrInitialState()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSetState::leftDepotOrInitialState(wildWeasel::guiElemEvFol* item, void* pUser)
{
	// unmark current one from depotParts or initial state
	if (cursorOverStone != selStone) {
		cursorOverStone->setColor(colNormalRect);

		// mark selected one in blue if already used in target state
		if (scnSetState.curMode == scenarioSetState::modes::final && cursorOverState->field == initialState) scnSetState.encolorUsedStones(cursorOverStone->stoneCluster);
	}

	// unmark
	cursorOverState	= nullptr;
	cursorOverStone	= nullptr;
}
#pragma endregion

/************************************************************************************************************************************\
  playing mode
\************************************************************************************************************************************/

#pragma region playing mode
//-----------------------------------------------------------------------------
// Name: scenarioPlayingGame::activate()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioPlayingGame::activate()
{
	// set initial state
	scnSharedObjects.activate();
	curState->copyFieldState(initialState);
	curState->adaptButtonsOnSystem();
	curState->setState(wildWeasel::guiElemState::DRAWED);
	curState->setButtonColor(colNormalRect);
	textInfoLevel1.setTextState(wildWeasel::guiElemState::DRAWED);
	textInfoLevel1.setText(wstring(L"Move cursor over stone and press arrow keys to move it."));

	// proccess key presses
	eventFollower::followEvent(this, wildWeasel::eventFollower::eventType::KEYDOWN);

	for (auto& curStone : curState->getContainer<traga2_3D_stone>()) {
		curStone->assignOnMouseEnteredRegion(bind(&scenarioPlayingGame::cursorHoverStone, &scnPlayGame, placeholders::_1, placeholders::_2), this);
		curStone->assignOnMouseLeftRegion   (bind(&scenarioPlayingGame::cursorleftStone,  &scnPlayGame, placeholders::_1, placeholders::_2), this);
	}
}

//-----------------------------------------------------------------------------
// Name: scenarioPlayingGame::deactivate()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioPlayingGame::deactivate()
{
	textInfoLevel1.setTextState(wildWeasel::guiElemState::HIDDEN);
	curState->setButtonColor(colNormalRect);

	eventFollower::forgetEvent(this, wildWeasel::eventFollower::eventType::KEYDOWN);

	for (auto& curStone : curState->getContainer<traga2_3D_stone>()) {
		curStone->assignOnMouseEnteredRegion(nullptr, nullptr);
		curStone->assignOnMouseLeftRegion   (nullptr, nullptr);
	}
}

//-----------------------------------------------------------------------------
// Name: keyDown()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioPlayingGame::keyDown(int keyCode)
{
	// translate keycode
	if (cursorOverState) {

		switch (keyCode)
		{
		case VK_HOME: case VK_DELETE:
			break;
		case VK_LEFT:
			cursorOverStone->field->moveStone(cursorOverStone->stoneId, MV_LEFT, 10, 100, finalState);
			break;
		case VK_RIGHT:
			cursorOverStone->field->moveStone(cursorOverStone->stoneId, MV_RIGHT, 10, 100, finalState);
			break;
		case VK_UP:
			cursorOverStone->field->moveStone(cursorOverStone->stoneId, MV_UP, 10, 100, finalState);
			break;
		case VK_DOWN:
			cursorOverStone->field->moveStone(cursorOverStone->stoneId, MV_DOWN, 10, 100, finalState);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Name: cursorHoverStone()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioPlayingGame::cursorHoverStone(wildWeasel::guiElemEvFol* item, void* pUser)
{
	// unmark selected one
	if (cursorOverState != nullptr && cursorOverStone != nullptr) cursorOverStone->setColor(colNormalRect);

	auto myStone = (traga2_3D_stone*) item; // ... item->getPointer<traga2_3D_stone>();

	// mark new one
	cursorOverState			= myStone->stoneCluster;
	cursorOverStone			= myStone;
	cursorOverStone->setColor(colCurRect);
}

//-----------------------------------------------------------------------------
// Name: cursorleftStone()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioPlayingGame::cursorleftStone(wildWeasel::guiElemEvFol* item, void* pUser)
{
	// unmark selected one
	if (cursorOverState != nullptr && cursorOverStone != nullptr) cursorOverStone->setColor(colNormalRect);

	// unmark
	cursorOverState		= nullptr;
	cursorOverStone		= nullptr;
}

#pragma endregion

/************************************************************************************************************************************\
  finish animation mode
\************************************************************************************************************************************/

#pragma region finish animation
//-----------------------------------------------------------------------------
// Name: scenarioFinishAnimation::init()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioFinishAnimation::init()
{
	eventFollower::followEvent(this, wildWeasel::eventFollower::eventType::LEFT_MOUSEBUTTON_PRESSED);
}

//-----------------------------------------------------------------------------
// Name: scenarioFinishAnimation::activate()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioFinishAnimation::activate()
{
	curState    ->setState(wildWeasel::guiElemState::VISIBLE);
	initialState->setState(wildWeasel::guiElemState::HIDDEN);
	finalState  ->setState(wildWeasel::guiElemState::HIDDEN);
	initialState->woodBox->setState(wildWeasel::guiElemState::HIDDEN);
	finalState  ->woodBox->setState(wildWeasel::guiElemState::HIDDEN);

	curState	->explode();
}

//-----------------------------------------------------------------------------
// Name: scenarioFinishAnimation::deactivate()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioFinishAnimation::deactivate()
{
	curState->stopAnimation();
	
	for (auto itr : curState->items)  {
		traga2_3D_stone* curButton = (traga2_3D_stone*) itr;
		curButton->setTextState(wildWeasel::guiElemState::HIDDEN);
		curState->calcButtonMatrix(curButton);
		curButton->setDirty();
	}
}

//-----------------------------------------------------------------------------
// Name: leftMouseButtonPressed()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioFinishAnimation::leftMouseButtonPressed(int xPos, int yPos)
{
	if (guiScenario::isActive()) {
		ww->scenarioManager.setActiveScenario(scnSharedObjects);
	}
}

#pragma endregion

/************************************************************************************************************************************\
  solving mode
\************************************************************************************************************************************/

#pragma region solving mode
//-----------------------------------------------------------------------------
// Name: scenarioSolve::activate()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::activate()
{
	showSolveControls(true);
	solveCurCtate();
}

//-----------------------------------------------------------------------------
// Name: scenarioSolve::deactivate()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::deactivate()
{
	showSolveControls(false);
}

//-----------------------------------------------------------------------------
// Name: scenarioSolve::init()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::init()
{
	// create events
	hEventLockSolutionVars.create(ww, false, false);
	hEventLockSolutionVars.setFunc(bind(&scenarioSolve::updateShowedSolution,	&scnSolve, placeholders::_1), this, true);
	scnSolve.hEventSolutionVarsLocked 		= CreateEvent(NULL, false, false, NULL);
	scnSolve.hEventUnlockSolutionVars 		= CreateEvent(NULL, false, false, NULL);
	scnSolve.hEventTerminateThreadSolve		= CreateEvent(NULL, true,  false, NULL);
	scnSolve.hEventThreadSolveTerminated	= CreateEvent(NULL, true,  true,  NULL);
}

//-----------------------------------------------------------------------------
// Name: showSolveControls()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::showSolveControls(bool visible)
{
	// locals
	wildWeasel::guiElemState	
		
	nCmdShow = visible ? wildWeasel::guiElemState::GRAYED : wildWeasel::guiElemState::HIDDEN;
	buttonSolvePlay			.setState(nCmdShow);
	buttonSolvePause		.setState(nCmdShow);
	buttonSolveForward		.setState(nCmdShow);
	buttonSolveBackward		.setState(nCmdShow);
	buttonSolveFaster		.setState(nCmdShow);
	buttonSolveSlower		.setState(nCmdShow);
	buttonSolveRestart		.setState(nCmdShow);

	nCmdShow = visible ? wildWeasel::guiElemState::DRAWED : wildWeasel::guiElemState::HIDDEN;
	textNumProcStates		.setTextState(nCmdShow);
	textStepInTotal			.setTextState(nCmdShow);
	textAnimationSpeed		.setTextState(nCmdShow);
	buttonSolveStopCalc		.setState(nCmdShow);

	// stop thread and timer
	if (visible == false) {
		timerPlay.terminate();
		terminateSolveThread();
	}
}

//-----------------------------------------------------------------------------
// Name: updateLabels()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::updateLabels()
{
	wstringstream wss;
	wss.str(L""); wss << "states processed: " << solVars.numProcessedStates << "/" << solVars.numPossibleStates;		textNumProcStates	.setText(wss.str());
	wss.str(L""); wss << "step in total: " << curShowedStep + 1 << "/" << solVars.totalNumSteps;						textStepInTotal		.setText(wss.str());
	wss.str(L""); wss << "steps per sec: " << setprecision(3) << 1024.0f / ((float) solutionAnimationSleepDuration);	textAnimationSpeed	.setText(wss.str());
}

//-----------------------------------------------------------------------------
// Name: solveCurState()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::solveCurCtate()
{
	// locals
	DWORD			threadId;
	void*			pParameter = (void*) &solVars;

	// check if initial und final cube are set correct
	if (!initialState->checkIntegrity()) {
		MessageBoxA(ww->getHwnd(), "Set initial state first!", "ERROR", MB_OK);
		return;
	}

	if (!finalState->checkIntegrity()) {
		MessageBoxA(ww->getHwnd(), "Set final state first!", "ERROR", MB_OK);
		return;
	}

	// check database directory
	if (!ww->isFolderPathValid(strDatabaseDir.c_str())) {
		MessageBoxA(ww->getHwnd(), "Selected database directory doesn't exist.\nPlease chose a vaid directory first.", "ERROR", MB_OK);
		return;
	}
	
	// set initial state
	curState->copyFieldState(initialState);
	curState->adaptButtonsOnSystem();
	curState->setState(wildWeasel::guiElemState::VISIBLE);
	curState->setButtonColor(colNormalRect);
	curState->setShowProgressFunc(bind(&scenarioSolve::showProgress, this, placeholders::_1, placeholders::_2, placeholders::_3));

	// zero solution variables
	solVars.totalNumSteps			= 0;
	solVars.numPossibleStates		= 0;
	solVars.numProcessedStates		= 0;
	solVars.solPathDirection		.clear();
	solVars.solPathStonePos			.clear();
	solVars.parent					= this;
	
	solutionAnimationSleepDuration	= 512;
	curShowedStep					= 0;
	autoForward						= false;

	updateLabels();

	// show controls
	buttonSolveStopCalc.setImageFiles(buttonSolveStopCalcImages);
	buttonSolveStopCalc.setText(wstring(L"Stop Calculation"));
	showSolveControls(true);
	timerPlay.terminate();
	autoForward = false;

	// create thread
	ResetEvent(hEventThreadSolveTerminated);
	hThreadSolve = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ThreadProcSolveStatic, (void*) pParameter, 0, &threadId);
	SetThreadPriority(hThreadSolve, THREAD_PRIORITY_BELOW_NORMAL);
}

//-----------------------------------------------------------------------------
// Name: ThreadProcSolveStatic()
// Desc: 
//-----------------------------------------------------------------------------
DWORD WINAPI scenarioSolve::ThreadProcSolveStatic(LPVOID lpParameter)
{
	return ((solutionVariablesStruct*) lpParameter)->parent->ThreadProcSolve();
}

//-----------------------------------------------------------------------------
// Name: ThreadProcSolve()
// Desc: 
//-----------------------------------------------------------------------------
DWORD WINAPI scenarioSolve::ThreadProcSolve()
{
	// calc possible moves from zones->zoneProp[selZone].excludedMove[]
	solVars.solPathDirection	.clear();
	solVars.solPathStonePos		.clear();
	solVars.numPossibleStates	= 0;
	solVars.numProcessedStates	= 0;
	solVars.totalNumSteps		= 0;

	// shall process be terminated?
	if (WaitForSingleObject(hEventTerminateThreadSolve, 0) == WAIT_OBJECT_0) {
		return 0;
	}

	// solve
	if (!curState->solveProblem(finalState, solVars.totalNumSteps, solVars.solPathStonePos, solVars.solPathDirection)) {
		if (WaitForSingleObject(hEventTerminateThreadSolve, 0) != WAIT_OBJECT_0) {
			MessageBoxA(ww->getHwnd(), "No solution found!", "ERROR", MB_OK);
		}
	// a solution was found
	} else {
		buttonSolvePlay			.setState(wildWeasel::guiElemState::DRAWED);
		buttonSolveRestart		.setState(wildWeasel::guiElemState::DRAWED);
		buttonSolveFaster		.setState(wildWeasel::guiElemState::DRAWED);
		buttonSolveSlower		.setState(wildWeasel::guiElemState::DRAWED);
		buttonSolveForward		.setState(wildWeasel::guiElemState::DRAWED);
	}

	// update display
	updateLabels();

	// if termination of thread is not requested
	if (WaitForSingleObject(hEventTerminateThreadSolve, 0) != WAIT_OBJECT_0) {
		buttonSolveStopCalc.setText(wstring(L"Quit"));
		buttonSolveStopCalc.setImageFiles(buttonSolveQuitImages);
	}
	SetEvent(hEventThreadSolveTerminated);
	hThreadSolve = NULL;
	ResetEvent(hEventTerminateThreadSolve);
	
	// terminate thread
	return 0;
}

//-----------------------------------------------------------------------------
// Name: showProgress()
// Desc: 
//-----------------------------------------------------------------------------
bool scenarioSolve::showProgress(int numPossibleStates, int numProcessedStates, wchar_t* strStatus)
{
	// leave if thread shall be terminated
	if (WaitForSingleObject(hEventTerminateThreadSolve, 0) == WAIT_OBJECT_0) return false;

	// lock solution variables
	hEventLockSolutionVars.set();
	WaitForSingleObject(hEventSolutionVarsLocked, INFINITE);

	// update solVars
	solVars.numPossibleStates  = (unsigned int) numPossibleStates;
	solVars.numProcessedStates = (unsigned int) numProcessedStates;
	solVars.strStatus.assign(strStatus);

	// unlock solution variables
	SetEvent(hEventUnlockSolutionVars);

	// continue search
	return true;
}

//-----------------------------------------------------------------------------
// Name: updateShowedSolution()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::updateShowedSolution(void* pUser)
{
	// wait until function addSolAndApplyOnCube() set the event hEventUnlockSolutionVars
	SetEvent(hEventSolutionVarsLocked);
	if (WaitForSingleObject(hEventUnlockSolutionVars, dwWaitTimeForUnlockSolVars) == WAIT_OBJECT_0) {

		// update edit fields
		updateLabels();
	}
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolveRestart()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::buttonFuncSolveRestart(void* pUser)
{
	buttonSolvePlay			.setState(wildWeasel::guiElemState::DRAWED);
	buttonSolvePause		.setState(wildWeasel::guiElemState::GRAYED);
	buttonSolveForward		.setState(wildWeasel::guiElemState::DRAWED);
	buttonSolveBackward		.setState(wildWeasel::guiElemState::GRAYED);

	timerPlay.terminate();
	autoForward = false;
	curShowedStep = 0;

	// update edit fields
	updateLabels();

	// set initial state
	curState->copyFieldState(initialState);
	curState->adaptButtonsOnSystem();
	curState->setState(wildWeasel::guiElemState::VISIBLE);
	curState->setButtonColor(colNormalRect);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolvePlay()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::buttonFuncSolvePlay(void* pUser)
{
	buttonSolvePlay			.setState(wildWeasel::guiElemState::GRAYED);
	buttonSolvePause		.setState(wildWeasel::guiElemState::DRAWED);
	timerPlay.start(ww, TimerProcPlay, this, solutionAnimationSleepDuration);
	autoForward = true;
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolvePause()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::buttonFuncSolvePause(void* pUser)
{
	buttonSolvePlay			.setState((hThreadSolve!=NULL)?wildWeasel::guiElemState::GRAYED:wildWeasel::guiElemState::DRAWED);
	buttonSolvePause		.setState(wildWeasel::guiElemState::GRAYED);
	timerPlay.terminate();
	autoForward = false;
}

//-----------------------------------------------------------------------------
// Name: vbuttonFuncSolveForward()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::buttonFuncSolveForward(void* pUser)
{
	// is there a further step ?
	if (solVars.totalNumSteps == (uintStepId) curShowedStep) return;	

	// update edit fields
	updateLabels();

	// move stone
	unsigned int stoneId	= curState->getStoneId(solVars.solPathStonePos[curShowedStep]);
	unsigned int direction	= solVars.solPathDirection[curShowedStep];
	if (!curState->moveStone(stoneId, direction, 10, 100, nullptr)) return;
	curShowedStep++;

	// last step?
	if (curShowedStep == solVars.totalNumSteps) {
		buttonSolvePlay			.setState(wildWeasel::guiElemState::GRAYED);
		buttonSolvePause		.setState(wildWeasel::guiElemState::GRAYED);
		buttonSolveForward		.setState(wildWeasel::guiElemState::GRAYED);
		timerPlay.terminate();
		autoForward = false;
	} 
	buttonSolveBackward		.setState(wildWeasel::guiElemState::DRAWED);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolveBackward()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::buttonFuncSolveBackward(void* pUser)
{
	// is there a further step ?
	if (0 == curShowedStep) return;	

	// update edit fields
	curShowedStep--;
	updateLabels();

	// calc reverse move
	unsigned int stonePosAfterMove	= curState->getStonePosAfterMove(solVars.solPathDirection[curShowedStep], solVars.solPathStonePos[curShowedStep]);
	unsigned int stoneId			= curState->getStoneId(stonePosAfterMove);
	unsigned int direction			= curState->getInverseDirection(solVars.solPathDirection[curShowedStep]);
	if (!curState->moveStone(stoneId, direction, 10, 100, nullptr)) {
		curShowedStep++;
		return;
	}
	
	// last step?
	if (curShowedStep == (uintStepId) 0) {
		buttonSolveBackward		.setState(wildWeasel::guiElemState::GRAYED);
	}
	buttonSolvePlay			.setState(wildWeasel::guiElemState::DRAWED);
	buttonSolvePause		.setState(wildWeasel::guiElemState::GRAYED);
	buttonSolveForward		.setState(wildWeasel::guiElemState::DRAWED);	
	timerPlay.terminate();
	autoForward = false;
}

//-----------------------------------------------------------------------------
// Name: terminateSolveThread()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::terminateSolveThread()
{
	SetEvent(hEventTerminateThreadSolve);
	SetEvent(hEventSolutionVarsLocked);
	if (WaitForSingleObject(hEventThreadSolveTerminated, dwWaitTimeForThreadTermination) != WAIT_OBJECT_0) {
		ResetEvent(hEventTerminateThreadSolve);
		SetEvent(hEventThreadSolveTerminated);
		TerminateThread(hThreadSolve, 0);
		hThreadSolve = NULL;
	}
	buttonSolveStopCalc.setText(wstring(L"Quit"));
	buttonSolveStopCalc.setImageFiles(buttonSolveQuitImages);
	ResetEvent(hEventSolutionVarsLocked);
	ResetEvent(hEventTerminateThreadSolve);
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolveStopCalc()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::buttonFuncSolveStopCalc(void* pUser)
{
	// calcution is not running any more, so just quit
	if (hThreadSolve == NULL) {
		timerPlay.terminate();
		showSolveControls(false);
	// calculation is still running
	} else {	
		terminateSolveThread();
	}
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolveFaster()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::buttonFuncSolveFaster(void* pUser)
{
	if (solutionAnimationSleepDuration <= 256) return;
	solutionAnimationSleepDuration /= 2;
	if (autoForward) {
		timerPlay.terminate();
		timerPlay.start(ww, TimerProcPlay, this, solutionAnimationSleepDuration);
	}
	updateLabels();
}

//-----------------------------------------------------------------------------
// Name: buttonFuncSolveSlower()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::buttonFuncSolveSlower(void* pUser)
{
	if (solutionAnimationSleepDuration >= 16384) return;
	solutionAnimationSleepDuration *= 2;
	if (autoForward) {
		timerPlay.terminate();
		timerPlay.start(ww, TimerProcPlay, this, solutionAnimationSleepDuration);
	}
	updateLabels();
}

//-----------------------------------------------------------------------------
// Name: scenarioSolve::TimerProcPlay()
// Desc: 
//-----------------------------------------------------------------------------
void scenarioSolve::TimerProcPlay(void* pUser)
{
	scnSolve.buttonFuncSolveForward(&scnSolve);
}

#pragma endregion

/* To Do's **********************************************************
 - Lösungsweg als Screenshot-Reihe exportieren
 - calcConnections()-Algorithmus optimieren
*******************************************************************/

