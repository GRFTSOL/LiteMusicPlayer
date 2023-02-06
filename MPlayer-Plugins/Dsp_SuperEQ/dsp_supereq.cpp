#include "../../base/base.h"
#include "dsp_supereq.h"
#include "paramlist.hpp"
#include <math.h>

extern void equ_init(int wb);
extern void equ_makeTable(double *lbc,double *rbc, paramlist *, double fs);
extern void equ_quit(void);
extern int equ_modifySamples(char *buf,int nsamples,int nch,int bps);
extern void equ_clearbuf(int,int);

double	lbands[18] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
double	rbands[18] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
paramlist	paramroot;

// The range of lslpos, rslpos is [-20, 20](dB).
int lslpos[19],rslpos[19],last_lslpos[19],last_rslpos[19];


void SetBandsFromSlpos()
{
	int i;
	float lpreamp = pow(10.0f, lslpos[0]/20.0f);
	float rpreamp = pow(10.0f, rslpos[0]/20.0f);

	for(i=0;i<18;i++)
	{
		lbands[i] = lpreamp * pow(10.0f,lslpos[i+1]/20.0f);
		rbands[i] = rpreamp * pow(10.0f,rslpos[i+1]/20.0f);
	}

	equ_makeTable(lbands,rbands,&paramroot, 0);
}

DWORD CALLBACK ThreadVisProc(LPVOID lpData)
{
	CDspSuperEQ	*pPlayer = (CDspSuperEQ *)lpData;

	pPlayer->ThreadVis();
	return 0;
}

CDspSuperEQ::CDspSuperEQ()
{
	m_nLastSRate = 0;
	m_nLastChannel = 0;
	m_nLastBps = 0;

	equ_init(14);

	for(int i = 0; i <= 18; i++)
	{
		if (lslpos[i] < -20 ) lslpos[i] = -20;
		if (lslpos[i] > 20) lslpos[i] = 20;
		if (rslpos[i] < -20 ) rslpos[i] = -20;
		if (rslpos[i] > 20) rslpos[i] = 20;
		//rslpos[i] = -20;
		//lslpos[i] = 0;
	}

#if 0
	lslpos[0] = rslpos[0] = 0;  // Preamp
	lslpos[1] = rslpos[1] = 10;
	lslpos[2] = rslpos[2] = 10;
	lslpos[3] = rslpos[3] = 10;
	lslpos[4] = rslpos[4] = 10;
	lslpos[5] = rslpos[5] = 10;
	lslpos[6] = rslpos[6] = 0;
	lslpos[7] = rslpos[7] = 0;
	lslpos[8] = rslpos[8] = 0;
	lslpos[9] = rslpos[9] = 0;
	lslpos[10] = rslpos[10] = 0;
	lslpos[11] = rslpos[11] = 0;
	lslpos[12] = rslpos[12] = 0;
	lslpos[13] = rslpos[13] = 0;
	lslpos[14] = rslpos[14] = 0;
	lslpos[15] = rslpos[15] = 0;
	lslpos[16] = rslpos[16] = 0;
	lslpos[17] = rslpos[17] = 0;
	lslpos[18] = rslpos[18] = 0;
#endif

	SetBandsFromSlpos();

	CreateWnd();

	CreateThread(NULL, 0, ThreadVisProc, this, 0, NULL);
}


CDspSuperEQ::~CDspSuperEQ()
{
	equ_quit();
}

CDspSuperEQ *CDspSuperEQ::GetInstance()
{
	static CDspSuperEQ instance;
	return &instance;
}

void CDspSuperEQ::Process(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate)
{
	if ((nChannels != 1 && nChannels != 2) || (nBps != 8 && nBps != 16 && nBps != 24)) 
		return;

	if (nSampleRate == 0)
		return;

	if (m_nLastSRate != nSampleRate)
	{
		equ_makeTable(lbands, rbands, &paramroot, (float)nSampleRate);
		m_nLastSRate = nSampleRate;
		m_nLastChannel = nChannels;
		m_nLastBps = nBps;
		equ_clearbuf(nBps, nSampleRate);
	}
	else if (m_nLastChannel != nChannels || m_nLastBps != nBps)
	{
		m_nLastChannel = nChannels;
		m_nLastBps = nBps;
		equ_clearbuf(nBps, nSampleRate);
	}

	equ_modifySamples((char *)pBuf->data(), pBuf->size() / (nChannels * nBps / 8), nChannels, nBps);
}

int RoundInt(double x) {
	return ((x) >= 0 ? ((int)((x) + 0.5)) : ((int)((x) - 0.5)));
}

