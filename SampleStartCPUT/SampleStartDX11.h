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
#ifndef __CPUT_SAMPLESTARTDX11_H__
#define __CPUT_SAMPLESTARTDX11_H__

#include <stdio.h>
#include "CPUT_DX11.h"
#include <D3D11.h>
#include <DirectXMath.h>
#include <time.h>
#include "CPUTSprite.h"
#include "CPUTScene.h"
#include "PerceptualAssistant.h"

// define some controls
const CPUTControlID ID_MAIN_PANEL = 10;
const CPUTControlID ID_SECONDARY_PANEL = 20;
const CPUTControlID ID_FULLSCREEN_BUTTON = 100;
const CPUTControlID ID_NEXTMODEL_BUTTON = 101;
const CPUTControlID ID_CAMERA_WINDOW = 1000;
const CPUTControlID ID_DETECTED_PERSONS_STRING = 1001;
const CPUTControlID ID_AWAY_ACTION = 1002;
const CPUTControlID ID_AWAY_TIMEOUT = 1003;
const CPUTControlID ID_PRIVACY_DETECTION = 1004;
const CPUTControlID ID_PRIVACY_INTERVAL = 1005;

const CPUTControlID ID_IGNORE_CONTROL_ID = -1;

const int ID_NOTHING = 0;
const int ID_PAUSE = 1;
const int ID_DIM_DISPLAY = 2;
const int ID_PAUSE_AND_DIM_DISPLAY = 3;
const int ID_PAUSE_AND_POWER_OFF_DISPLAY = 4;

const int ID_PRIVACY_NOTHING = 0;
const int ID_PRIVACY_PAUSE = 1;
const int ID_PRIVACY_PAUSE_AND_DIM_DISPLAY = 2;
const int ID_PRIVACY_CHANGE_CONTENT = 3;

//-----------------------------------------------------------------------------
class MySample : public CPUT_DX11
{
private:
    float                  mfElapsedTime;
    CPUTCameraController  *mpCameraController;
    CPUTSprite            *mpDebugSprite;

    CPUTAssetSet          *mpShadowCameraSet;
    CPUTRenderTargetDepth *mpShadowRenderTarget;

    CPUTText              *mpFPSCounter;
	CPUTText              *mpDetectCount;
	CPUTText			  *mpPresentText;
	
	CPUTDropdown		  *mpAwayAction;
	CPUTSlider			  *mpAwayInterval;

	CPUTDropdown		  *mpPrivacyAction;
	CPUTSlider			  *mpPrivacyInterval;

    CPUTScene             *mpScene;
	CPUTAssetSet		  *mpSceneSet;
	CPUTAssetSet		  *mpOverlaySet;
	CPUTCamera            *mpOverlayCamera;

	PerCAssist			  *mpAssistModule;

	bool				  mVoicePaused;
	bool				  mDisplayInLowPower;

public:
    MySample() : 
        mpCameraController(NULL),
        mpDebugSprite(NULL),
        mpShadowCameraSet(NULL),
		mpScene(NULL),
		mpSceneSet(NULL),
		mpOverlaySet(NULL),
		mpOverlayCamera(NULL),
		mpAssistModule(NULL),
		mVoicePaused(false),
		mDisplayInLowPower(false)
    {}
    virtual ~MySample()
    {
        // Note: these two are defined in the base.  We release them because we addref them.
        SAFE_RELEASE(mpCamera);
        SAFE_RELEASE(mpShadowCamera);

        SAFE_DELETE( mpCameraController );
        SAFE_DELETE( mpDebugSprite);
        SAFE_RELEASE(mpShadowCameraSet);
        SAFE_DELETE( mpShadowRenderTarget );
        SAFE_DELETE( mpScene );
		SAFE_RELEASE( mpSceneSet );
		SAFE_RELEASE( mpOverlaySet );
		SAFE_RELEASE( mpOverlayCamera );
		SAFE_DELETE( mpAssistModule );
    }
    virtual CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key);
    virtual CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state);
    virtual void                 HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl );

    virtual void Create();
    virtual void Render(double deltaSeconds);
    virtual void Update(double deltaSeconds);
    virtual void ResizeWindow(UINT width, UINT height);

    void LoadAssets();
	
	bool DetermineGameState();
};
#endif // __CPUT_SAMPLESTARTDX11_H__
