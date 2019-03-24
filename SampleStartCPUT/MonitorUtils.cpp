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
#include "CPUT.h"
#include "MonitorUtils.h"
#include <HighLevelMonitorConfigurationAPI.h> //SetMonitorBrightness - must include DXVA2.lib

// Constructor
//--------------------------------------------------------------------------------------
MonitorUtils::MonitorUtils():m_pPhysMonitors(NULL), mpMinimumBrightness(NULL), mpUserBrightness(NULL), mpMaximumBrightness(NULL), mPhysicalMonitorArraySize(0)
{

}

// Destructor
//--------------------------------------------------------------------------------------
MonitorUtils::~MonitorUtils()
{
	if(m_pPhysMonitors)
	{
		delete [] m_pPhysMonitors;
		m_pPhysMonitors=NULL;
	}

	if(mpMinimumBrightness)
	{
		delete [] mpMinimumBrightness;
		mpMinimumBrightness=NULL;
	}
	if(mpUserBrightness)
	{
		delete [] mpUserBrightness;
		mpUserBrightness=NULL;
	}
	if(mpMaximumBrightness)
	{
		delete [] mpMaximumBrightness;
		mpMaximumBrightness=NULL;
	}
	
	mPhysicalMonitorArraySize = 0;
}

// Initialization
//--------------------------------------------------------------------------------------
bool MonitorUtils::Create(HWND hWnd)
{
	//HWND hWnd = mpWindow->GetHWnd();

	// get the monitor handle associated with this hWnd
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY) ;
	if(!hMonitor)
	{
		ASSERT(0, _L("Error: Could not get monitor handle from supplied hWnd"));
		return false;
	}

	// get number of physical monitors attached
	if(FALSE == GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &mPhysicalMonitorArraySize))	
	{
		ReportError();
		return false;
	}
	
	// allocate storage for all the properties
	m_pPhysMonitors = new PHYSICAL_MONITOR[mPhysicalMonitorArraySize];
	mpMinimumBrightness = new DWORD[mPhysicalMonitorArraySize];
	mpUserBrightness = new DWORD[mPhysicalMonitorArraySize];
	mpMaximumBrightness = new DWORD[mPhysicalMonitorArraySize];

	// enumerate the physical monitors
	if(FALSE == GetPhysicalMonitorsFromHMONITOR(hMonitor, mPhysicalMonitorArraySize, m_pPhysMonitors))	
	{
		ReportError();
		return false;
	}
	 
	//Capabilities
	//DWORD MonitorCapabilities, SupportedColorTemperatures;
	//if(FALSE == GetMonitorCapabilities(m_pPhysMonitors[0].hPhysicalMonitor, &MonitorCapabilities, &SupportedColorTemperatures))
	//{
	//	ReportError();
	//	return false;
	//}

	for(DWORD i=0; i<mPhysicalMonitorArraySize; i++)
	{
		if(FALSE == GetMonitorBrightness(m_pPhysMonitors[i].hPhysicalMonitor, &mpMinimumBrightness[i], &mpUserBrightness[i], &mpMaximumBrightness[i]))
		{
			ReportError();
			return false;
		}
	}

	//if(FALSE == SetMonitorBrightness(m_pPhysMonitors[0].hPhysicalMonitor, mpMinimumBrightness[0]))
	//{
	//	ASSERT(0, _L("ERROR"));
	//	return false;
	//}

	//if(FALSE == SetMonitorBrightness(m_pPhysMonitors[0].hPhysicalMonitor, mpCurrentBrightness[0]))
	//{
	//	ASSERT(0, _L("ERROR"));
	//	return false;
	//}

	return true;
}


//--------------------------------------------------------------------------------------
bool MonitorUtils::SetDisplayBrightness(MonitorBrightness eBrightnessLevel)
{
	if(MonitorBrightnessOff == eBrightnessLevel)
	{
		// todo: better solution is to send in own hwnd
		SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM) 2); 
	}
	else
	{
		ULONG value=0;
		mouse_event(1, 0, 1, 0, value);
		Sleep(40);
		mouse_event(1, 0, -1, 0, value);
		POINT position;
		GetCursorPos(&position);

		WPARAM VirtualKeysDown = 0;
		LPARAM samePoint = MAKELONG(position.x, position.y);
		//SendMessage(HWND_BROADCAST, WM_MOUSEMOVE, VirtualKeysDown, (LPARAM)samePoint);

		//SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM) -1); 


		DWORD *pDesiredBrightness = NULL;
		
		switch(eBrightnessLevel)
		{
		case MonitorBrightnessFull:
			pDesiredBrightness = mpMaximumBrightness;
			break;

		case MonitorBrightnessNormal:
			pDesiredBrightness = mpUserBrightness;
			break;

		case MonitorBrightnessDim:
			pDesiredBrightness = mpMinimumBrightness;
			break;
		}

		ASSERT(pDesiredBrightness, _L("Invalid monitor brightness"));

		// walk all attached monitors and perform the update
		for(DWORD i=0; i<mPhysicalMonitorArraySize; i++)
		{
			if(FALSE == SetMonitorBrightness(m_pPhysMonitors[i].hPhysicalMonitor, pDesiredBrightness[i]))
			{
				ASSERT(0, _L("ERROR"));
				return false;
			}
		}
	}
	return true;
}



//--------------------------------------------------------------------------------------
void MonitorUtils::ReportError()
{
		DWORD nErrorCode = GetLastError();
		TCHAR* msg;
		// Ask Windows to prepare a standard message for a GetLastError() code:
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0, NULL);
		ASSERT(0, msg);
		LocalFree(msg);
}