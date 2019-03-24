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
#include "PerceptualAssistant.h"
#include "CPUT.h"



// Constructor
//-----------------------------------------------------------------------------
PerCAssist::PerCAssist():m_pFaceRender(NULL),
	mNumFacesDetected(0),
	mIsPresent(true),
	mPrivacyOn(false),
	mPresentTimeoutInterval(1.25),	// 1 1/4 second timeout by default
	mPrivacyTimeoutInterval(1.25),	// 1 1/4 second timeout by default 
	mLastVoiceCommand(VOICE_COMMAND_NONE)
{
	Create();
}

// Destructor
//-----------------------------------------------------------------------------
PerCAssist::~PerCAssist()
{
	SAFE_DELETE(m_pFaceRender);
}

//-----------------------------------------------------------------------------
int PerCAssist::Create()
{	
	// enable face tracking module
	EnableFaceLocation();

	// enable voice module
	EnableVoiceRecognition();

	// Call super-class initialize
	// Must be done after enabling and configuring all the submodules
	Init();	

	
	std::vector<std::wstring> CommandList;

	int index=0;
	while('\0' != *Commands[index])
	{
		std::wstring Command = Commands[index];
		CommandList.push_back(Command);
		index++;
	}
	
	// set the voice commands
	SetVoiceCommands(CommandList);

	// display the face tracking window by default
	m_pFaceRender = new FaceRender(L"Face Detection");

	return 0;
}

//-----------------------------------------------------------------------------
void PXCAPI PerCAssist::OnRecognized(PXCVoiceRecognition::Recognition *data)
{
	mLastVoiceCommand = VOICE_COMMAND_NONE;

	switch(data->label)
	{
	case VOICE_COMMAND_PAUSE_GAME:
		mLastVoiceCommand = VOICE_COMMAND_PAUSE_GAME;
		TRACE(_L("VOICE COMMAND: PAUSE GAME\n"));
		break;
	case VOICE_COMMAND_RESUME_GAME:
		mLastVoiceCommand = VOICE_COMMAND_RESUME_GAME;
		TRACE(_L("VOICE COMMAND: RESUME GAME\n"));
		break;
	case VOICE_COMMAND_DEBUG:
		mLastVoiceCommand = VOICE_COMMAND_DEBUG;
		TRACE(_L("VOICE COMMAND: DEBUG\n"));
		break;
	default:
		TRACE(_L("VOICE COMMAND: UNRECOGNIZED\n"));
	}
}

// Return the last un-processed voice command
// After you query this, it clears the command so that you only get new un-processed
// commands
//-----------------------------------------------------------------------------
VOICE_COMMAND PerCAssist::GetLastVoiceCommand()
{
	VOICE_COMMAND temp = mLastVoiceCommand;
	mLastVoiceCommand = VOICE_COMMAND_NONE;

	return temp;
}

//-----------------------------------------------------------------------------
void PerCAssist::SetPauseTimeoutDuration(float seconds)
{
	mPresentTimeoutInterval = seconds;
	ResetTimers();
}

//-----------------------------------------------------------------------------
void PerCAssist::SetPrivacyTimeoutDuration(float seconds)
{
	mPrivacyTimeoutInterval = seconds;
	ResetTimers();
}


// Reset the timers
//-----------------------------------------------------------------------------
void PerCAssist::ResetTimers()
{
	// reset timers if someone changes this
	mIsPresent = true;
	mAbsentTimer.ResetTimer();
	mPresentTimer.ResetTimer();
}

// Display the debug camera window 
//-----------------------------------------------------------------------------
void PerCAssist::EnableCameraWindow(bool DisplayCameraWindow)
{
	if((true==DisplayCameraWindow) && (NULL==m_pFaceRender))
    {
        m_pFaceRender = new FaceRender(L"Face Detection");
    }
    else
    {
        SAFE_DELETE(m_pFaceRender);
    }
}

