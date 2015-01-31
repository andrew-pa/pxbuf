#define LINK_DIRECTX
#include "framebuffer.h"
#include <thread>
#undef max

inline int make_color(char r, char b, char g)
{
	return ((int)r) | ((int)b<<16) | ((int)g<<8);
}

int CALLBACK WinMain(
	_In_  HINSTANCE inst,
	_In_  HINSTANCE pinst,
	_In_  LPSTR cmdLine,
	_In_  int cmds
	)
{
	XMFLOAT2 s = XMFLOAT2(640,480);
	int* vd = new int[(int)s.x*(int)s.y];
	pxbuf::framebuffer fb{ "pxbuf test", s };
	vector<int> pal;
	for (char i = 0; i < 64; ++i) pal.push_back(make_color(i+64,128,64-i));
	while (fb.refresh(vd, s.x*s.y*sizeof(int)))
	{
		float t = fb.frame_timer.time();
		memset(vd, 0, sizeof(int) * s.x * s.y);
		/*for (int _x = 0; _x < s.x; ++_x)
		{
			float x = (float)_x / s.x;
			float y = sinf(x*8.f + t)*.7f + sinf(x*16.f + t)*.3f;
			int _y = (y * (s.y*.5)) + (s.y*.5);
			if (_y < 0) _y = 0;
			if (_y > s.y) _y = s.y;
			vd[_x + _y*(int)s.x] = 128;
		}*/
		for (int _y = 0; _y < s.y; ++_y) 
		{
			for (int _x = 0; _x < s.x; ++_x)
			{
				float x0 = (_x / s.x)*2.5f - 2.f;
				float y0 = (_y / s.y)*2.f - 1.f;
				float x = 0.f, y = 0.f;
				uint itr = 0;
				while ((x*x + y*y) < 4 && itr < 512)
				{
					float xt = x*x - y*y + x0;
					y = 2 * x*y + y0;
					x = xt;
					itr++;
				}
				vd[_x + _y*(int)s.x] = pal[itr%pal.size()];
			}
		}
	}
}