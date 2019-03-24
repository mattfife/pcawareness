//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------
#include "SampleStartDX11.h"
#include "CPUTRenderTarget.h"

 #include <TCHAR.H> // for osstring

const UINT SHADOW_WIDTH_HEIGHT = 2048;

// set file to open
cString g_OpenFilePath;
cString g_OpenShaderPath;
cString g_OpenFileName;
std::string g_OpenSceneFileName;

extern float3 gLightDir;

// Application entry point.  Execution begins here.
//-----------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Prevent unused parameter compiler warnings
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

#ifdef DEBUG
    // tell VS to report leaks at any exit of the program
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    CPUTResult result=CPUT_SUCCESS;
    int returnCode=0;

    // create an instance of my sample
    MySample *pSample = new MySample();
    
    // We make the assumption we are running from the executable's dir in
    // the CPUT SampleStart directory or it won't be able to use the relative paths to find the default
    // resources    
    cString ResourceDirectory;
    CPUTFileSystem::GetExecutableDirectory(&ResourceDirectory);
    ResourceDirectory.append(_L("../../../Media/gui/"));
    
    // Initialize the system and give it the base CPUT resource directory (location of GUI images/etc)
    // For now, we assume it's a relative directory from the executable directory.  Might make that resource
    // directory location an env variable/hardcoded later
    pSample->CPUTInitialize(ResourceDirectory);

    CPUTFileSystem::GetExecutableDirectory(&ResourceDirectory);
    ResourceDirectory.append(_L("../../../Media/System/"));
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();
    pAssetLibrary->SetSystemDirectoryName(ResourceDirectory);


    // window and device parameters
    CPUTWindowCreationParams params;
    params.deviceParams.refreshRate         = 60;
    params.deviceParams.swapChainBufferCount= 1;
    params.deviceParams.swapChainFormat     = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    params.deviceParams.swapChainUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;

    // parse out the parameter settings or reset them to defaults if not specified
    std::string AssetFilename;
    g_OpenSceneFileName = "../../../Media/hallwayscene.scene";
    char *pCommandLine = ws2s(lpCmdLine);
    std::string CommandLine(pCommandLine);
    pSample->CPUTParseCommandLine(CommandLine, &params, &AssetFilename, &g_OpenSceneFileName);
    delete pCommandLine;

    // parse out the filename of the .set file to open (if one was given)
    if(AssetFilename.size())
    {
        // strip off the target .set file, and parse it's full path
        cString PathAndFilename, Drive, Dir, Ext;  
        g_OpenFilePath.clear();
        std::wstring wsAssetFileName = s2ws(AssetFilename.c_str());
        
        // resolve full path, and check to see if the file actually exists
        CPUTFileSystem::ResolveAbsolutePathAndFilename(wsAssetFileName, &PathAndFilename);
        result = CPUTFileSystem::DoesFileExist(PathAndFilename);
        if(CPUTFAILED(result))
        {
            cString ErrorMessage = _L("File specified in the command line does not exist: \n")+PathAndFilename;
            #ifndef DEBUG
            DEBUGMESSAGEBOX(_L("Error loading command line file"), ErrorMessage);
            #else
            ASSERT(false, ErrorMessage);
            #endif
        }
        
        // now parse through the path and removing the trailing \\Asset\\ directory specification
        CPUTFileSystem::SplitPathAndFilename(PathAndFilename, &Drive, &Dir, &g_OpenFileName, &Ext);
        // strip off any trailing \\ 
        size_t index=Dir.size()-1;
        if(Dir[index]=='\\' || Dir[index]=='/')
        {
            index--;
        }

        // strip off \\Asset
        for(size_t ii=index; ii>=0; ii--)
        {
            if(Dir[ii]=='\\' || Dir[ii]=='/')
            {
                Dir = Dir.substr(0, ii+1); 
                g_OpenFilePath = Drive+Dir; 
                index=ii;
                break;
            }
        }        
        
        // strip off \\<setname> 
        for(size_t ii=index; ii>=0; ii--)
        {
            if(Dir[ii]=='\\' || Dir[ii]=='/')
            {
                Dir = Dir.substr(0, ii+1); 
                g_OpenShaderPath = Drive + Dir + _L("\\Shader\\");
                break;
            }
        }
    }

    // create the window and device context
    result = pSample->CPUTCreateWindowAndContext(_L("CPUTWindow DirectX 11"), params);
    ASSERT( CPUTSUCCESS(result), _L("CPUT Error creating window and context.") );
    
    // start the main message loop
    returnCode = pSample->CPUTMessageLoop();

    pSample->DeviceShutdown();

    // cleanup resources
    SAFE_DELETE(pSample);

    return returnCode;
}

