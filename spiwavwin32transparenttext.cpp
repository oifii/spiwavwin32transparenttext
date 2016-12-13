/*
 * Copyright (c) 2010-2016 Stephane Poirier
 *
 * stephane.poirier@oifii.org
 *
 * Stephane Poirier
 * 3532 rue Ste-Famille, #3
 * Montreal, QC, H2X 2L1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

///////////////////////////////////////////////////////////////////////////
// spiwavwin32transparenttext.cpp : Defines the entry point for the application.
//
// 2014march19, creation to optimize display, formerly based on StretchDIBits
// 2014march19, now planed to be based on BitBlt
///////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "spiwavwin32transparenttext.h"
#include "FreeImage.h"
#include <shellapi.h> //for CommandLineToArgW()
#include <mmsystem.h> //for timeSetEvent()
#include <stdio.h> //for swprintf()
#include <assert.h>
#include "spiwavsetlib.h"

// Global Variables:

#define MAX_LOADSTRING 100
FIBITMAP* global_dib;
HBITMAP global_hddb=NULL;
HFONT global_hFont;
HWND global_hwnd=NULL;
MMRESULT global_timer=0;
int global_timernidevent=1;
#define MAX_GLOBALTEXT	4096
WCHAR global_text[MAX_GLOBALTEXT+1];
int global_x=100;
int global_y=200;
int global_xwidth=400;
int global_yheight=400;
BYTE global_alpha=200;
int global_fontheight=480;
int global_fontwidth=-1; //will be computed within WM_PAINT handler
int global_staticalignment = 0; //0 for left, 1 for center and 2 for right
int global_staticheight=-1; //will be computed within WM_SIZE handler
int global_staticwidth=-1; //will be computed within WM_SIZE handler 
//spi, begin
int global_imageheight=-1; //will be computed within WM_SIZE handler
int global_imagewidth=-1; //will be computed within WM_SIZE handler 
//spi, end
int global_titlebardisplay=0; //0 for off, 1 for on
int global_acceleratoractive=0; //0 for off, 1 for on
int global_menubardisplay=0; //0 for off, 1 for on
#define IDC_MAIN_EDIT	100
#define IDC_MAIN_STATIC	101

HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

char charbuffer[1024]={""};
char charbuffer_prev[1024]={""};

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


void CALLBACK StartGlobalProcess(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	
	WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight, global_staticalignment);

	WCHAR buffer[1024];
	int pause_ms = 1000;
	for(int ii=0; ii<30; ii++)
	{
		Sleep(pause_ms);
		swprintf(buffer, L"%i, some long chain of text, presumed to eventually go behond borders.\n", ii);
		//MainAddText(buffer);
		StatusAddText(buffer);
		swprintf(buffer, L"%i, some extra text added.\n", ii);
		//MainAddText(buffer);
		StatusAddText(buffer);
		swprintf(buffer, L"%i, more extra text added.\n", ii);
		//MainAddText(buffer);
		StatusAddText(buffer);
		swprintf(buffer, L"%i, even more extra text added.\n", ii);
		//MainAddText(buffer);
		StatusAddText(buffer);
		swprintf(buffer, L"%i, even even more extra text added.\n", ii);
		//MainAddText(buffer);
		StatusAddText(buffer);
	}
	
	PostMessage(global_hwnd, WM_DESTROY, 0, 0);
}




PCHAR*
    CommandLineToArgvA(
        PCHAR CmdLine,
        int* _argc
        )
    {
        PCHAR* argv;
        PCHAR  _argv;
        ULONG   len;
        ULONG   argc;
        CHAR   a;
        ULONG   i, j;

        BOOLEAN  in_QM;
        BOOLEAN  in_TEXT;
        BOOLEAN  in_SPACE;

        len = strlen(CmdLine);
        i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

        argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
            i + (len+2)*sizeof(CHAR));

        _argv = (PCHAR)(((PUCHAR)argv)+i);

        argc = 0;
        argv[argc] = _argv;
        in_QM = FALSE;
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        i = 0;
        j = 0;

        while( a = CmdLine[i] ) {
            if(in_QM) {
                if(a == '\"') {
                    in_QM = FALSE;
                } else {
                    _argv[j] = a;
                    j++;
                }
            } else {
                switch(a) {
                case '\"':
                    in_QM = TRUE;
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    in_SPACE = FALSE;
                    break;
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    if(in_TEXT) {
                        _argv[j] = '\0';
                        j++;
                    }
                    in_TEXT = FALSE;
                    in_SPACE = TRUE;
                    break;
                default:
                    in_TEXT = TRUE;
                    if(in_SPACE) {
                        argv[argc] = _argv+j;
                        argc++;
                    }
                    _argv[j] = a;
                    j++;
                    in_SPACE = FALSE;
                    break;
                }
            }
            i++;
        }
        _argv[j] = '\0';
        argv[argc] = NULL;

        (*_argc) = argc;
        return argv;
    }

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//LPWSTR *szArgList;
	LPSTR *szArgList;
	int nArgs;
	int i;

	//szArgList = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	szArgList = CommandLineToArgvA(GetCommandLineA(), &nArgs);
	if( NULL == szArgList )
	{
		//wprintf(L"CommandLineToArgvW failed\n");
		return FALSE;
	}
	if(nArgs>1)
	{
		global_x = atoi(szArgList[1]);
	}
	if(nArgs>2)
	{
		global_y = atoi(szArgList[2]);
	}
	if(nArgs>3)
	{
		global_xwidth = atoi(szArgList[3]);
	}
	if(nArgs>4)
	{
		global_yheight = atoi(szArgList[4]);
	}
	if(nArgs>5)
	{
		global_alpha = atoi(szArgList[5]);
	}
	if(nArgs>6)
	{
		global_titlebardisplay = atoi(szArgList[6]);
	}
	if(nArgs>7)
	{
		global_menubardisplay = atoi(szArgList[7]);
	}
	if(nArgs>8)
	{
		global_acceleratoractive = atoi(szArgList[8]);
	}
	if(nArgs>9)
	{
		global_fontheight = atoi(szArgList[9]);
	}
	LocalFree(szArgList);
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SPIWAVWIN32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	if(global_acceleratoractive)
	{
		hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPIWAVWIN32));
	}
	else
	{
		hAccelTable = NULL;
	}
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}


//******************************************************************
//******************************************************************
//******************************************************************
//******************************************************************
//******************************************************************
//******************************************************************
//******************************************************************

DWORD CalcNumColors( BITMAPINFOHEADER* pBMIH)
{
    if( !pBMIH )
	{
    	assert(FALSE);
	}    
  	if (pBMIH->biClrUsed != 0)
		return pBMIH->biClrUsed;

    switch (pBMIH->biBitCount)	// biBitCount - bits per pixel 
    {
    case 1:
            return 2;	// number of entryes in the color table
    case 4:
            return 16;
    case 8:
            return 256;
    default:			
            return 0;	// a 24 biBitCount DIB has no color table
    }
}

DWORD GetPaletteSize(BITMAPINFOHEADER* pBMIH)
{
	return CalcNumColors(pBMIH)*sizeof(RGBQUAD);

}

HBITMAP DIBtoDDBitmap(BYTE* pDIB)
{
	HBITMAP myHBITMAP;
	BITMAPINFOHEADER* pBMIH;
	BYTE* pBits;
	
	//CPalette myPalette;
	//CPalette* pOldPalette;
	HPALETTE hPalette=NULL;
	HPALETTE hOldPal=NULL;
	//CClientDC myDC(NULL);
	HDC hDC = GetDC(global_hwnd);

	// initilize the pointer to the BITMAPINFOHEADER and to the bits
	pBMIH = (BITMAPINFOHEADER*)pDIB;
	pBits = pDIB + pBMIH->biSize + GetPaletteSize(pBMIH);

	// We also need a DC to hold our bitmap.  Also make sure that 
	//  the global palette is selected into the DC.

	/*
	HDC hDC = ::GetDC(NULL);
	ASSERT(hDC);
	*/

	/*
	HPALETTE hOldPalette = ::SelectPalette(hDC, (HPALETTE)CBaseView::m_globPal.GetGlobPalette()->GetSafeHandle(), FALSE);	
	::RealizePalette(hDC);
	*/
	// Create and select a logical palette if needed
	int nColors = CalcNumColors(pBMIH);
	//if( nColors <= 256 && myDC.GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
	if( nColors <= 256 && GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE)
	{
		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
		LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];
		pLP->palVersion = 0x300;
		pLP->palNumEntries = nColors;
		for( int i=0; i < nColors; i++)
		{
			pLP->palPalEntry[i].peRed = ((BITMAPINFO*)pDIB)->bmiColors[i].rgbRed;
			pLP->palPalEntry[i].peGreen = ((BITMAPINFO*)pDIB)->bmiColors[i].rgbGreen;
			pLP->palPalEntry[i].peBlue = ((BITMAPINFO*)pDIB)->bmiColors[i].rgbBlue;
			pLP->palPalEntry[i].peFlags = 0;
		}
		//myPalette.CreatePalette(pLP);
		hPalette = CreatePalette((LPLOGPALETTE)pLP);
		delete[] pLP;
		// Select and realize the palette
		//pOldPalette = myDC.SelectPalette(&myPalette, FALSE);
		hOldPal = SelectPalette(hDC, hPalette, TRUE);
		//myDC.RealizePalette();
		RealizePalette(hDC);
	}
	else
	{
		//ASSERT(FALSE);
	}
	// initialize the bitmap handle
	myHBITMAP = CreateDIBitmap( hDC, pBMIH, CBM_INIT, pBits, (BITMAPINFO*)pDIB, DIB_RGB_COLORS );
	assert(myHBITMAP!=NULL);	

	if (hOldPal) SelectPalette(hDC, hOldPal, FALSE);
	if (hPalette) DeleteObject(hPalette);
	ReleaseDC(global_hwnd, hDC);	
	return myHBITMAP;
}

