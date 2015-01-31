#include "framebuffer.h"


namespace pxbuf 
{
	static map<HWND, framebuffer*> fbfs;

	framebuffer::framebuffer(const string& title, XMFLOAT2 size)
		: fbsize(size)
	{
		WNDCLASS wc;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = 
			[](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)->LRESULT
			{
				if(msg == WM_CLOSE) 
				{
					PostQuitMessage(0);
					return 0;
				}
				else if(msg == WM_SIZE)
				{
					auto f = fbfs.find(hwnd);
					if (f != fbfs.end())
						f->second->resize();
					return 0;
				}
				return DefWindowProc(hwnd, msg, wParam, lParam);
			};
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetModuleHandle(nullptr);
		wc.hIcon = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
		wc.lpszMenuName = 0;
		wc.lpszClassName = TEXT("DXAppWndClass");

		if (!RegisterClass(&wc))
		{
			throw exception("Failed to Register window class");
		}

		RECT r = { 0, 0, size.x, size.y };
		AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);
		int w = r.right - r.left;
		int h = r.bottom - r.top;


		HWND wnd = CreateWindow(TEXT("DXAppWndClass"), title.c_str(), WS_OVERLAPPEDWINDOW, 100,
			100, w, h, 0, 0, wc.hInstance, 0);
		if (!wnd)
		{
			throw exception("Failed to create window");
		}


		ShowWindow(wnd, 10);
		UpdateWindow(wnd);

		window = wnd;

		UINT creflg = 0;
#if defined(_DEBUG)
		creflg |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};

		D3D_FEATURE_LEVEL featureLevel;
		ComPtr<ID3D11Device> xdev;
		ComPtr<ID3D11DeviceContext> xcntx;
		chr(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creflg,
			featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
			&xdev, &featureLevel, &xcntx));
		chr(xdev.As(&device));
		chr(xcntx.As(&context));
		init_resdpr();
		fbfs[wnd] = this;
		frame_timer.reset();
	}

	void framebuffer::init_resdpr()
	{
		RECT cre;
		GetClientRect(window, &cre);

		float winwidth = (float)(cre.right - cre.left);
		float winheight = (float)(cre.bottom - cre.top);

		rtv.Reset();
		dsv.Reset();

		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

		if (swapchain != nullptr)
		{
			chr(swapchain->ResizeBuffers(2,
				static_cast<UINT>(winwidth), static_cast<UINT>(winheight), format, 0));
		}
		else
		{
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
			swapChainDesc.Width = static_cast<UINT>(winwidth);
			swapChainDesc.Height = static_cast<UINT>(winheight);
			swapChainDesc.Format = format;
			swapChainDesc.Stereo = false;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 2;
			swapChainDesc.Scaling = DXGI_SCALING_NONE;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
			swapChainDesc.Flags = 0;

			ComPtr<IDXGIDevice1> dxgid;
			chr(device.As(&dxgid));

			ComPtr<IDXGIAdapter> dxgiadp;
			chr(dxgid->GetAdapter(&dxgiadp));

			ComPtr<IDXGIFactory2> dxgif;
			chr(dxgiadp->GetParent(__uuidof(IDXGIFactory2), &dxgif));


			chr(dxgif->CreateSwapChainForHwnd(device.Get(), window, &swapChainDesc, nullptr, nullptr, 
				swapchain.GetAddressOf()));

			//chr(dxgid->SetMaximumFrameLatency(2));
		}

		ComPtr<ID3D11Texture2D> bkbf;
		chr(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), &bkbf));

		chr(device->CreateRenderTargetView(bkbf.Get(), nullptr, &rtv));

		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			static_cast<UINT>(winwidth),
			static_cast<UINT>(winheight),
			1,
			1,
			D3D11_BIND_DEPTH_STENCIL
			);

		ComPtr<ID3D11Texture2D> depthStencil;
		chr(device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil));

		CD3D11_DEPTH_STENCIL_VIEW_DESC dsvd(D3D11_DSV_DIMENSION_TEXTURE2D);
		chr(device->CreateDepthStencilView(depthStencil.Get(), &dsvd, &dsv));


		CD3D11_VIEWPORT vp(0.f, 0.f, winwidth, winheight);
		context->RSSetViewports(1, &vp);

		CD3D11_TEXTURE2D_DESC td{ format, (UINT)winwidth, (UINT)winheight };
		td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		td.Usage = D3D11_USAGE_DYNAMIC;
		td.MipLevels = 1;
		chr(device->CreateTexture2D(&td, nullptr, fbf.GetAddressOf()));

		winsize = XMFLOAT2(winwidth, winheight);
	}

	bool framebuffer::refresh(void* vd, size_t vdsize)
	{
		if(need_reinit)
		{
			init_resdpr();
			need_reinit = false;
		}
		MSG msg = { 0 };
		if (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		{
			frame_timer.update();

			D3D11_MAPPED_SUBRESOURCE mr;
			context->Map(fbf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mr);
			memcpy(mr.pData, vd, vdsize);
			context->Unmap(fbf.Get(), 0);

			ComPtr<ID3D11Texture2D> bkbf;
			chr(swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), &bkbf));

			context->CopySubresourceRegion(bkbf.Get(), 0, 0, 0, 0, fbf.Get(), 0, nullptr);
			bkbf.ReleaseAndGetAddressOf();
			
			DXGI_PRESENT_PARAMETERS p = { 0 };

			auto hr = swapchain->Present1(1, 0, &p);
			context->DiscardView(rtv.Get());
			context->DiscardView(dsv.Get());

			if (hr == DXGI_ERROR_DEVICE_REMOVED)
			{
				swapchain.Reset();

				UINT creflg = 0;
#if defined(_DEBUG)
				creflg |= D3D11_CREATE_DEVICE_DEBUG;
#endif

				D3D_FEATURE_LEVEL featureLevels[] =
				{
					D3D_FEATURE_LEVEL_11_1,
					D3D_FEATURE_LEVEL_11_0,
					D3D_FEATURE_LEVEL_10_1,
					D3D_FEATURE_LEVEL_10_0,
					D3D_FEATURE_LEVEL_9_3,
					D3D_FEATURE_LEVEL_9_2,
					D3D_FEATURE_LEVEL_9_1
				};

				D3D_FEATURE_LEVEL featureLevel;
				ComPtr<ID3D11Device> xdev;
				ComPtr<ID3D11DeviceContext> xcntx;
				chr(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creflg,
					featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
					&xdev, &featureLevel, &xcntx));
				chr(xdev.As(&device));
				chr(xcntx.As(&context));
				init_resdpr();
			}
			else
			{
				chr(hr);
			}
			ostringstream oss;
			oss << "SPF: " << frame_timer.delta_time() << " FPS: " << 1./frame_timer.delta_time() << endl;
			OutputDebugStringA(oss.str().c_str());
			double excess = fabs((1/60)-frame_timer.delta_time()); //tries for 60fps or 1/60spf 
			DWORD msst = (DWORD)(excess);
			Sleep(msst+1);
		}
		if(msg.message == WM_QUIT)
		{
			CloseWindow(window);
			return false;
		}
		return true;
	}

	framebuffer::~framebuffer()
	{
	}
}