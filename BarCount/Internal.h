/*******************************************************************************************************/
/* Project : Bar Count                                                                                 */
/*                                                                                                     */
/* Author : G.M.Crean                                                                                  */
/*                                                                                                     */
/*    (c)2015 Zebra Technologies                                                                       */
/*******************************************************************************************************/
#pragma once
#include <windows.h>
#include <aygshell.h>
#include <ScanCApi.h>

//Include Libraries
#pragma comment(lib,"aygshell.lib")
#pragma comment(lib,"SCNAPI32.lib")

//Definitions
#define ARRAYSIZE(s)		(sizeof(s) / sizeof(s[0]))
#define SCALE(s)			(iScale * (s))
#define MAX_BUFFER			1024
#define WNDCLASSNAME		L"BARCOUNT_CLS"
#define MSG_BARCODE			WM_APP + 1

//Structures
struct DecodeMessage
{                             
    UINT Code;                                  
    LRESULT (*Fxn)(HWND, UINT, WPARAM, LPARAM);
};
