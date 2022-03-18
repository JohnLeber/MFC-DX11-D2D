
// MFCDirectX11View.h : interface of the CMFCDirectX11View class
//

#pragma once

class CMFCDirectX11Doc;
class CMFCDirectX11View : public CView
{
protected: // create from serialization only
	CMFCDirectX11View() noexcept;
	DECLARE_DYNCREATE(CMFCDirectX11View)
	//DXGI
	IDXGISwapChain* m_pSwapChain = 0;
	DXGI_SWAP_CHAIN_DESC m_sd;
	//IDXGIFactory* m_pDXGIFactory = 0;
	//Direct3D
	ID3D11Device* m_pD3DDevice = 0;
	long m_nID = -1;
	bool m_bEnable4xMsaa = false;
	UINT m_4xMsaaQuality = 0;
	ID3D11Texture2D* m_pDepthStencilBuffer = 0;
	ID3D11DeviceContext* m_pD3DImmediateContext = 0;
	ID3D11RenderTargetView* m_pRenderTargetView = 0;
	ID3D11DepthStencilView* m_pDepthStencilView = 0;
	ID2D1SolidColorBrush* m_pSolidBrush = 0;
	ID2D1SolidColorBrush* m_pTextBrush = 0;
	//Direct2D
	ID2D1Factory* m_pD2DFactory = 0;
	ID2D1RenderTarget* m_pBackBufferRT = 0;
	IDWriteTextFormat* m_pTextFormat = 0;
	long m_nClientWidth = 0;
	long m_nClientHeight = 0;
	void ReCreateBuffers(int w, int h);
	void DiscardDeviceResources();
	void CreateDeviceResources();
// Attributes
public:
	CMFCDirectX11Doc* GetDocument() const;

// Operations
public:
	void Init(long nID, IDWriteTextFormat* pTextFormat, ID2D1Factory* pD2DFactory, DXGI_SWAP_CHAIN_DESC sd, bool bEnable4xMsaa, UINT n4xMsaaQuality, ID3D11Device* pD3DDevice, ID3D11DeviceContext* d3dImmediateContext);

	void Render();
// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CMFCDirectX11View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // debug version in MFCDirectX11View.cpp
inline CMFCDirectX11Doc* CMFCDirectX11View::GetDocument() const
   { return reinterpret_cast<CMFCDirectX11Doc*>(m_pDocument); }
#endif

