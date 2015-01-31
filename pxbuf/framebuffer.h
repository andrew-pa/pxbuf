#pragma once
#include "cmmn.h"

namespace pxbuf
{

	class framebuffer
	{
#ifdef DIRECTX
		ComPtr<ID3D11Device1> device;
		ComPtr<ID3D11DeviceContext1> context;
		ComPtr<IDXGISwapChain1> swapchain;
		ComPtr<ID3D11RenderTargetView> rtv;
		ComPtr<ID3D11DepthStencilView> dsv;
		ComPtr<ID3D11Texture2D> fbf;
		HWND window;
		void init_resdpr();
		XMFLOAT2 winsize;
		bool need_reinit;
#endif
	public:
		framebuffer(const string& title, XMFLOAT2 size);
		bool refresh(void* vd, size_t vdsize);
		inline void resize() 
		{
#ifdef DIRECTX
			need_reinit = true;
#endif
		}
		timer frame_timer;
		~framebuffer();
	};

}