//-----------------------------------------------------------------------------
bool PerCAssist::OnNewFrame(void) 
{
	//return true;

	bool faceDetect = false;

    // PC-SDK defined data structs
    PXCFaceAnalysis::Detection::Data        faceData;
    //PXCFaceAnalysis::Landmark::LandmarkData lmData;


    // Get face, landmark interfaces
    PXCFaceAnalysis *faceAnalyzer = QueryFace();
    PXCFaceAnalysis::Detection *pDetectIfc = faceAnalyzer->DynamicCast<PXCFaceAnalysis::Detection>();
    //PXCFaceAnalysis::Landmark  *pLandmarkIfc = faceAnalyzer->DynamicCast<PXCFaceAnalysis::Landmark>();

    // clear feedback window, if enabled
    if (m_pFaceRender) m_pFaceRender->ClearData();

    // Request the first face in the frame (TODO: track a 'primary' face, if multiple are found)
    pxcUID faceID = 0;
    
	mNumFacesDetected = 0;



	pxcStatus sts;
	for(int fidx =0; ; fidx++)
	{
            pxcUID fid = 0;
            pxcU64 timeStamp = 0;
            sts = faceAnalyzer->QueryFace(fidx, &fid, &timeStamp);
            if (sts < PXC_STATUS_NO_ERROR) break; // no more faces
			
			mNumFacesDetected++;
            //PXCFaceAnalysis::Detection::Data face_data;
            pDetectIfc->QueryData(fid, &faceData);
			if(m_pFaceRender)
			{
				m_pFaceRender->SetDetectionData(&faceData);
			}
	}

	// handle user present/away cases using a 'sticky' method
	// Here we use a smoothed result - instantaneous results can lead to lots of random on/off behavior.
	// User must be present or away for a short time interval before being reported as such	
	if( 0 == mNumFacesDetected )
	{
		mPresentTimer.StopTimer();

		if( false == mAbsentTimer.IsTimerRunning() )
		{
			mAbsentTimer.StartTimer();
		}
		
		double elapsed = mAbsentTimer.GetTotalTime();
		if( elapsed > mPresentTimeoutInterval )
		{
			mIsPresent = false;
		}

	}

	if( 0 != mNumFacesDetected )
	{
		mAbsentTimer.StopTimer();

		
		if( false == mPresentTimer.IsTimerRunning() )
		{
			mPresentTimer.StartTimer();
		}
		
		double elapsed = mPresentTimer.GetTotalTime();
		if( elapsed > mPresentTimeoutInterval )
		{
			mIsPresent = true;
		}
	}


	// privacy screen
	if( 1>= mNumFacesDetected)
	{
		mPrivacyTimer.StopTimer();
		
		if( false == mPrivacyOffTimer.IsTimerRunning() )
		{
			mPrivacyOffTimer.StartTimer();
		}
		
		double elapsed = mPrivacyOffTimer.GetTotalTime();
		if( elapsed > mPrivacyTimeoutInterval )
		{
			mPrivacyOn = false;
		}
	}

	
	if( 1 < mNumFacesDetected)
	{
		mPrivacyOffTimer.StopTimer();

		if( false == mPrivacyTimer.IsTimerRunning() )
		{
			mPrivacyTimer.StartTimer();
		}

		double elapsed = mPrivacyTimer.GetTotalTime();
		if( elapsed > mPrivacyTimeoutInterval )
		{
			mPrivacyOn = true;
		}		
	}
	

	// Draw face and landmark data in feedback window, if enabled
	if (m_pFaceRender) 
	{
		//m_pFaceRender->SetDetectionData( &faceData );   
		//m_pFaceRender->SetLandmarkData( pLandmarkIfc, faceID );

		// Update window
		m_pFaceRender->RenderFrame( QueryImage( PXCImage::IMAGE_TYPE_COLOR ) );
	}





    return true;
}
