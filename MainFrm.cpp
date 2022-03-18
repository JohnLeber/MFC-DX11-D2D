
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "framework.h"
#include "MFCDirectX11.h"
#include "MFCDirectX11View.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame() noexcept
{
	m_pView[0] = m_pView[1] = m_pView[2] = m_pView[3] = 0;
}

CMainFrame::~CMainFrame()
{
	//D3D
	if (m_pD3DImmediateContext)
	{
		m_pD3DImmediateContext->ClearState();
		m_pD3DImmediateContext->Release();
	}

	//if (m_pSwapChain)
	//{
	//	m_pSwapChain->Release();
	//}
	if (m_pD3DDevice)
	{
		m_pD3DDevice->Release();
	}
	
	//D2D
	if (m_pTextFormat)
	{
		m_pTextFormat->Release();
	}
	if (m_pWriteFactory)
	{
		m_pWriteFactory->Release();
	}
	if (m_pD2DFactory)
	{
		m_pD2DFactory->Release();
	}

	//DXGI
	//if (m_pDXGIDevice)
	//{
	//	m_pDXGIDevice->Release();
	//}
	//if (m_pDXGIAdapter)
	//{
	//	m_pDXGIAdapter->Release();
	//}
	if (m_pDXGIFactory)
	{
		m_pDXGIFactory->Release();
	} 
}
//-------------------------------------------------------------//
bool CMainFrame::Initialize()
{
	if (!InitD2D())
	{
		ATLASSERT(0);
		return false;
	}
	if (!InitD3D())
	{
		ATLASSERT(0);
		return false;
	}
	return true;
}
//-------------------------------------------------------------//
bool CMainFrame::InitD2D()
{
	//initialize Direct2D and DirectWrite
	if (!m_pD2DFactory)
	{
		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
		if (FAILED(hr))
		{
			ATLASSERT(0);
		}
	}
	if (!m_pWriteFactory)
	{
		float nTextHeight = 88;

		HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pWriteFactory), reinterpret_cast<IUnknown**>(&m_pWriteFactory));

		DWRITE_FONT_WEIGHT w = DWRITE_FONT_WEIGHT_NORMAL;
		DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
		hr = m_pWriteFactory->CreateTextFormat(L"Arial",
			NULL,
			w,
			style,
			DWRITE_FONT_STRETCH_NORMAL,
			nTextHeight,
			L"",
			&m_pTextFormat
		);

	}

	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	return SUCCEEDED(hr);
}
//-------------------------------------------------------------//
// CMainFrame message handlers
bool CMainFrame::InitD3D()
{ 
	//initialize DirectX 11
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		0,                 // default adapter
		m_D3DDriverType,
		0,                 // no software device
		createDeviceFlags,
		0, 0,              // default feature level array
		D3D11_SDK_VERSION,
		&m_pD3DDevice,
		&featureLevel,
		&m_pD3DImmediateContext);


	if (FAILED(hr))
	{
		AfxMessageBox(L"D3D11CreateDevice Failed.");
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		AfxMessageBox(L"Direct3D Feature Level 11 unsupported.");
		return false;
	}

	hr = m_pD3DDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality);
	ATLASSERT(m_4xMsaaQuality > 0);

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
	CRect quadrantRect;
	GetClientRect(&quadrantRect);
	float w = (float)quadrantRect.Width() / 2;
	float h = (float)quadrantRect.Height() / 2;

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = w;
	sd.BufferDesc.Height = h;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Use 4X MSAA? 
	if (m_Enable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = GetSafeHwnd();
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;


	/*hr = m_pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&m_pDXGIDevice);
	if (FAILED(hr))
	{
		ATLASSERT(0);
	}*/
	//hr = m_pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&m_pDXGIAdapter);
	//if (FAILED(hr))
	//{
	//	ATLASSERT(0);
	//}
	//hr = m_pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&m_pDXGIFactory);
	//if (FAILED(hr))
	//{
	//	ATLASSERT(0);
	//}
	//hr = m_pDXGIFactory->CreateSwapChain(m_pD3DDevice, &sd, &m_pSwapChain);
	//if (FAILED(hr))
	//{
	//	ATLASSERT(0);
	//}
 
	m_pView[0]->Init(0, m_pTextFormat, m_pD2DFactory, sd, m_Enable4xMsaa, m_4xMsaaQuality, m_pD3DDevice, m_pD3DImmediateContext);
	m_pView[1]->Init(1, m_pTextFormat, m_pD2DFactory, sd, m_Enable4xMsaa, m_4xMsaaQuality, m_pD3DDevice, m_pD3DImmediateContext);
	m_pView[2]->Init(2, m_pTextFormat, m_pD2DFactory, sd, m_Enable4xMsaa, m_4xMsaaQuality, m_pD3DDevice, m_pD3DImmediateContext);
	m_pView[3]->Init(3, m_pTextFormat, m_pD2DFactory, sd, m_Enable4xMsaa, m_4xMsaaQuality, m_pD3DDevice, m_pD3DImmediateContext);

	DragAcceptFiles(TRUE);
	return true;
}
//-------------------------------------------------------------//
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	return 0;
}
//-------------------------------------------------------------//
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers


//-------------------------------------------------------------//
void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);

	// When the main frame window is resized, readjust the pane sizes.
	if (m_bSplitterCreated && nType != SIZE_MINIMIZED)
	{
		// The child of the frame is the CSplitterWnd.  The client area
		// of the CSplitterWnd covers the left over client area of the frame
		// window after control bars have been placed.
		CWnd* v = GetWindow(GW_CHILD);
		CRect R;
		v->GetClientRect(&R);

		int w = R.right / 2;
		int h = R.bottom / 2;

		m_wndSplitter.SetRowInfo(0, h, 1);
		m_wndSplitter.SetRowInfo(1, h, 1);
		m_wndSplitter.SetColumnInfo(0, w, 1);
		m_wndSplitter.SetColumnInfo(1, w, 1);

		m_wndSplitter.RecalcLayout();
	}
}
//-------------------------------------------------------------//
void CMainFrame::Render()
{
	if (m_pView[0]) m_pView[0]->Render();
	if (m_pView[1]) m_pView[1]->Render();
	if (m_pView[2]) m_pView[2]->Render();
	if (m_pView[3]) m_pView[3]->Render();
}
//-------------------------------------------------------------//
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// Observe that the CSplitterWnd is a child of the frame.  The CSplitterWnd
	// is the window that basically covers the left over client area of the frame
	// window after control bars have been placed.  (The frame window includes the 
	// control bar areas.)
	m_bSplitterCreated = m_wndSplitter.CreateStatic(this, 2, 2);
	if (!m_bSplitterCreated)
		return FALSE;

	// Associate a view instance with each pane.  
	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CMFCDirectX11View), CSize(0, 0), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CMFCDirectX11View), CSize(0, 0), pContext) ||
		!m_wndSplitter.CreateView(1, 0, RUNTIME_CLASS(CMFCDirectX11View), CSize(0, 0), pContext) ||
		!m_wndSplitter.CreateView(1, 1, RUNTIME_CLASS(CMFCDirectX11View), CSize(0, 0), pContext))
		return FALSE;

	// Save pointers to the views (the returned pane is usually a CView-derived class).
	m_pView[0] = (CMFCDirectX11View*)m_wndSplitter.GetPane(0, 0);
	m_pView[1] = (CMFCDirectX11View*)m_wndSplitter.GetPane(0, 1);
	m_pView[2] = (CMFCDirectX11View*)m_wndSplitter.GetPane(1, 0);
	m_pView[3] = (CMFCDirectX11View*)m_wndSplitter.GetPane(1, 1);

	return TRUE;
}