VisParam *visParam1 = new VisParam;
VisParam *visParam2 = new VisParam;
Mutex	mutex;
VisParam *visParam = visParam1;
Event eventReady(false, false);

void OnSpectrumData(double *output, int count, int chanels)
{
	VisParam *freeOne;
	if (visParam == visParam1)
		freeOne = visParam2;
	else
		freeOne = visParam1;

	for (int i = 0; i < chanels; i++) {
		visParam->nChannels = chanels;
		unsigned char *specturnData = freeOne->spectrumData[i];
		float step = count / (float)VIS_N_WAVE_SAMPLE;
		float start = 0, next = step + 0.5f;
		for (int k = 0; k < VIS_N_WAVE_SAMPLE; k++) {
			double v = 0;
			for (int m = (int)start; m < (int)next; m++) {
				v += sqrt(output[i * count + m]);
			}
			v /= (int)next - (int)start;
			if (RoundInt(v) >> 3 > 256)
				specturnData[k] = 255;
			specturnData[k] = RoundInt(v) >> 3;
			start = next;
			next += step;
		}
	}

	{
		CMutexAutolock lock(mutex);
		visParam = freeOne;
	}

	eventReady.Set();
}

void CDspSuperEQ::ThreadVis()
{
	while (true) {
		VisParam *param;

		mutex.Acquire();
		param = visParam;
		mutex.Release();

		eventReady.Acquire();
		Render(param);
	}
}

static HDC memDC;		// memory device context
static HBITMAP	memBM, oldBM;  // old bitmap (from memDC)

static int		width = 488,height = 256 * 2 + 80;

static int	nMode = 4;

int CDspSuperEQ::Render(VisParam *visParam)
{
	if (nMode == 2)
		return Render2(visParam);
	else if (nMode == 3)
		return Render3(visParam);
	else if (nMode == 4)
		return Render4(visParam);
	int x, y;
	// clear background
	Rectangle(memDC,0,0,256,32);
	// draw VU meter
	for (y = 0; y < 2; y ++)
	{
		int last=visParam->waveformData[y][0];
		int total=0;
		for (x = 1; x < VIS_N_SPTR_SAMPLE; x ++)
		{
			total += abs(last - visParam->waveformData[y][x]);
			last = visParam->waveformData[y][x];
		}
		total /= 288;
		if (total > 127) total = 127;
		if (y) Rectangle(memDC,128,0,128+total,32);
		else Rectangle(memDC,128-total,0,128,32);
	}
	{ // copy doublebuffer to window
		SetLastError(0);
		HDC hdc = GetDC(m_hWnd);
		BitBlt(hdc,0,0,288,256,memDC,0,0,SRCCOPY);
		ReleaseDC(m_hWnd,hdc);
	}

	return ERR_OK;
}

int CDspSuperEQ::Render2(VisParam *visParam)
{
	int x, y;
	// clear background
	Rectangle(memDC,0,0,VIS_N_WAVE_SAMPLE,256);
	// draw analyser
	for (y = 0; y < visParam->nChannels; y ++)
	{
		for (x = 0; x < VIS_N_WAVE_SAMPLE; x ++)
		{
			MoveToEx(memDC,x,(y*256+256)>>(visParam->nChannels-1),NULL);
			LineTo(memDC,x,(y*256 + 256 - visParam->spectrumData[y][x])>>(visParam->nChannels-1));
		}
	}
	{ // copy doublebuffer to window
		SetLastError(0);
		HDC hdc = GetDC(m_hWnd);
		BitBlt(hdc,0,0,width,height,memDC,0,0,SRCCOPY);
		ReleaseDC(m_hWnd,hdc);
	}
	return 0;
}

int CDspSuperEQ::Render3(VisParam *visParam)
{
	int x, y;
	// clear background
	Rectangle(memDC,0,0,288,256);
	// draw oscilliscope
	for (y = 0; y < visParam->nChannels; y ++)
	{
		MoveToEx(memDC,0,(y*256)>>(visParam->nChannels-1),NULL);
		for (x = 0; x < 288; x ++)
		{
			LineTo(memDC,x,(y*256 + visParam->waveformData[y][x]^128)>>(visParam->nChannels-1));
		}
	}
	{ // copy doublebuffer to window
		SetLastError(0);
		HDC hdc = GetDC(m_hWnd);
		BitBlt(hdc,0,0,288,256,memDC,0,0,SRCCOPY);
		ReleaseDC(m_hWnd,hdc);
	}
	return 0;
}