//******************************************************************
//******************************************************************
//******************************************************************
//******************************************************************
//******************************************************************
//******************************************************************
//******************************************************************


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	//wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPIWAVWIN32));
	wcex.hIcon			= (HICON)LoadImage(NULL, L"background_32x32x16.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);

	if(global_menubardisplay)
	{
		wcex.lpszMenuName = MAKEINTRESOURCE(IDC_SPIWAVWIN32); //original with menu
	}
	else
	{
		wcex.lpszMenuName = NULL; //no menu
	}
	wcex.lpszClassName	= szWindowClass;
	//wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.hIconSm		= (HICON)LoadImage(NULL, L"background_16x16x16.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	global_dib = FreeImage_Load(FIF_JPEG, "background.jpg", JPEG_DEFAULT);

	global_hddb = DIBtoDDBitmap((BYTE*)FreeImage_GetInfo(global_dib));

	//global_hFont=CreateFontW(32,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
	global_hFont=CreateFontW(global_fontheight,0,0,0,FW_NORMAL,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");

	if(global_titlebardisplay)
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, //original with WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	else
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP | WS_VISIBLE, //no WS_CAPTION etc.
			global_x, global_y, global_xwidth, global_yheight, NULL, NULL, hInstance, NULL);
	}
	if (!hWnd)
	{
		return FALSE;
	}
	global_hwnd = hWnd;

	SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	SetTimer(hWnd, global_timernidevent,1000,0);	
	return TRUE;
}



