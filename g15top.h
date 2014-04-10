#pragma once

#include "resource.h"

#define LOGI_LCD_TYPE_MONO    (0x00000001)
#define LOGI_LCD_MONO_BUTTON_0 (0x00000001)
#define LOGI_LCD_MONO_BUTTON_1 (0x00000002)
#define LOGI_LCD_MONO_BUTTON_2 (0x00000004)
#define LOGI_LCD_MONO_BUTTON_3 (0x00000008)

typedef bool (* LPFNDLLINIT)(wchar_t*, int );
typedef bool (* LPFNDLLISBUTTONPRESSED)(int);
typedef void (* LPFNDLLUPDATE)();
typedef void (* LPFNDLLSHUTDOWN)();
typedef bool (* LPFNDLLMONOSETTEXT)(int, wchar_t*);

LPFNDLLINIT LogiLcdInit = NULL;
LPFNDLLISBUTTONPRESSED LogiLcdIsButtonPressed = NULL;
LPFNDLLUPDATE LogiLcdUpdate = NULL;
LPFNDLLSHUTDOWN LogiLcdShutdown = NULL;
LPFNDLLMONOSETTEXT LogiLcdMonoSetText = NULL;

void DLL_Init();
void WMI_Init();
void WMI_Close();
bool access_compare(IWbemObjectAccess*, IWbemObjectAccess*);
void WMI_Refresh();