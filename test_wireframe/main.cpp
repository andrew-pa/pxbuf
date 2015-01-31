#define LINK_DIRECTX
#include "framebuffer.h"
#include <thread>
#undef max

const int width = 640, height = 480;

inline int make_color(char r, char b, char g)
{
	return ((int)r) | ((int)b << 16) | ((int)g << 8);
}

inline void putpx(int* vd, XMINT2 p, int color)
{
	vd[p.x + p.y * width] = color;
}

inline void draw_line(int* vd, XMFLOAT2 a, XMFLOAT2 b, int color)
{
	float deltax = b.x - a.x;
	float deltay = b.y - a.y;
	float err = 0;
	float dterr = fabsf(deltay / deltax);
	int y = b.y;
	for (int x = a.x; x < b.x; ++x)
	{
		putpx(vd, XMINT2(x, y), color);
		err += dterr;
		while(err >= 0.01f)
		{
			putpx(vd, XMINT2(x, y), color);
			y += signbit(deltay) ? 1 : -1;
			err = err - 1.f;
		}
	}
}

int CALLBACK WinMain(
	_In_  HINSTANCE inst,
	_In_  HINSTANCE pinst,
	_In_  LPSTR cmdLine,
	_In_  int cmds
	)
{
	XMFLOAT2 s = XMFLOAT2(width, height);
	int* vd = new int[(int)s.x*(int)s.y];
	pxbuf::framebuffer fb{ "pxbuf test", s };
	vector<int> pal;
	while (fb.refresh(vd, s.x*s.y*sizeof(int)))
	{
		float t = fb.frame_timer.time();
		memset(vd, 0, sizeof(int) * s.x * s.y);
		draw_line(vd, XMFLOAT2(64, 64), XMFLOAT2(200+sinf(t)*50.f, 200), 0xff0033ff);
	}
}