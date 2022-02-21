#include "stdafx.h"
#include "mesh.h"
#include "polygon.h"

cPolygon polygon;

int _isRightTurn(fpxy& p, fpxy& q, fpxy& r)
{
	float sum1 = q.x*r.y + p.x*q.y + r.x*p.y;
	float sum2 = q.x*p.y + r.x*q.y + p.x*r.y;

	if( sum1 - sum2 < 0)
		return 1;
	else
		return 0;
}

fpxyvect* cPolygon::convexHull(fpxyvect *pointList, char* _file, int _line)
{
/* python version --------------
    unique = {} // make dictionary, as we want to sort array of points
	for p in pointList:
		unique[p[0],p[1]] = 1 // add points to dictionary

	points = unique.keys() // put the "keys" to points
	points.sort() // now sort the points
*/
	// cpp version
	fpxyvect* points = pointList->copy(__FILE__, __LINE__);
	points->sort();
	if(points->size < 1) {
		delete points;
		return new fpxyvect(0, __FILE__, __LINE__);; // numpy.zeros((0, 2), numpy.float32);
	}
	if(points->size < 2) {
		return points; // return numpy.array(points, numpy.float32);
	}

//	# Build upper half of the hull.
	fpxyvect* upper = new fpxyvect(0, _file, _line);
	upper->append(points->data[0]);
	upper->append(points->data[1]);
	for(int pidx = 2; pidx < points->size; pidx++) {
		fpxy p = points->data[pidx];
		upper->append(p);
		while(upper->size > 2 && !_isRightTurn(upper->data[upper->size-3], upper->data[upper->size-2], upper->data[upper->size-1])) {
			// python [-3:] means [-3],[-2],[-1].. [-n] means [size-n]
			upper->del(-2);
		}
	}
//	# Build lower half of the hull.
	points->size -= 1; //points = points[::-1] // copy from first to end-1
	fpxyvect* lower = new fpxyvect(0, __FILE__, __LINE__);
	lower->append(points->data[0]);
	lower->append(points->data[1]);
	for(int pidx = 2; pidx < points->size; pidx++) { //  p in points[2:]:
		fpxy p = points->data[pidx];
		lower->append(p);
		while( lower->size > 2 && ! _isRightTurn(lower->data[lower->size-3], lower->data[lower->size-2], lower->data[lower->size-1])) {
			lower->del(-2);
		}
	}
//	# Remove duplicates.
	lower->del(0);
	lower->del(1);

	upper->append(lower);
	delete lower;
	delete points;

	return upper;
	// return numpy.array(upper + lower, numpy.float32)
}

fpxyvect* cPolygon::minkowskiHull(fpxyvect* a, fpxyvect* b, char* _file, int _line)
{
// make [x][2] array and fill with 0
//	points = numpy.zeros((len(a) * len(b), 2))
	fpxyvect *points = new fpxyvect(a->size * b->size, __FILE__, __LINE__);

	for(int n = 0; n < a->size; n++) { // n in xrange(0, len(a)):
		for(int m = 0; m < b->size; m++) { // m in xrange(0, len(b)):
			points->data[n * b->size + m] = a->data[n] + b->data[m];
		}
	}

//	return convexHull(points.copy());
	fpxyvect* res = convexHull(points, _file, _line); // python requires "copy" function, as it release original points when it return from this function
	delete points;
	return res;
}

float numpy_dot(fpxy& a, fpxy& b)
{
	return a.x * b.x + a.y * b.y;
}

void cPolygon::projectPoly(fpxyvect* poly, fpxy& normal, float& pMin, float& pMax)
{
	pMin = numpy_dot(normal, poly->data[0]);
	pMax = pMin;
	for(int n = 0; n < poly->size; n++ ) {
		float p = numpy_dot(normal, poly->data[n]);
		pMin = min(pMin, p);
		pMax = max(pMax, p);
	}
	// return pMin, pMax
}

float norm(fpxy& p)
{
	float f = p.x * p.x + p.y * p.y;

	return sqrt(f);
}

#define	True	1
#define	False	0

int cPolygon::polygonCollision(fpxyvect* polyA, fpxyvect* polyB)
{
	for (int n = 0; n < polyA->size; n++) {
		fpxy p0 = polyA->data[n-1];
		fpxy p1 = polyA->data[n];
		//normal = (p1 - p0)[::-1] // reverse the order after substraction
		fpxy normal = p1 - p0;
		normal.reverse();
		normal.y = -normal.y;
		normal /= norm(normal);
		float aMin, aMax;
		float bMin, bMax;
		projectPoly(polyA, normal, aMin, aMax);
		projectPoly(polyB, normal, bMin, bMax);
		if (aMin > bMax)
			return False;
		if (bMin > aMax)
			return False;
	}

	for(int n = 0; n < polyB->size; n++) {
		fpxy p0 = polyB->data[n-1];
		fpxy p1 = polyB->data[n];
		//fpxy normal = (p1 - p0)[::-1]
		fpxy normal = p1 - p0;
		normal.reverse();
		normal.y = -normal.y;
		normal /= norm(normal);
		float aMin, aMax;
		projectPoly(polyA, normal, aMin, aMax);
		float bMin, bMax;
		projectPoly(polyB, normal, bMin, bMax);
		if (aMin > bMax)
			return False;
		if (aMax < bMin)
			return False;
	}

	return 1;
}

