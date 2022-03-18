
// MFCDirectX11View.cpp : implementation of the CMFCDirectX11View class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "MFCDirectX11.h"
#endif

#include "MFCDirectX11Doc.h"
#include "MFCDirectX11View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace Colors
{
	XMGLOBALCONST DirectX::XMVECTORF32 Orange = { 248.0f / 255.0f,	104.0f / 255.0f,	44.0f / 255.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Green = { 145.0f / 255.0f,	195.0f / 255.0f,	0.0f / 255.0f,	1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Blue = { 0.0f / 255.0f,		180.0f / 255.0f,	241.0f / 255.0f, 1.0f };
	XMGLOBALCONST DirectX::XMVECTORF32 Yellow = { 255.0f / 255.0f,	195.0f / 255.0f,	4.0f / 255.0f,	1.0f };
};
// CMFCDirectX11View

IMPLEMENT_DYNCREATE(CMFCDirectX11View, CView)

BEGIN_MESSAGE_MAP(CMFCDirectX11View, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)

	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()
//-----------------------------------------------------------------------//
// CMFCDirectX11View construction/destruction
CMFCDirectX11View::CMFCDirectX11View() noexcept
{
	// TODO: add construction code here

}
//-----------------------------------------------------------------------//
CMFCDirectX11View::~CMFCDirectX11View()
{
	//Cleanup
	DiscardDeviceResources();

	if (m_pBackBufferRT) {
		m_pBackBufferRT->Release();
	}
	if (m_pSwapChain)	{
		m_pSwapChain->Release();
	}
	if (m_pRenderTargetView)	{
		m_pRenderTargetView->Release();
	}
	if (m_pDepthStencilBuffer) {
		m_pDepthStencilBuffer->Release();
	}
	if (m_pDepthStencilView)	{
		m_pDepthStencilView->Release();
	}
}
//-----------------------------------------------------------------------//
BOOL CMFCDirectX11View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}
//-----------------------------------------------------------------------//
// CMFCDirectX11View drawing

void CMFCDirectX11View::OnDraw(CDC* /*pDC*/)
{
	CMFCDirectX11Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}

//-----------------------------------------------------------------------//
// CMFCDirectX11View printing

BOOL CMFCDirectX11View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}
//-----------------------------------------------------------------------//
void CMFCDirectX11View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}
//-----------------------------------------------------------------------//
void CMFCDirectX11View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

//-----------------------------------------------------------------------//
// CMFCDirectX11View diagnostics

#ifdef _DEBUG
void CMFCDirectX11View::AssertValid() const
{
	CView::AssertValid();
}

void CMFCDirectX11View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCDirectX11Doc* CMFCDirectX11View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCDirectX11Doc)));
	return (CMFCDirectX11Doc*)m_pDocument;
}
#endif //_DEBUG