void DrawTextXOR(HDC hdc, const char* charbuffer, int charbufferlength)
{

		//spi, begin
		HDC myMemHDC = CreateCompatibleDC(hdc);
		HFONT hOldFont_memhdc=(HFONT)SelectObject(myMemHDC,global_hFont);
		SIZE mySIZE;
		GetTextExtentPoint32A(myMemHDC, charbuffer, charbufferlength, &mySIZE);

		HBITMAP myHBITMAP = CreateCompatibleBitmap(hdc, mySIZE.cx, mySIZE.cy);
		HGDIOBJ prevHBITMAP = SelectObject(myMemHDC, myHBITMAP);
		//COLORREF crOldBkColor = SetBkColor(myMemHDC, RGB(0xFF, 0xFF, 0xFF));
		COLORREF crOldBkColor = SetBkColor(myMemHDC, RGB(0x00, 0x00, 0xFF));
		//COLORREF crOldBkColor = SetBkColor(myMemHDC, RGB(0x00, 0x00, 0x00));
		COLORREF crOldTextColor_memhdc = SetTextColor(myMemHDC, RGB(0xFF, 0x00, 0x00));
		int nOldDrawingMode_memhdc = SetROP2(myMemHDC, R2_NOTXORPEN); //XOR mode, always have to erase what's drawn.
		int iOldBkMode_memhdc = SetBkMode(myMemHDC, TRANSPARENT);
		//HFONT hOldFont_memhdc=(HFONT)SelectObject(myMemHDC,global_hFont);
		//TextOutA(myMemHDC, 1, 1, "test string", 11);
		TextOutA(myMemHDC, 0, 0, charbuffer, charbufferlength);
		strcpy(charbuffer_prev, charbuffer);
		//Rectangle(myMemHDC, 0, 0, 1000, 800);
		//BitBlt(hdc, 0, 0, 1000, 800, myMemHDC, 0, 0, SRCCOPY); 
		BitBlt(hdc, 0, 0, mySIZE.cx, mySIZE.cy, myMemHDC, 0, 0, 0x00990066); //XOR mode, always have to erase what's drawn.
		SelectObject(myMemHDC, prevHBITMAP);
		DeleteDC(myMemHDC);
		DeleteObject(myHBITMAP);
		//DeleteDC(myMemHDC2);
		
		//spi, end
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HGDIOBJ hOldBrush;
	HGDIOBJ hOldPen;
	int iOldMixMode;
	COLORREF crOldBkColor;
	COLORREF crOldTextColor;
	int iOldBkMode;
	HFONT hOldFont, hFont;
	TEXTMETRIC myTEXTMETRIC;

	switch (message)
	{
	case WM_CREATE:
		{
			/*
			//HWND hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", 
			HWND hEdit = CreateWindowEx(WS_EX_TRANSPARENT, L"EDIT", L"", 
				//WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 
				//WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, //this one 
				WS_CHILD | WS_VISIBLE | ES_MULTILINE, //this one 
				//WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 
				0, 0, 100, 100, hWnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);
			if(hEdit == NULL)
				MessageBox(hWnd, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR);
			//HFONT hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			//SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
			SendMessage(hEdit, WM_SETFONT, (WPARAM)global_hFont, MAKELPARAM(FALSE, 0));
			//SendMessage(hEdit, EM_SETREADONLY, (WPARAM)TRUE, MAKELPARAM(FALSE, 0));
			*/
			/*
			HWND hStatic = CreateWindowEx(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTER,  
				0, 100, 100, 100, hWnd, (HMENU)IDC_MAIN_STATIC, GetModuleHandle(NULL), NULL);
			if(hStatic == NULL)
				MessageBox(hWnd, L"Could not create static text.", L"Error", MB_OK | MB_ICONERROR);
			SendMessage(hStatic, WM_SETFONT, (WPARAM)global_hFont, MAKELPARAM(FALSE, 0));
			*/


			//global_timer=timeSetEvent(2000,25,(LPTIMECALLBACK)&StartGlobalProcess,0,TIME_ONESHOT);
		}
		break;
	case WM_SIZE:
		{
			RECT rcClient;

			GetClientRect(hWnd, &rcClient);
			/*
			HWND hEdit = GetDlgItem(hWnd, IDC_MAIN_EDIT);
			SetWindowPos(hEdit, NULL, 0, 0, rcClient.right/2, rcClient.bottom/2, SWP_NOZORDER);
			*/
			/*
			HWND hStatic = GetDlgItem(hWnd, IDC_MAIN_STATIC);
			global_staticwidth = rcClient.right - 0;
			//global_staticheight = rcClient.bottom-(rcClient.bottom/2);
			global_staticheight = rcClient.bottom-0;
			
			global_imagewidth = rcClient.right - 0;
			global_imageheight = rcClient.bottom - 0; 
			WavSetLib_Initialize(global_hwnd, IDC_MAIN_STATIC, global_staticwidth, global_staticheight, global_fontwidth, global_fontheight, global_staticalignment);
			*/
			//spi, begin
			//did not work, the idea is to get the stretched blt into a ddbitmap
			/*
			HDC myMemHDC = CreateCompatibleDC(NULL);
			SetStretchBltMode(myMemHDC, COLORONCOLOR);
			StretchDIBits(myMemHDC, 0, 0, global_imagewidth, global_imageheight,
									0, 0, FreeImage_GetWidth(global_dib), FreeImage_GetHeight(global_dib),
									FreeImage_GetBits(global_dib), FreeImage_GetInfo(global_dib), DIB_RGB_COLORS, SRCCOPY);
			HDC myMemHDC2 = CreateCompatibleDC(myMemHDC);
			HBITMAP myhbitmap = CreateCompatibleBitmap(myMemHDC, global_imagewidth, global_imageheight);
			HGDIOBJ prevHBITMAP = SelectObject(myMemHDC2, myhbitmap);
			BitBlt(myMemHDC2, 0, 0, global_imagewidth, global_imageheight, myMemHDC, 0, 0, SRCCOPY); 
			SelectObject(myMemHDC2, prevHBITMAP);
			
			if(global_hddb!=NULL)
			{
				DeleteObject(global_hddb);
				global_hddb = myhbitmap;
			}
			DeleteDC(myMemHDC);
			DeleteDC(myMemHDC2);
			*/
			//spi, end
			//SetWindowPos(hStatic, NULL, 0, rcClient.bottom/2, global_staticwidth, global_staticheight, SWP_NOZORDER);
			//SetWindowPos(hStatic, NULL, 0, 0, global_staticwidth, global_staticheight, SWP_NOZORDER);
			hdc = GetDC(hWnd);
			HFONT hOldFont=(HFONT)SelectObject(hdc,global_hFont);
			SIZE mySIZE;
			GetTextExtentPoint32A(hdc, "88:88:88", 8, &mySIZE);
			SetWindowPos(hWnd, NULL, 0, 0, mySIZE.cx, mySIZE.cy, SWP_NOZORDER);
			RedrawWindow(hWnd, NULL, NULL, RDW_NOERASE);  //RDW_ERASE //RDW_NOERASE
			SelectObject(hdc,hOldFont);
			ReleaseDC(hWnd, hdc);
		}
		break;
	case WM_CTLCOLOREDIT:
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(0xFF, 0xFF, 0xFF));
			return (INT_PTR)::GetStockObject(NULL_PEN);
		}
		break;
	case WM_CTLCOLORSTATIC:
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(0xFF, 0xFF, 0xFF));
			return (INT_PTR)::GetStockObject(NULL_PEN);
		}
		break;
	/*
	case WM_ERASEBKGND:
		{
			return FALSE;
		}
		break;
	*/
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_TIMER:
		{
			hdc = GetDC(hWnd);
			//DrawTextXOR(hdc, "test string", 11);
			SYSTEMTIME st;
			GetSystemTime(&st);
			sprintf(charbuffer, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
			int charbufferlength = strlen(charbuffer);
			if(strcmp(charbuffer_prev, "")) DrawTextXOR(hdc, charbuffer_prev, strlen(charbuffer));
			DrawTextXOR(hdc, charbuffer, charbufferlength);

			ReleaseDC(hWnd, hdc);
		}
		break;
	case WM_PAINT:
		{
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		
		/*
		SetStretchBltMode(hdc, COLORONCOLOR);
		
		StretchDIBits(hdc, 0, 0, global_imagewidth, global_imageheight,
						0, 0, FreeImage_GetWidth(global_dib), FreeImage_GetHeight(global_dib),
						FreeImage_GetBits(global_dib), FreeImage_GetInfo(global_dib), DIB_RGB_COLORS, SRCCOPY);
		*/

		/*
		BITMAP myBITMAP;
		GetObject(global_hddb, sizeof(BITMAP), &myBITMAP);
		HDC myMemHDC = CreateCompatibleDC(hdc);
		HGDIOBJ prevHBITMAP = SelectObject(myMemHDC, global_hddb);
		BitBlt(hdc, 0, 0, myBITMAP.bmWidth, myBITMAP.bmHeight, myMemHDC, 0, 0, SRCCOPY); 
		SelectObject(myMemHDC, prevHBITMAP);
		DeleteDC(myMemHDC);
		*/

		hOldBrush = SelectObject(hdc, (HBRUSH)GetStockObject(GRAY_BRUSH));
		hOldPen = SelectObject(hdc, (HPEN)GetStockObject(WHITE_PEN));
		//iOldMixMode = SetROP2(hdc, R2_MASKPEN);
		iOldMixMode = SetROP2(hdc, R2_MERGEPEN);
		//Rectangle(hdc, 100, 100, 200, 200);

		crOldBkColor = SetBkColor(hdc, RGB(0xFF, 0x00, 0x00));
		crOldTextColor = SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
		iOldBkMode = SetBkMode(hdc, TRANSPARENT);
		//hFont=CreateFontW(70,0,0,0,FW_BOLD,0,0,0,0,0,0,2,0,L"SYSTEM_FIXED_FONT");
		//hOldFont=(HFONT)SelectObject(hdc,global_hFont);
		hOldFont=(HFONT)SelectObject(hdc,global_hFont);
		GetTextMetrics(hdc, &myTEXTMETRIC);
		global_fontwidth = myTEXTMETRIC.tmAveCharWidth;
		//TextOutW(hdc, 100, 100, L"test string", 11);

		SelectObject(hdc, hOldBrush);
		SelectObject(hdc, hOldPen);
		SetROP2(hdc, iOldMixMode);
		SetBkColor(hdc, crOldBkColor);
		SetTextColor(hdc, crOldTextColor);
		SetBkMode(hdc, iOldBkMode);
		SelectObject(hdc,hOldFont);
		//DeleteObject(hFont);
		EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		//WavSetLib_Terminate();
		//if (global_timer) timeKillEvent(global_timer);
		KillTimer(hWnd, global_timernidevent);
		FreeImage_Unload(global_dib);
		if(global_hddb!=NULL) DeleteObject(global_hddb);
		DeleteObject(global_hFont);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
