#pragma once

#define _CRT_SECURE_NO_WARNINGS

#ifdef DIRECTX
#include <wrl.h>
#include <ppl.h>
#include <ppltasks.h>


#include <d3d11_2.h>
#include <d2d1_2.h>
#include <dwrite_2.h>
#include <DirectXMath.h>
using namespace DirectX;
#endif

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
#include <iostream>
#include <fstream>
#include <functional>
#include <exception>
#include <algorithm>
using namespace std;

#ifdef DIRECTX
using Microsoft::WRL::ComPtr;
typedef unsigned int uint;

#ifdef LINK_DIRECTX
/*d2d1.lib
d3d11.lib
dxgi.lib
ole32.lib
windowscodecs.lib
dwrite.lib
xinput.lib
*/
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "ole32.lib")
#endif


namespace pxbuf
{
	//HRexception
	// exception that resulted from a failed HRESULT, which is passed along
	struct hresult_exception : public exception
	{
	public:
		HRESULT hr;
		hresult_exception(HRESULT h, const char* m = "") : hr(h), exception(m) { }
	};

	//chr : check if FAILED(hr) and if so, throw a HRexception
	inline void chr(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw new hresult_exception(hr);
		}
	}

	class timer
	{
		double last_time;
		double curr_time;
		double _ctime;
		double _deltat;
		double invfreq;
		inline double get_time_ds()
		{
			LARGE_INTEGER ctla;
			QueryPerformanceCounter(&ctla);
			return (((double)ctla.QuadPart) * invfreq);
		}
	public:
		timer()
		{
			LARGE_INTEGER freqq;
			QueryPerformanceFrequency(&freqq);
			invfreq = 1.0 / (double)freqq.QuadPart;
			last_time = curr_time = get_time_ds();
			_deltat = 0;
			_ctime = 0;
		}

		void reset()
		{
			last_time = curr_time = get_time_ds();
			_ctime = 0;
			_deltat = 0;
		}

		void update()
		{
			curr_time = get_time_ds();

			_deltat = curr_time - last_time;
			_ctime += _deltat;

			last_time = curr_time;
		}

		inline float time() const { return (float)_ctime; }
		inline float delta_time() const { return (float)_deltat; }
	};
}
#endif