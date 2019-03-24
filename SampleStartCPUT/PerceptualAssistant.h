//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
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
#ifndef __CPUT_ASSIST_H__
#define __CPUT_ASSIST_H__

#include "CPUTTimerWin.h"
#include "MonitorUtils.h"

// required perC headers
#include <windows.h>
#include <wchar.h>
#include <vector>
#include <conio.h>
#include "util_cmdline.h"
#include "util_capture_file.h"
#include "pxcvoice.h"
#include "pxcface.h"

#include "util_pipeline.h"

#include ".//face_render//face_render.h"

// voice commands
enum VOICE_COMMAND
{	
	VOICE_COMMAND_PAUSE_GAME=0,
	VOICE_COMMAND_RESUME_GAME=1,
	VOICE_COMMAND_DEBUG=2,
	VOICE_COMMAND_NONE=99,
};

static TCHAR* Commands[] = 
{
	L"Pause game",
	L"Resume game",	
	L"Debug",
	L""
};


// PerCAssist
//-----------------------------------------------------------------------------
class PerCAssist : public UtilPipeline 
{
public:
	PerCAssist();
	virtual ~PerCAssist();
	virtual int Create();
	
	// UtilPipeline functions
	virtual bool OnNewFrame(void);
	virtual void  PXCAPI OnRecognized(PXCVoiceRecognition::Recognition *data);
	
	// PerCAssist module
	virtual void EnableCameraWindow(bool DisplayCameraWindow);
	virtual int GetNumberOfDetectedViewers() {return mNumFacesDetected;}
	virtual VOICE_COMMAND GetLastVoiceCommand();

	virtual void SetPauseTimeoutDuration(float seconds);
	virtual void SetPrivacyTimeoutDuration(float seconds);   
	
	virtual bool IsUserPresent() {return mIsPresent;}
	virtual bool IsPrivacyModeOn() {return mPrivacyOn;}
	virtual void ResetTimers();

private:
	FaceRender*	m_pFaceRender;
	int			mNumFacesDetected;

    static const UINT32                     m_cNumLandmarks = 6;
    PXCFaceAnalysis::Detection::Data        m_FaceData;
    PXCFaceAnalysis::Landmark::LandmarkData m_LandmarkData[m_cNumLandmarks];
    bool            fdValid;                    // valid face data
    bool            ldValid[m_cNumLandmarks];   // valid landmark data 

	bool			mIsPresent;
	double			mPresentTimeoutInterval;

	bool			mPrivacyOn;
	double			mPrivacyTimeoutInterval;

	VOICE_COMMAND	mLastVoiceCommand;

	CPUTTimerWin	mAbsentTimer;
	CPUTTimerWin	mPresentTimer;
	CPUTTimerWin	mPrivacyTimer;
	CPUTTimerWin	mPrivacyOffTimer;
};


#endif // #define __CPUT_ASSIST_H__