int cPolygon::polygonCollisionPushVector(fpxyvect* polyA, fpxyvect* polyB, fpxy& ret)
{
	float retSize = 10000000.0;

	for(int n = 0; n < polyA->size; n++) {
		fpxy p0 = polyA->data[n-1];
		fpxy p1 = polyA->data[n];
		fpxy normal = p1 - p0;
		normal.reverse();
		normal.y = -normal.y;
		normal /= norm(normal);
		float aMin, aMax;
		projectPoly(polyA, normal, aMin, aMax);
		float bMin, bMax;
		projectPoly(polyB, normal, bMin, bMax);
		if (aMin > bMax)
			return False;
		if (bMin > aMax)
			return False;
		float size = min(bMax, bMax) - max(aMin, bMin);
		if (size < retSize){
			ret = normal * (size + 0.1);
			retSize = size;
		}
	}
	
	for (int n = 0; n < polyB->size; n++) {
		fpxy p0 = polyB->data[n-1];
		fpxy p1 = polyB->data[n];
		fpxy normal = (p1 - p0);
		normal.reverse();
		normal.y = -normal.y;
		normal /= norm(normal);
		float aMin, aMax;
		projectPoly(polyA, normal, aMin, aMax);
		float bMin, bMax;
		projectPoly(polyB, normal, bMin, bMax);
		if (aMin > bMax)
			return False;
		if (aMax < bMin)
			return False;
		float size = min(bMax, bMax) - max(aMin, bMin);
		if (size < retSize) {
			ret = normal * -(size + 0.1);
			retSize = size;
		}
	}

	return True;
}

//#Check if polyA is fully inside of polyB.
int cPolygon::fullInside(fpxyvect* polyA, fpxyvect* polyB)
{
	for (int n = 0; n < polyA->size; n++) {
		fpxy p0 = polyA->data[n-1];
		fpxy p1 = polyA->data[n];
		fpxy normal = p1 - p0;
		normal.reverse();
		normal.y = -normal.y;
		normal /= norm(normal);
		float aMin, aMax;
		projectPoly(polyA, normal, aMin, aMax);
		float bMin, bMax;
		projectPoly(polyB, normal, bMin, bMax);
		if (aMax > bMax)
			return False;
		if (aMin < bMin)
			return False;
	}
	for(int n = 0; n < polyB->size; n++) {
		fpxy p0 = polyB->data[n-1];
		fpxy p1 = polyB->data[n];
		fpxy normal = (p1 - p0);
		normal.reverse();
		normal.y = -normal.y;
		normal /= norm(normal);
		float aMin, aMax ;
		projectPoly(polyA, normal, aMin, aMax);
		float bMin, bMax ;
		projectPoly(polyB, normal, bMin, bMax);
		if (aMax > bMax)
			return False;
		if (aMin < bMin)
			return False;
	}

	return True;
}

int cPolygon::isLeft(fpxy& a, fpxy& b, fpxy& c)
{
//	return ((b[0] - a[0])*(c[1] - a[1]) - (b[1] - a[1])*(c[0] - a[0])) > 0;
	return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) > 0;
}

fpxy cPolygon::lineLineIntersection(fpxy& p0, fpxy& p1, fpxy& p2, fpxy& p3)
{
	float A1 = p1.y - p0.y;
	float B1 = p0.x - p1.x;
	float C1 = A1*p0.x + B1*p0.y;

	float A2 = p3.y - p2.y;
	float B2 = p2.x - p3.x;
	float C2 = A2 * p2.x + B2 * p2.y;

	float det = A1*B2 - A2*B1;
	if (det == 0)
		return p0;
	return fpxy((B2*C1 - B1*C2)/det, (A1 * C2 - A2 * C1) / det);
}

fpxyvect* cPolygon::clipConvex(fpxyvect* poly0, fpxyvect* poly1)
{
	fpxyvect* res = poly0->copy(__FILE__, __LINE__);
	for(int p1idx = 0; p1idx < poly1->size; p1idx++) {
		fpxyvect* src = res->copy(__FILE__, __LINE__);
		res->size = 0;
		fpxy p0 = poly1->data[p1idx-1];
		fpxy p1 = poly1->data[p1idx];
		for(int n = 0; n < src->size; n++) {
			fpxy p = src->data[n];
			if (! isLeft(p0, p1, p)) {
				if (isLeft(p0, p1, src->data[n-1])) {
					res->append(lineLineIntersection(p0, p1, src->data[n-1], p));
				}
				res->append(p);
			} else if( ! isLeft(p0, p1, src->data[n-1])) {
				res->append(lineLineIntersection(p0, p1, src->data[n-1], p));
			}
		}
		delete src;
	}

	return res;
}