// CMFCDirectX11View message handlers
//-----------------------------------------------------------------------//
// CDx11ChainView message handlers
BOOL CMFCDirectX11View::OnEraseBkgnd(CDC* pDC)
{
	//Avoid flicker
	return FALSE;
}
//-----------------------------------------------------------------------//
void CMFCDirectX11View::Init(long nID, IDWriteTextFormat* pTextFormat, ID2D1Factory* pD2DFactory, DXGI_SWAP_CHAIN_DESC sd, bool bEnable4xMsaa, UINT n4xMsaaQuality, ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DImmediateContext)
{
	m_nID = nID;
	m_pTextFormat = pTextFormat;
	m_pD2DFactory = pD2DFactory;
	m_pD3DImmediateContext = pD3DImmediateContext;
	m_pD3DDevice = pD3DDevice;
	m_sd = sd;
	m_bEnable4xMsaa = bEnable4xMsaa;
	m_4xMsaaQuality = n4xMsaaQuality;
	CRect R;
	GetClientRect(&R);
	ReCreateBuffers(R.Width(), R.Height()); 
}
//-----------------------------------------------------------------------//
void CMFCDirectX11View::ReCreateBuffers(int w, int h)
{
	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = 0;
	}
	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
		m_pRenderTargetView = 0;
	}
	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
		m_pDepthStencilBuffer = 0;
	}
	if (m_pBackBufferRT)
	{
		m_pBackBufferRT->Release();
		m_pBackBufferRT = 0;
	} 
	IDXGIDevice* pDXGIDevice = 0;
	IDXGIAdapter* pDXGIAdapter = 0;
	IDXGIFactory* pDXGIFactory = 0;

	HRESULT hr = m_pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDXGIDevice);
	if (FAILED(hr))
	{
		ATLASSERT(0);
	}
	hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pDXGIAdapter);
	if (FAILED(hr))
	{
		ATLASSERT(0);
	}
	hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pDXGIFactory);
	if (FAILED(hr))
	{
		ATLASSERT(0);
	}

	m_sd.OutputWindow = GetSafeHwnd();
	m_sd.BufferDesc.Width = w;
	m_sd.BufferDesc.Height = h;
	hr = pDXGIFactory->CreateSwapChain(m_pD3DDevice, &m_sd, &m_pSwapChain);
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	m_nClientWidth = w;
	m_nClientHeight = h;
	depthStencilDesc.Width = w;
	depthStencilDesc.Height = h;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (m_bEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = m_pD3DDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer);

	hr = m_pSwapChain->ResizeBuffers(1, w, h, m_sd.BufferDesc.Format, 0);
	ID3D11Texture2D* pBackBuffer = 0;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	hr = m_pD3DDevice->CreateRenderTargetView(pBackBuffer, 0, &m_pRenderTargetView);
	if (pBackBuffer)
	{
		pBackBuffer->Release();
		pBackBuffer = 0;
	}
	hr = m_pD3DDevice->CreateDepthStencilView(m_pDepthStencilBuffer, 0, &m_pDepthStencilView);
	long nStop = 0;


	// Create the DXGI Surface Render Target.
	FLOAT dpiX;
	FLOAT dpiY;
	//m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);
	dpiX = (FLOAT)GetDpiForWindow(::GetDesktopWindow());
	dpiY = dpiX;

	D2D1_RENDER_TARGET_PROPERTIES props =
		D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
			dpiX,
			dpiY
		);

	// Create a Direct2D render target which can draw into the surface in the swap chain
	IDXGISurface* pD2DBackBuffer = 0;
	hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pD2DBackBuffer));
	hr = m_pD2DFactory->CreateDxgiSurfaceRenderTarget(
		pD2DBackBuffer,
		&props,
		&m_pBackBufferRT
	);
	if (pD2DBackBuffer)
	{
		pD2DBackBuffer->Release();
	}
	if (pDXGIDevice)
	{
		pDXGIDevice->Release();
	}
	if (pDXGIAdapter)
	{
		pDXGIAdapter->Release();
	}
	if (pDXGIFactory)
	{
		pDXGIFactory->Release();
	} 

	DiscardDeviceResources();
	CreateDeviceResources();
}
//-----------------------------------------------------------------------//
void CMFCDirectX11View::CreateDeviceResources()
{
	if (!m_pBackBufferRT) return;

	D2D1_COLOR_F clr;
	clr.a = 1.0f;
	clr.r = 255.0f / 255.0f;
	clr.g = 0.0f / 255.0f;
	clr.b = 0.0f / 255.0f;

	HRESULT hr = m_pBackBufferRT->CreateSolidColorBrush(clr, &m_pSolidBrush);

	clr.r = 0.0f / 255.0f;
	clr.g = 0.0f / 255.0f;
	clr.b = 0.0f / 255.0f;

	hr = m_pBackBufferRT->CreateSolidColorBrush(clr, &m_pTextBrush);
}
//-----------------------------------------------------------------------//
void CMFCDirectX11View::DiscardDeviceResources()
{
	if (m_pTextBrush)
	{
		m_pTextBrush->Release();
		m_pTextBrush = 0;
	}
	if (m_pSolidBrush)
	{
		m_pSolidBrush->Release();
		m_pSolidBrush = 0;
	}
}
//-----------------------------------------------------------------------//
void CMFCDirectX11View::Render()
{
	if (m_pDepthStencilView == 0) return;
	if (m_pRenderTargetView == 0) return;

	DirectX::XMVECTORF32 color = Colors::Yellow;
	if (m_nID == 0)
	{
		color = Colors::Green;
	}
	else if (m_nID == 1)
	{
		color = Colors::Orange;
	}
	else if (m_nID == 2)
	{
		color = Colors::Blue;
	}
	else if (m_nID == 3)
	{
		color = Colors::Yellow;
	}

	m_pD3DImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	D3D11_VIEWPORT Viewport;
	Viewport.TopLeftX = 0;
	Viewport.TopLeftY = 0;
	Viewport.Width = (FLOAT)m_nClientWidth;
	Viewport.Height = (FLOAT)m_nClientHeight;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;
	m_pD3DImmediateContext->RSSetViewports(1, &Viewport);

	//render background with DirectX
	m_pD3DImmediateContext->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const float*>(&color));
	m_pD3DImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//Render foreground with Direct2D
	DXGI_SWAP_CHAIN_DESC swapDesc;
	HRESULT hr = m_pSwapChain->GetDesc(&swapDesc);
	if (SUCCEEDED(hr))
	{
		// Draw a gradient background.
		if (m_pBackBufferRT)
		{
			D2D1_SIZE_F targetSize = m_pBackBufferRT->GetSize();

			m_pBackBufferRT->BeginDraw();

			m_pSolidBrush->SetTransform(
				D2D1::Matrix3x2F::Scale(targetSize)
			);

			D2D1_RECT_F rect = D2D1::RectF(
				targetSize.width / 4,
				targetSize.height / 4,
				targetSize.width * 3 / 4,
				targetSize.height * 3 / 4
			);
	 
			//use Direct2D to draw a rectangle
			//m_pBackBufferRT->FillRectangle(&rect, m_pSolidBrush);

			//use DirectWrite to display text
			CString strID;
			strID.Format(L"%d", m_nID + 1);
			HRESULT hr = m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			m_pBackBufferRT->DrawText(strID, strID.GetLength(), m_pTextFormat, rect, m_pTextBrush, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP);

			hr = m_pBackBufferRT->EndDraw();
		}
	}

	hr = m_pSwapChain->Present(0, 0);
}
//-----------------------------------------------------------------------//
void CMFCDirectX11View::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	if (m_pD3DDevice && cx > 0 && cy > 0)
	{
		ReCreateBuffers(cx, cy);
		Render();
	}
}
//-----------------------------------------------------------------------//