int CDspSuperEQ::Render4(VisParam *visParam)
{
	int y;
	static int pos = 0;
	// clear background
	//if (pos == 0)
	//	Rectangle(memDC,0,0,width,height);
	pos++;
	if (pos > width)
		pos = 0;

	y = 0;
	for (int i = 0; i < 1; i++)
	{
		for (int y1 = 0; y1 < VIS_N_WAVE_SAMPLE; y1 ++)
		{
			byte b = visParam->spectrumData[i][y1];
			SetPixel(memDC, pos, y + y1, RGB(b, b, b));
			// MoveToEx(memDC,x,(y*256+256)>>(visParam->nChannels-1),NULL);
			// LineTo(memDC,x,(y*256 + 256 - visParam->spectrumData[y][x])>>(visParam->nChannels-1));
		}
		y += 300;
	}
	{ // copy doublebuffer to window
		SetLastError(0);
		HDC hdc = GetDC(m_hWnd);
		BitBlt(hdc,0,0,width,height,memDC,0,0,SRCCOPY);
		ReleaseDC(m_hWnd,hdc);
	}
	return 0;
}

// window procedure for our window
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:		return 0;
	case WM_ERASEBKGND: return 0;
	case WM_PAINT:
		{
			// update from doublebuffer
			PAINTSTRUCT ps;
			RECT r;
			HDC hdc = BeginPaint(hwnd,&ps);
			GetClientRect(hwnd,&r);
			BitBlt(hdc,0,0,r.right,r.bottom,memDC,0,0,SRCCOPY);
			{
				RECT x={r.left+width, r.top, r.right, r.bottom};
				RECT y={r.left, r.top+height, r.right, r.bottom};
				FillRect(hdc, &x, (HBRUSH)GetStockObject(WHITE_BRUSH));
				FillRect(hdc, &y, (HBRUSH)GetStockObject(WHITE_BRUSH));
			}
			EndPaint(hwnd,&ps);
		}
		return 0;
	case WM_DESTROY: PostQuitMessage(0); return 0;
	case WM_KEYDOWN: // pass keyboard messages to main winamp window (for processing)
	case WM_KEYUP:
		return 0;
	case WM_LBUTTONDOWN:
		nMode++;
		if (nMode > 4)
			nMode = 1;
		return 0;
//	case WM_WINDOWPOSCHANGING:
// 		{	// get config_x and config_y for configuration
// 			RECT r;
// 			GetWindowRect(myWindowState.me,&r);
// 			config_x = r.left;
// 			config_y = r.top;
// 		}
//		return 0;
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

HINSTANCE GetAppInstance();

bool CDspSuperEQ::CreateWnd()
{

	{	// Register our window class
		WNDCLASS wc;
		memset(&wc,0,sizeof(wc));
		wc.lpfnWndProc = WndProc;				// our window procedure
		wc.hInstance = GetAppInstance();	// hInstance of DLL
		wc.lpszClassName = _T("Dsp Vis");			// our window class name
	
		if (!RegisterClass(&wc)) 
		{
			MessageBox(NULL, _T("Error registering window class"), _T("Message"), MB_OK);
			return false;
		}
	}

	UINT	styles = WS_VISIBLE|WS_OVERLAPPED|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;


	m_hWnd = CreateWindowEx(
		0,	// these exstyles put a nice small frame, 
											// but also a button in the taskbar
		_T("Demo Vis"),							// our window class name
		NULL,				// no title, we're a child
		styles,				                   // do not make the window visible 
		0,0,					// screen position (read from config)
		width,height,						// width & height of window (need to adjust client area later)
		NULL,				// parent window (winamp main window)
		NULL,								// no menu
		GetAppInstance(),				// hInstance of DLL
		0); // no window creation data

	if (!m_hWnd) 
	{
		MessageBox(NULL, _T("Error creating window"), _T("Message"), MB_OK);
		return false;
	}

	SetWindowLong(m_hWnd,GWL_USERDATA,(LONG)this); // set our user data to a "this" pointer

/*	{	// adjust size of window to make the client area exactly width x height
		RECT r;
		GetClientRect(hMainWnd,&r);
		SetWindowPos(hMainWnd,0,0,0,width*2-r.right,height*2-r.bottom,SWP_NOMOVE|SWP_NOZORDER);
	}*/

	// create our doublebuffer
	memDC = CreateCompatibleDC(NULL);
	memBM = CreateCompatibleBitmap(memDC,width,height);
	oldBM = (HBITMAP)SelectObject(memDC,memBM);

  {
    RECT r={0,0,width,height};
    FillRect(memDC, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
  }

	// show the window
	ShowWindow(m_hWnd, SW_SHOWNORMAL);
	return 0;
}
