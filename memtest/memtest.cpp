// memtest.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"

class fpxyz {
public:
	float data[3];

	fpxyz(){}
	fpxyz(float* f) { data[0] = f[0]; }
};

fpxyz operator +(fpxyz& a, fpxyz &b)
{
	fpxyz xx;
	xx.data[0] = xx.data[1] = xx.data[2] = 0;

	return xx;
}

fpxyz func(fpxyz& a, fpxyz& b)
{
	fpxyz r = a + b;
	fpxyz k = r;
	return k;
}

int _tmain(int argc, _TCHAR* argv[])
{
	fpxyz a;
	fpxyz b;
	fpxyz c;

	c = func(a, b);

	return 0;
}

