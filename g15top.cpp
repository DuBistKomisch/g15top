#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <WbemCli.h>

#include "stdafx.h"
#include "g15top.h"

using namespace std;

IWbemServices* services = NULL;
IWbemRefresher* refresher = NULL;
IWbemHiPerfEnum* refresherEnum = NULL;

IWbemObjectAccess** access = NULL;

int _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	DLL_Init();
	WMI_Init();
	LogiLcdInit(_T("top"), LOGI_LCD_TYPE_MONO);

	int i = 0;

	while (!LogiLcdIsButtonPressed(LOGI_LCD_MONO_BUTTON_0))
	{
		WMI_Refresh();

		LogiLcdUpdate();
		Sleep(2000);
	}

	LogiLcdShutdown();
	WMI_Close();
	
	return 0;
}

void DLL_Init()
{
	HINSTANCE logiDllHandle = LoadLibrary(_T("LogitechLcd.dll"));
    if (logiDllHandle != NULL)
    {
        LogiLcdInit = (LPFNDLLINIT)GetProcAddress(logiDllHandle, "LogiLcdInit");
        LogiLcdIsButtonPressed = (LPFNDLLISBUTTONPRESSED)GetProcAddress(logiDllHandle, "LogiLcdIsButtonPressed");
        LogiLcdUpdate = (LPFNDLLUPDATE)GetProcAddress(logiDllHandle, "LogiLcdUpdate");
        LogiLcdShutdown = (LPFNDLLSHUTDOWN)GetProcAddress(logiDllHandle, "LogiLcdShutdown");
        LogiLcdMonoSetText = (LPFNDLLMONOSETTEXT)GetProcAddress(logiDllHandle, "LogiLcdMonoSetText");
    }
}

void WMI_Init()
{
	CoInitializeEx(0, COINIT_MULTITHREADED);
	CoInitializeSecurity(
		NULL, 
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
	);

	// get service
	IWbemLocator* locator = NULL;
	CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator,
		(LPVOID *) &locator
	);
	locator->ConnectServer(
		_T("root\\cimv2"),           // Object path of WMI namespace
		NULL,						 // User name. NULL = current user
		NULL,						 // User password. NULL = current
		0,			                 // Locale. NULL indicates current
		NULL,			             // Security flags.
		0,					         // Authority (for example, Kerberos)
		0,						     // Context object 
		&services				     // pointer to IWbemServices proxy
	);
	locator->Release();

	// get refresher
	CoCreateInstance(
        CLSID_WbemRefresher,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWbemRefresher, 
        (void**) &refresher
	);

	// config
	IWbemConfigureRefresher* config;
	long refresherEnumId;
	refresher->QueryInterface(
        IID_IWbemConfigureRefresher,
        (void **) &config
	);
	config->AddEnum(
        services,
        _T("Win32_PerfFormattedData_PerfProc_Process"),
        0,
        NULL,
        &refresherEnum,
        &refresherEnumId
	);
	config->Release();
}

void WMI_Close()
{
	refresherEnum->Release();
	refresher->Release();
	services->Release();
	CoUninitialize();
}

bool access_compare(IWbemObjectAccess* a, IWbemObjectAccess* b)
{
	VARIANT value;

	wstringstream line;
	a->Get(_T("PercentProcessorTime"), 0, &value, NULL, NULL);
	line << value.bstrVal;
	b->Get(_T("PercentProcessorTime"), 0, &value, NULL, NULL);
	line << " " << value.bstrVal;

	int x, y;
	line >> x >> y;

	if (x > y)
		return true;
	else
		return false;
}

void WMI_Refresh()
{
	DWORD returned = 0;
    DWORD process = 0;
    DWORD objects = 0;

	VARIANT value;

	// refresh
	refresher->Refresh(0);
	if (refresherEnum->GetObjects(0, objects, access, &returned) == WBEM_E_BUFFER_TOO_SMALL)
	{
		// reallocate larger buffer
		access = new IWbemObjectAccess*[returned];
		SecureZeroMemory(access, returned * sizeof(IWbemObjectAccess*));
		objects = returned;
		refresherEnum->GetObjects(0, objects, access, &returned);
	}

	// sort
	sort(access, access + returned, access_compare);

	// process
	for (unsigned int i = 2; i < 6 && i < returned; i++)
	{
		wostringstream line;
		access[i]->Get(_T("PercentProcessorTime"), 0, &value, NULL, NULL);
		if (SysStringLen(value.bstrVal) == 1)
			line << " ";
		line << value.bstrVal << "%";
		access[i]->Get(_T("Name"), 0, &value, NULL, NULL);
		line << " " << value.bstrVal;
		LogiLcdMonoSetText(i - 2, (wchar_t *) line.str().c_str());
	}

	// clean up
	for (unsigned int i = 0; i < returned; i++)
	{
		access[i]->Release();
	}
	delete [] access;
	access = NULL;
}
