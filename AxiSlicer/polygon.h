#ifndef _POLYGON_H_
#define _POLYGON_H_

class fpxy;

#include "mesh.h"

class fpxy;

class cPolygon
{
public:
	int polygonCollisionPushVector(fpxyvect* polyA, fpxyvect* polyB, fpxy& ret);
	void projectPoly(fpxyvect* poly, fpxy& normal, float& pMin, float& pMax);
	fpxyvect* minkowskiHull(fpxyvect* a, fpxyvect* b, char* _file, int _line);
	fpxyvect* convexHull(fpxyvect* pointList, char* _file, int _line);
//	int _isRightTurn(int p, int q, int r);
	//int fpxy(fpxy& a, fpxyvect& b);
	//int projectPoly(int poly, int normal);
	int polygonCollision(fpxyvect* polyA, fpxyvect* polyB);
	//#Check if polyA is fully inside of polyB.
	int fullInside(fpxyvect* polyA, fpxyvect* polyB);
	int isLeft(fpxy& a, fpxy& b, fpxy& c);
	fpxy lineLineIntersection(fpxy& p0, fpxy& p1, fpxy& p2, fpxy& p3);
	fpxyvect* clipConvex(fpxyvect* poly0, fpxyvect* poly1);
};

extern cPolygon polygon;

#endif // _POLYGON_H_