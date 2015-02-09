/*******************************************************************************************************/
/* Project : Bar Count                                                                                 */
/*                                                                                                     */
/* Author : G.M.Crean                                                                                  */
/*                                                                                                     */
/*    (c)2015 Zebra Technologies                                                                       */
/*******************************************************************************************************/
#include "Internal.h"

//Local Varibles
static HWND					hWnd					= NULL;
static DWORD				dwWidth					= 0;
static DWORD				dwHeight				= 0;
static HINSTANCE			hInstanace				= NULL;
static HFONT				hBarcode				= NULL;
static HFONT				hCount					= NULL;
static HFONT				hText					= NULL;
static INT					iScale					= 0;
static DWORD				dwCount					= 0;
static LPSCAN_BUFFER		lpScanBuffer			= NULL;
static HANDLE				hScanner				= 0;
static HANDLE				hFile					= NULL;

//Internal Messages
static LRESULT WndPaint(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
static LRESULT WndCreate(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
static LRESULT WndDestory(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
static LRESULT WndBarcode(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
static LRESULT WndKeyUp(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

// Message dispatch table for WindowProc
const struct DecodeMessage Messages[] = 
{
    WM_PAINT,		WndPaint,
	WM_CREATE,		WndCreate,
	WM_DESTROY,		WndDestory,
	WM_KEYUP,		WndKeyUp,
	MSG_BARCODE,	WndBarcode
};
/*******************************************************************************************************/
static LRESULT WndCreate(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}
/*******************************************************************************************************/
static LRESULT WndDestory(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	SCAN_Flush(hScanner);
	PostQuitMessage(0);
	return TRUE;
}
/*******************************************************************************************************/
static LRESULT WndBarcode(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	DWORD	dwWritten;
	WCHAR	szCRLF[] = L"\n";

	dwCount ++;
	WriteFile(hFile,SCNBUF_GETDATA(lpScanBuffer),SCNBUF_GETLEN(lpScanBuffer) * sizeof(WCHAR) ,&dwWritten,NULL);
	WriteFile(hFile,szCRLF,wcslen(szCRLF) * sizeof(WCHAR),&dwWritten,NULL);

	InvalidateRect(hWnd,NULL,FALSE);
	SCAN_ReadLabelMsg(hScanner,lpScanBuffer,hWnd,MSG_BARCODE,INFINITE,NULL);
	return TRUE;
}
/*******************************************************************************************************/
static LRESULT WndKeyUp(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_RETURN) DestroyWindow(hWnd);	
	return TRUE;
}
/*******************************************************************************************************/
static LRESULT WndPaint(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	WCHAR	szBuffer[MAX_BUFFER] = {0};
	PAINTSTRUCT ps;
	RECT	DrawRect;
	RECT	CalcRect;
	HFONT	hPrevFont;

	//Get Window Size
	GetClientRect(hWnd,&DrawRect);
	GetClientRect(hWnd,&CalcRect);

	//Paint the Window
	BeginPaint(hWnd,&ps);
	FillRect(ps.hdc,&ps.rcPaint,(HBRUSH) GetStockObject(BLACK_BRUSH));
	SetBkMode(ps.hdc,TRANSPARENT);

	//Draw Barcode Label
	SetTextColor(ps.hdc,RGB(0,255,255));
	hPrevFont = (HFONT) SelectObject(ps.hdc,hText);
	GetClientRect(hWnd,&CalcRect);
	DrawText(ps.hdc,L"Barcode",-1,&CalcRect,DT_CALCRECT);

	DrawRect.top = (DrawRect.bottom / 6);
	DrawText(ps.hdc,L"Barcode",-1,&DrawRect,DT_SINGLELINE | DT_CENTER);
	DrawRect.top += CalcRect.bottom + SCALE(10);

	//Draw Barcode
	SetTextColor(ps.hdc,RGB(255,255,255));
	SelectObject(ps.hdc,hBarcode);
	DrawText(ps.hdc,(LPWSTR) SCNBUF_GETDATA(lpScanBuffer),-1,&DrawRect,DT_SINGLELINE | DT_CENTER);

	//Draw Count Label
	SetTextColor(ps.hdc,RGB(0,255,255));
	hPrevFont = (HFONT) SelectObject(ps.hdc,hText);
	GetClientRect(hWnd,&CalcRect);
	DrawText(ps.hdc,L"Count",-1,&CalcRect,DT_CALCRECT);

	DrawRect.top = (DrawRect.bottom / 6) * 4;
	DrawText(ps.hdc,L"Count",-1,&DrawRect,DT_SINGLELINE | DT_CENTER);
	DrawRect.top += CalcRect.bottom + SCALE(10);

	//Draw Count
	wsprintf(szBuffer,L"%i",dwCount);
	SetTextColor(ps.hdc,RGB(255,255,0));
	SelectObject(ps.hdc,hCount);
	DrawText(ps.hdc,szBuffer,-1,&DrawRect,DT_SINGLELINE | DT_CENTER);

	//Clean up
	SelectObject(ps.hdc,hPrevFont);

	EndPaint(hWnd,&ps);
	return TRUE;
}
/*******************************************************************************************************/
static LRESULT CALLBACK WindowProc (HWND hWnd, UINT wMsg, WPARAM wParam,LPARAM lParam)
{
    for (DWORD dwCount = 0; dwCount < ARRAYSIZE(Messages); dwCount++)
	{
        if (wMsg == Messages[dwCount].Code) return (*Messages[dwCount].Fxn)(hWnd, wMsg, wParam, lParam);
    }
    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}
/*******************************************************************************************************/
HFONT BuildFont(int iFontSize, BOOL bBold, BOOL bItalic)
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = iFontSize;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = bBold ? 600 : 500;
	lf.lfItalic = bItalic;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = EASTEUROPE_CHARSET;
	lf.lfOutPrecision = OUT_RASTER_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = CLEARTYPE_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	wcscpy(lf.lfFaceName, L"Tahoma");
	return CreateFontIndirect(&lf);
}
/*******************************************************************************************************/
int	WINAPI	WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPWSTR lpCmdLine,int nShowCmd)
{
MSG				Msg;
WNDCLASS		WndClass;
DWORD			dwWritten;
BYTE			UnicodeHeader[] = {0xFF,0xFE };

	//Setup Variables
	HDC hDc = GetDC(NULL);
	dwWidth = GetSystemMetrics(SM_CXFULLSCREEN);
	dwHeight = GetSystemMetrics(SM_CYFULLSCREEN);
	hInstance = GetModuleHandle(NULL);
	iScale = int(GetDeviceCaps(hDc,LOGPIXELSX) / 96);
	ReleaseDC(NULL,hDc);

	//Create a File
	hFile = CreateFile(L"\\Barcodes.txt",GENERIC_WRITE,NULL,NULL,CREATE_ALWAYS,NULL,NULL);
	if (hFile == INVALID_HANDLE_VALUE) return 0;
	WriteFile(hFile,UnicodeHeader,2,&dwWritten,NULL);


	//Open Scanner
	if (SCAN_Open(L"SCN1:",&hScanner) != E_SCN_SUCCESS) return 1;

	//Enable Scanner
	SCAN_Enable(hScanner);

	//Create Fonts
	hBarcode = BuildFont(SCALE(20),FALSE,FALSE);
	hCount = BuildFont(SCALE(70),FALSE,FALSE);
	hText = BuildFont(SCALE(30),FALSE,FALSE);

	//Register a Warning window class
	ZeroMemory(&WndClass,sizeof(WNDCLASS));
	WndClass.lpfnWndProc = WindowProc;
	WndClass.style = CS_VREDRAW | CS_HREDRAW;
	WndClass.hInstance = hInstance;
	WndClass.lpszClassName = WNDCLASSNAME;
	WndClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);

	// Register window class
	if (!RegisterClass(&WndClass)) return 0;

	//Create a background window
	hWnd = CreateWindowEx(NULL,WNDCLASSNAME,L"",WS_POPUP | WS_VISIBLE,0,0, dwWidth,dwHeight,NULL,NULL,hInstance,NULL);
	if (hWnd == NULL) return 0;

	//Set to Fullscreen
	ShowWindow(hWnd,SW_SHOWMAXIMIZED);
	SHFullScreen(hWnd,SHFS_HIDETASKBAR | SHFS_HIDESTARTICON | SHFS_HIDESIPBUTTON);

	lpScanBuffer = SCAN_AllocateBuffer(TRUE,MAX_BUFFER);
	SCAN_ReadLabelMsg(hScanner,lpScanBuffer,hWnd,MSG_BARCODE,INFINITE,NULL);
	
	//Message Pump
	while (GetMessage(&Msg,NULL,0,0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	//Cleanup
	UnregisterClass(WNDCLASSNAME,hInstance);
	DeleteObject(hBarcode);
	DeleteObject(hCount);
	DeleteObject(hText);
	SCAN_Disable(hScanner);
	SCAN_Close(hScanner);
	CloseHandle(hFile);
	return 0;
}