//-----------------------------------------------------------------------------
void MySample::Create()
{    
    // Call ResizeWindow() because it creates some resources that our blur material needs (e.g., the back buffer)
    int width, height;
    mpWindow->GetClientDimensions(&width, &height);
    ResizeWindow(width, height);

    CPUTRenderStateBlockDX11 *pBlock = new CPUTRenderStateBlockDX11();
    CPUTRenderStateDX11 *pStates = pBlock->GetState();

    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    gLightDir.normalize();

    // TODO: read from cmd line, using these as defaults
    pAssetLibrary->SetMediaDirectoryName( _L("../../../Media/") );

    CPUTGuiControllerDX11 *pGUI = CPUTGetGuiController();


	// Create the Perceptual computing assistant module
	mpAssistModule = new PerCAssist();
	ASSERT((NULL!=mpAssistModule), _L("Unable to create PerC Assistant module"));


    // Create some controls
    CPUTButton     *pButton = NULL;
    CPUTCheckbox   *pCheckbox = NULL;
    CPUTDropdown   *pDropdown = NULL;
    CPUTCheckbox   *pSlider = NULL;
    CPUTText       *pText = NULL;

    pGUI->CreateButton(_L("Fullscreen"), ID_FULLSCREEN_BUTTON, ID_MAIN_PANEL, &pButton);
	pGUI->CreateText( _L("\t"), ID_IGNORE_CONTROL_ID, ID_MAIN_PANEL, &pText);

	pGUI->CreateText( _L("Number of persons detected: "), ID_DETECTED_PERSONS_STRING, ID_MAIN_PANEL, &mpDetectCount);
	pGUI->CreateText( _L(" "), ID_IGNORE_CONTROL_ID, ID_MAIN_PANEL, &mpPresentText);
	

	pGUI->CreateCheckbox(_L("Display camera window"), ID_CAMERA_WINDOW, ID_MAIN_PANEL, &pCheckbox);
	pCheckbox->SetCheckboxState(CPUT_CHECKBOX_CHECKED);
	pGUI->CreateText( _L(" "), ID_IGNORE_CONTROL_ID, ID_MAIN_PANEL, &pText);

	// user absence+action(s)
	pGUI->CreateText( _L("Action if user leaves:"), ID_IGNORE_CONTROL_ID, ID_MAIN_PANEL);
	pGUI->CreateDropdown(_L("Nothing"), ID_AWAY_ACTION, ID_MAIN_PANEL, &mpAwayAction);	
	mpAwayAction->AddSelectionItem(_L("Pause"));
	mpAwayAction->AddSelectionItem(_L("Dim display"));
	mpAwayAction->AddSelectionItem(_L("Pause and Dim Display"));
	mpAwayAction->AddSelectionItem(_L("Pause and Power off the display"));
	mpAwayAction->SetSelectedItem(2);

	pGUI->CreateSlider(_L("User away timeout interval: 0.00"), ID_AWAY_TIMEOUT, ID_MAIN_PANEL, &mpAwayInterval);
	mpAwayInterval->SetScale(0.5, 10.5, 41);
	mpAwayInterval->SetValue(1.25);
	pGUI->CreateText( _L(" "), ID_IGNORE_CONTROL_ID, ID_MAIN_PANEL);

	// privacy screen+action(s)
	pGUI->CreateText( _L("Privacy action:"), ID_IGNORE_CONTROL_ID, ID_MAIN_PANEL);
	pGUI->CreateDropdown(_L("Nothing"), ID_AWAY_ACTION, ID_MAIN_PANEL, &mpPrivacyAction);		
	mpPrivacyAction->AddSelectionItem(_L("Pause"));
	mpPrivacyAction->AddSelectionItem(_L("Pause and Dim Display"));
	mpPrivacyAction->AddSelectionItem(_L("Change content"));
	mpPrivacyAction->SetSelectedItem(2);
	pGUI->CreateSlider(_L("Privacy timeout interval: 0.00"), ID_PRIVACY_INTERVAL, ID_MAIN_PANEL, &mpPrivacyInterval);
	mpPrivacyInterval->SetScale(0.5, 10.5, 41);
	mpPrivacyInterval->SetValue(1.25);

    // Create Static text
    CPUTText *pStaticText = NULL;
    pGUI->CreateText( _L("F1 for Help"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("[Escape] to quit application"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("A,S,D,F - move camera position"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("Q - camera position up"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("E - camera position down"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("mouse + right click - camera look location"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);

    pGUI->SetActivePanel(ID_MAIN_PANEL);
    pGUI->DrawFPS(true);

    // Add our programatic (and global) material parameters
    CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerFrameValues"), _L("$cbPerFrameValues") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerModelValues"), _L("$cbPerModelValues") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("_Shadow"), _L("$shadow_depth") );

    // load shadow casting material+sprite object
    cString ExecutableDirectory;
    CPUTFileSystem::GetExecutableDirectory(&ExecutableDirectory);
    pAssetLibrary->SetMediaDirectoryName(  ExecutableDirectory+_L("..\\..\\..\\Media\\"));
    mpShadowRenderTarget = new CPUTRenderTargetDepth();
    mpShadowRenderTarget->CreateRenderTarget( cString(_L("$shadow_depth")), SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT, DXGI_FORMAT_D32_FLOAT );

    mpDebugSprite = new CPUTSprite();
    mpDebugSprite->CreateSprite( -1.0f, -1.0f, 0.5f, 0.5f, _L("%Sprite") );
    // Override default sampler desc for our default shadowing sampler
    pStates->SamplerDesc[1].Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    pStates->SamplerDesc[1].AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
    pStates->SamplerDesc[1].AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
    pStates->SamplerDesc[1].ComparisonFunc = D3D11_COMPARISON_GREATER;
    pBlock->CreateNativeResources();
    pAssetLibrary->AddRenderStateBlock( _L("$DefaultRenderStates"), pBlock );
    pBlock->Release(); // We're done with it.  The library owns it now.

    mpScene = new CPUTScene();

    if (!g_OpenFilePath.empty())
    {
        CPUTAssetSet *pAssetSet = NULL;
        pAssetLibrary->SetMediaDirectoryName( g_OpenFilePath );
        pAssetSet        = pAssetLibrary->GetAssetSet( g_OpenFileName );
        mpScene->AddAssetSet(pAssetSet);
        pAssetSet->Release();
    } else {
        if (CPUTFAILED(mpScene->LoadScene(g_OpenSceneFileName))) {
            CPUTAssetSet *pAssetSet = NULL;
            g_OpenFilePath = _L("../../../Media/Hallway/");
            g_OpenFileName = _L("hallway");
            pAssetLibrary->SetMediaDirectoryName( g_OpenFilePath );
            pAssetSet        = pAssetLibrary->GetAssetSet( g_OpenFileName );
			g_OpenFileName = _L("xyplane");
			pAssetSet = pAssetLibrary->GetAssetSet( g_OpenFileName );
            mpScene->AddAssetSet(pAssetSet);
            pAssetSet->Release();
        }
    }
	g_OpenFilePath = _L("../../../Media/Hallway/");
	pAssetLibrary->SetMediaDirectoryName( g_OpenFilePath );
	mpSceneSet = pAssetLibrary->GetAssetSet(_L("Hallway"));
	g_OpenFilePath = _L("../../../Media/Overlay/");
	pAssetLibrary->SetMediaDirectoryName( g_OpenFilePath );
	mpOverlaySet = pAssetLibrary->GetAssetSet(_L("xyplane"));

	//CPUTModel* pModel = NULL;
	//mpOverlaySet->GetAssetByIndex(2, (CPUTRenderNode**) &pModel);
	//ASSERT(pModel, _L("Could not find proper overlay model"));
	//pModel->SetParentMatrix(float4x4Scale(0.5f, 0.5f, 0.5f));
	
    // Get the camera. Get the first camera encountered in the scene or
    // if there are none, create a new one.
    unsigned int numAssets = mpScene->GetNumAssetSets();
    for (unsigned int i = 0; i < numAssets; ++i) {
        CPUTAssetSet *pAssetSet = mpScene->GetAssetSet(i);
        if (pAssetSet->GetCameraCount() > 0) {
            mpCamera = pAssetSet->GetFirstCamera();
            mpCamera->AddRef();
            break;
        }
    }

	// create an orthographic view camera for overlays
    mpOverlayCamera = new CPUTCamera();
    mpOverlayCamera->SetProjectionMatrix(float4x4Identity()); 
    mpOverlayCamera->SetParentMatrix(float4x4Identity());
    
    if (mpCamera == NULL) {
        mpCamera = new CPUTCamera();
        pAssetLibrary->AddCamera( _L("SampleStart Camera"), mpCamera );

        mpCamera->SetPosition( 0.0f, 0.0f, 5.0f );
        // Set the projection matrix for all of the cameras to match our window.
        // TODO: this should really be a viewport matrix.  Otherwise, all cameras will have the same FOV and aspect ratio, etc instead of just viewport dimensions.
        mpCamera->SetAspectRatio(((float)width)/((float)height));
    }

    mpCamera->SetFov(DirectX::XMConvertToRadians(90.0f)); // TODO: Fix converter's FOV bug (Maya generates cameras for which fbx reports garbage for fov)
    mpCamera->SetFarPlaneDistance(100000.0f);
    mpCamera->Update();

    // Position and orient the shadow camera so that it sees the whole scene.
    // Set the near and far planes so that the frustum contains the whole scene.
    // Note that if we are allowed to move the shadow camera or the scene, this may no longer be true.
    // Consider updating the shadow camera when it (or the scene) moves.
    // Treat bounding box half as radius for an approximate bounding sphere.
    // The tightest-fitting sphere might be smaller, but this is close enough for placing our shadow camera.
    float3 sceneCenterPoint, halfVector;
    mpScene->GetBoundingBox(&sceneCenterPoint, &halfVector);
    float  length = halfVector.length();
    mpShadowCamera = new CPUTCamera( CPUT_CAMERA_TYPE_ORTHOGRAPHIC );
    mpShadowCamera->SetAspectRatio(1.0f);
    mpShadowCamera->SetNearPlaneDistance(1.0f);
    mpShadowCamera->SetFarPlaneDistance(2.0f*length + 1.0f);
    mpShadowCamera->SetPosition( sceneCenterPoint - gLightDir * length );
    mpShadowCamera->LookAt( sceneCenterPoint );
    mpShadowCamera->SetWidth( length*2);
    mpShadowCamera->SetHeight(length*2);
    mpShadowCamera->Update();

    pAssetLibrary->AddCamera( _L("ShadowCamera"), mpShadowCamera );

    mpCameraController = new CPUTCameraControllerFPS();
    mpCameraController->SetCamera(mpCamera);
    mpCameraController->SetLookSpeed(0.004f);
    mpCameraController->SetMoveSpeed(20.0f);

	// run one frame through the assist module to initialize it
	mpAssistModule->AcquireFrame(false);
	mpAssistModule->OnNewFrame();
	mpAssistModule->ReleaseFrame();

	// set window focus	
	SetFocus(mpWindow->GetHWnd());
}

//-----------------------------------------------------------------------------
void MySample::Update(double deltaSeconds)
{
	if(mpAssistModule)
	{   
		// allow the PerC module to process a frame
		mpAssistModule->AcquireFrame(false);
		mpAssistModule->OnNewFrame();
		mpAssistModule->ReleaseFrame();


		int count = mpAssistModule->GetNumberOfDetectedViewers();
		cString message = _L("Number of persons detected: ") + std::to_wstring(count);
		mpDetectCount->SetText(message);


		// update the text to say if a user is present or not
		if(true==mpAssistModule->IsUserPresent())
		{
			mpPresentText->SetText(_L("User detected"));
		}
		else
		{
			mpPresentText->SetText(_L("No user detected"));
		}
	}


    if (mpWindow->DoesWindowHaveFocus())
    {
        mpCameraController->Update((float)deltaSeconds);
    }
}

// Handle keyboard events
//-----------------------------------------------------------------------------
CPUTEventHandledCode MySample::HandleKeyboardEvent(CPUTKey key)
{
    static bool panelToggle = false;
    CPUTEventHandledCode    handled = CPUT_EVENT_UNHANDLED;
    cString fileName;
    CPUTGuiControllerDX11*  pGUI = CPUTGetGuiController();

    switch(key)
    {
    case KEY_F1:
        panelToggle = !panelToggle;
        if(panelToggle)
        {
            pGUI->SetActivePanel(ID_SECONDARY_PANEL);
        }
        else
        {
            pGUI->SetActivePanel(ID_MAIN_PANEL);
        }
        handled = CPUT_EVENT_HANDLED;
        break;
    case KEY_L:
        {
            static int cameraObjectIndex = 0;
            CPUTRenderNode *pCameraList[] = { mpCamera, mpShadowCamera };
            cameraObjectIndex = (++cameraObjectIndex) % (sizeof(pCameraList)/sizeof(*pCameraList));
            CPUTRenderNode *pCamera = pCameraList[cameraObjectIndex];
            mpCameraController->SetCamera( pCamera );
        }
        handled = CPUT_EVENT_HANDLED;
        break;
    case KEY_ESCAPE:
        handled = CPUT_EVENT_HANDLED;
		mpAssistModule->EnableCameraWindow(false);
        ShutdownAndDestroy();
        break;
    }

    // pass it to the camera controller
    if(handled == CPUT_EVENT_UNHANDLED)
    {
        handled = mpCameraController->HandleKeyboardEvent(key);
    }
    return handled;
}


// Handle mouse events
//-----------------------------------------------------------------------------
CPUTEventHandledCode MySample::HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state)
{
    if( mpCameraController )
    {
        return mpCameraController->HandleMouseEvent(x, y, wheel, state);
    }
    return CPUT_EVENT_UNHANDLED;
}

// Handle any control callback events
//-----------------------------------------------------------------------------
void MySample::HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl )
{
    UNREFERENCED_PARAMETER(Event);
    

	CPUTCheckbox* pCheckbox=NULL;
    cString SelectedItem;
    static bool resize = false;
	float SliderValue=0.0f;
	cString text;
	TCHAR buffer[1024];

    switch(ControlID)
    {
    case ID_FULLSCREEN_BUTTON:
        CPUTToggleFullScreenMode();
        break;

	case ID_CAMERA_WINDOW:
		pCheckbox = (CPUTCheckbox*) pControl;
		if(CPUT_CHECKBOX_CHECKED == pCheckbox->GetCheckboxState())
		{
			mpAssistModule->EnableCameraWindow(true);
		}
		else
		{
			mpAssistModule->EnableCameraWindow(false);
		}

	case ID_AWAY_TIMEOUT:
		// update the user is away timeout string
		mpAwayInterval->GetValue(SliderValue);	
		_stprintf_s(&buffer[0], 1024, _L("%.2f"), SliderValue);

		text = _L("User away timeout interval: ");
		text = text + buffer;
		mpAwayInterval->SetText(text);

		// set the timeout interval on the assist module
		mpAssistModule->SetPauseTimeoutDuration(SliderValue);		
		break;

	case ID_PRIVACY_INTERVAL:	
		// update the user is away timeout string
		mpPrivacyInterval->GetValue(SliderValue);	
		_stprintf_s(&buffer[0], 1024, _L("%.2f"), SliderValue);

		text = _L("Privacy timeout interval: ");
		text = text + buffer;
		mpPrivacyInterval->SetText(text);

		// set the timeout interval on the assist module
		mpAssistModule->SetPrivacyTimeoutDuration(SliderValue);


		break;

    default:
        break;
    }
}

//-----------------------------------------------------------------------------
void MySample::ResizeWindow(UINT width, UINT height)
{
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    // Before we can resize the swap chain, we must release any references to it.
    // We could have a "AssetLibrary::ReleaseSwapChainResources(), or similar.  But,
    // Generic "release all" works, is simpler to implement/maintain, and is not performance critical.
    pAssetLibrary->ReleaseTexturesAndBuffers();

    // Resize the CPUT-provided render target
    CPUT_DX11::ResizeWindow( width, height );

    // Resize any application-specific render targets here
    if( mpCamera ) mpCamera->SetAspectRatio(((float)width)/((float)height));

    pAssetLibrary->RebindTexturesAndBuffers();
}

//-----------------------------------------------------------------------------
void MySample::Render(double deltaSeconds)
{
    CPUTRenderParametersDX renderParams(mpContext);

    //*******************************
    // Draw the shadow scene
    //*******************************
    CPUTCamera *pLastCamera = mpCamera;
    mpCamera = renderParams.mpCamera = mpShadowCamera;
    mpShadowRenderTarget->SetRenderTarget( renderParams, 0, 0.0f, true );

    mpScene->Render( renderParams, CPUT_MATERIAL_INDEX_SHADOW_CAST);

    mpShadowRenderTarget->RestoreRenderTarget(renderParams );
    mpCamera = renderParams.mpCamera = pLastCamera;

    // Save the light direction to a global so we can set it to a constant buffer later (TODO: have light do this)
    gLightDir = mpShadowCamera->GetLook();

    // Clear back buffer
    const float clearColor[] = { 0.0993f, 0.0993f, 0.0993f, 1.0f };
    mpContext->ClearRenderTargetView( mpBackBufferRTV,  clearColor );
    mpContext->ClearDepthStencilView( mpDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

	// render the hallway
	//mpSceneSet->RenderRecursive( renderParams );

	// draw the pause overlay if the user has left the camera's view and settings say to do so
	//UINT Selection;
	//bool UserPresentPause = false;
	//bool UserPresentPowerOff = false;
	//bool UserPresentDim = false;
	//bool PrivacyPauseOn = false;
	//bool PrivacyDim = false;

	// deal with user away/present
	//mpAwayAction->GetSelectedItem(Selection);
	//if( !mpAssistModule->IsUserPresent() && (
	//	(ID_PAUSE == Selection) ||
	//	(ID_PAUSE_AND_DIM_DISPLAY == Selection) ||
	//	(ID_PAUSE_AND_POWER_OFF_DISPLAY == Selection)
	//	))
	//{
	//	UserPresentPause = true;
	//	if(ID_PAUSE_AND_POWER_OFF_DISPLAY == Selection)
	//	{
	//		 power off display
	//		UserPresentPowerOff = true;
	//	}
	//}	

	//if( !mpAssistModule->IsUserPresent() && (
	//	(ID_DIM_DISPLAY == Selection) ||
	//	(ID_PAUSE_AND_DIM_DISPLAY == Selection) 
	//	))
	//{
	//	 dim display
	//	UserPresentDim = true;
	//}


	// deal with privacy mode
	//mpPrivacyAction->GetSelectedItem(Selection);	
	//if( mpAssistModule->IsPrivacyModeOn() && (
	//	(ID_PRIVACY_PAUSE == Selection) ||
	//	(ID_PRIVACY_PAUSE_AND_DIM_DISPLAY == Selection)
	//	))
	//{
	//	PrivacyPauseOn = true;
	//	if(ID_PRIVACY_PAUSE_AND_DIM_DISPLAY == Selection)
	//	{
	//		PrivacyDim = true;
	//	}
	//}

	// deal with voice commands
	//VOICE_COMMAND newVoiceCommand = mpAssistModule->GetLastVoiceCommand();
	//if(VOICE_COMMAND_NONE != newVoiceCommand)
	//{
	//	switch( newVoiceCommand )
	//	{
	//	case VOICE_COMMAND_PAUSE_GAME:
	//		mVoicePaused = true;
	//		break;
	//	case VOICE_COMMAND_RESUME_GAME:
	//		mVoicePaused = false;
	//		break;
	//	}		
	//}


	// handle monitor state
	//if(PrivacyDim || UserPresentDim)
	//{
	//	if(true != mDisplayInLowPower)
	//	{
	//		mDisplayInLowPower = true;
	//		 dim the display
	//	}
	//	
	//}
	//else if(UserPresentPowerOff)
	//{
	//	if(true != mDisplayInLowPower)
	//	{
	//		mDisplayInLowPower = true;
	//		 power off
	//	}
	//}
	//else
	//{
	//	if(mDisplayInLowPower)
	//	{
	//		 issue restore power command
	//		mDisplayInLowPower = false; 
	//	}
	//}


	if(DetermineGameState())
	{
		pLastCamera = mpCamera;
		mpCamera = renderParams.mpCamera = mpOverlayCamera;
		mpOverlaySet->RenderRecursive( renderParams );
		mpCamera = renderParams.mpCamera = pLastCamera;
	}

    if(mpCameraController->GetCamera() == mpShadowCamera)
    {
        mpDebugSprite->DrawSprite(renderParams);
    }
    CPUTDrawGUI();
}

//
//-----------------------------------------------------------------------------
bool MySample::DetermineGameState()
{
	// draw the pause overlay if the user has left the camera's view and settings say to do so
	UINT Selection;
	bool UserPresentPause = false;
	bool UserPresentPowerOff = false;
	bool UserPresentDim = false;
	bool PrivacyPauseOn = false;
	bool PrivacyDim = false;

	// deal with user away/present
	mpAwayAction->GetSelectedItem(Selection);
	if( !mpAssistModule->IsUserPresent() && (
		(ID_PAUSE == Selection) ||
		(ID_PAUSE_AND_DIM_DISPLAY == Selection) ||
		(ID_PAUSE_AND_POWER_OFF_DISPLAY == Selection)
		))
	{
		UserPresentPause = true;
		if(ID_PAUSE_AND_POWER_OFF_DISPLAY == Selection)
		{
			// power off display
			UserPresentPowerOff = true;
		}
	}	

	if( !mpAssistModule->IsUserPresent() && (
		(ID_DIM_DISPLAY == Selection) ||
		(ID_PAUSE_AND_DIM_DISPLAY == Selection) 
		))
	{
		// dim display
		UserPresentDim = true;
	}


	// deal with privacy mode
	mpPrivacyAction->GetSelectedItem(Selection);	
	if( mpAssistModule->IsPrivacyModeOn() && (
		(ID_PRIVACY_PAUSE == Selection) ||
		(ID_PRIVACY_PAUSE_AND_DIM_DISPLAY == Selection)
		))
	{
		PrivacyPauseOn = true;
		if(ID_PRIVACY_PAUSE_AND_DIM_DISPLAY == Selection)
		{
			PrivacyDim = true;
		}
	}

	// deal with voice commands
	VOICE_COMMAND newVoiceCommand = mpAssistModule->GetLastVoiceCommand();
	if(VOICE_COMMAND_NONE != newVoiceCommand)
	{
		switch( newVoiceCommand )
		{
		case VOICE_COMMAND_PAUSE_GAME:
			mVoicePaused = true;
			break;
		case VOICE_COMMAND_RESUME_GAME:
			mVoicePaused = false;
			break;
		}		
	}


	// handle monitor state
	if(PrivacyDim || UserPresentDim)
	{
		if(true != mDisplayInLowPower)
		{
			mDisplayInLowPower = true;
			// dim the display
		}
		
	}
	else if(UserPresentPowerOff)
	{
		if(true != mDisplayInLowPower)
		{
			mDisplayInLowPower = true;
			// power off
		}
	}
	else
	{
		if(mDisplayInLowPower)
		{
			// issue restore power command
			mDisplayInLowPower = false; 
		}
	}


	if(mVoicePaused || PrivacyPauseOn || UserPresentPause)
	{
		return true;
	}
	return false;
}