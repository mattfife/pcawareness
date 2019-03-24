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
#ifndef __MONITOR_UTILS_H__
#define __MONITOR_UTILS_H__
#include <stdio.h>
#include <LowLevelMonitorConfigurationAPI.h> //- must include Dxva2.lib

enum MonitorBrightness
{
	MonitorBrightnessFull,
	MonitorBrightnessNormal,
	MonitorBrightnessDim,
	MonitorBrightnessOff,
};

class MonitorUtils
{
public:
	MonitorUtils();
	~MonitorUtils();
	bool Create(HWND hWnd);
	bool SetDisplayBrightness(MonitorBrightness eBrightnessLevel);

private:
	void ReportError();

	DWORD mPhysicalMonitorArraySize;
	PHYSICAL_MONITOR* m_pPhysMonitors;
	DWORD* mpMinimumBrightness;
	DWORD* mpUserBrightness;
	DWORD* mpMaximumBrightness;
};

#endif // #define __MONITOR_UTILS_H__