#define LINK_DIRECTX
#include "framebuffer.h"
#include <thread>
#undef max

const int width = 640, height = 480;


struct c { char r, g, b, a; };

inline int make_color(char r, char g, char b)
{
	//0xaabbggrr
	c v;
	v.r = r; v.g = g; v.b = b; v.a = 0xff;
	return *((int*)(&v));
}

inline void putpx(int* vd, XMINT2 p, int color)
{
	p.x = max(min(p.x + width / 2, width), 0);
	p.y = max(min(p.y + height / 2, height), 0);
	vd[p.x + p.y * width] = color;
}

inline void draw_line(int* vd, XMFLOAT2 a, XMFLOAT2 b, int color)
{
	float deltax = b.x - a.x;
	if(deltax == 0.f)
	{
		int start = min(a.y, b.y);
		int finish = max(a.y, b.y);
		for (int y = start; y < finish; ++y)
			putpx(vd, XMINT2(a.x, y), color);
		return;
	}
	float deltay = b.y - a.y;
	float err = 0;
	float dterr = fabsf(deltay / deltax);
	int y = b.y;
	int start = min(a.x, b.x);
	int finish = max(a.x, b.x);
	for (int x = start; x < finish; ++x)
	{
		putpx(vd, XMINT2(x, y), color);
		err += dterr;
		while(err >= 0.2f)
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
	vector<XMFLOAT3> pnts;
	const float ss = 64.f;
	pnts.push_back(XMFLOAT3(-ss, -ss, ss));
	pnts.push_back(XMFLOAT3( ss, -ss, ss));
	pnts.push_back(XMFLOAT3( ss,  ss, ss));
	pnts.push_back(XMFLOAT3(-ss,  ss, ss));

	pnts.push_back(XMFLOAT3(-ss, -ss, -ss));
	pnts.push_back(XMFLOAT3( ss, -ss, -ss));
	pnts.push_back(XMFLOAT3( ss,  ss, -ss));
	pnts.push_back(XMFLOAT3(-ss,  ss, -ss));

	vector<pair<uint,uint>> edges;
	edges.push_back(make_pair(1, 0));
	edges.push_back(make_pair(2, 1));
	edges.push_back(make_pair(3, 2));
	edges.push_back(make_pair(0, 3));

	edges.push_back(make_pair(5, 4));
	edges.push_back(make_pair(6, 5));
	edges.push_back(make_pair(7, 6));
	edges.push_back(make_pair(4, 7));

	edges.push_back(make_pair(4, 0));
	edges.push_back(make_pair(5, 1));
	edges.push_back(make_pair(6, 2));
	edges.push_back(make_pair(7, 3));

	vector<int> pal;
	pal.push_back(make_color(255, 0, 100));
	pal.push_back(make_color(100, 255, 0));
	pal.push_back(make_color(0, 100, 255));

	while (fb.refresh(vd, s.x*s.y*sizeof(int)))
	{
		float t = fb.frame_timer.time();
		memset(vd, 0, sizeof(int) * s.x * s.y);
		XMMATRIX trf = XMMatrixLookAtLH(XMVectorSet(sinf(t)*3.f, 3.f, -5.f, 0.f),
			XMVectorSet(0.f, 0.f, 0.f, 0.f), XMVectorSet(0.f, 1.f, 0.f, 0.f));// *XMMatrixRotationRollPitchYaw(0, 0, 0);
		vector<pair<XMFLOAT2,XMFLOAT2>> dp;
		for (int i = 0; i < edges.size(); ++i) 
		{
			XMVECTOR p0 = XMLoadFloat3(&pnts[edges[i].first]);
			XMVECTOR p1 = XMLoadFloat3(&pnts[edges[i].second]);
			p0 = ::XMVectorSetW(p0, 1.f);
			p0 = ::XMVector4Transform(p0, trf);
			p0 = ::XMVectorDivide(p0, XMVectorSwizzle<3, 3, 3, 3>(p0));
			p1 = ::XMVectorSetW(p1, 1.f);
			p1 = ::XMVector4Transform(p1, trf);
			p1 = ::XMVectorDivide(p1, XMVectorSwizzle<3, 3, 3, 3>(p1));
			XMFLOAT2 _p0;
			XMStoreFloat2(&_p0, p0);
			XMFLOAT2 _p1;
			XMStoreFloat2(&_p1, p1);
			dp.push_back(make_pair(_p0, _p1));
		}
		for (int i = 0; i < dp.size(); ++i)
		{
			draw_line(vd, dp[i].first, dp[i].second, pal[i%pal.size()]);
		}
	}
}