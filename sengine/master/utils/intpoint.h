/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#ifndef INT_POINT_H
#define INT_POINT_H

/*
The integer point classes are used as soon as possible and represent microns in 2D or 3D space.
Integer points are used to avoid floating point rounding errors, and because ClipperLib uses them.
*/
#define INLINE static inline

//Include Clipper to get the ClipperLib::IntPoint definition, which we reuse as Point definition.
#include "../clipper/clipper.hpp"

#include <limits.h>
#include <stdint.h>
#include <math.h>

class Point3
{
public:
    int32_t x,y,z;
    Point3() {}
    Point3(const int32_t _x, const int32_t _y, const int32_t _z): x(_x), y(_y), z(_z) {}
    
    Point3 operator+(const Point3& p) const { return Point3(x+p.x, y+p.y, z+p.z); }
    Point3 operator-(const Point3& p) const { return Point3(x-p.x, y-p.y, z-p.z); }
    Point3 operator/(const int32_t i) const { return Point3(x/i, y/i, z/i); }
    
    Point3& operator += (const Point3& p) { x += p.x; y += p.y; z += p.z; return *this; }
    Point3& operator -= (const Point3& p) { x -= p.x; y -= p.y; z -= p.z; return *this; }
    
    bool operator==(const Point3& p) const { return x==p.x&&y==p.y&&z==p.z; }
    bool operator!=(const Point3& p) const { return x!=p.x||y!=p.y||z!=p.z; }
    
#ifdef max
#undef max
#endif
	int32_t max()
    {
        if (x > y && x > z) return x;
        if (y > z) return y;
        return z;
    }
    
    bool testLength(int32_t len)
    {
        if (x > len || x < -len)
            return false;
        if (y > len || y < -len)
            return false;
        if (z > len || z < -len)
            return false;
        return vSize2() <= len*len;
    }
    
    int64_t vSize2()
    {
        return int64_t(x)*int64_t(x)+int64_t(y)*int64_t(y)+int64_t(z)*int64_t(z);
    }
    
    int32_t vSize()
    {
        return sqrt((long double)vSize2());
    }
    
    Point3 cross(const Point3& p)
    {
        return Point3(
            y*p.z-z*p.y,
            z*p.x-x*p.z,
            x*p.y-y*p.x);
    }
};

/* 64bit Points are used mostly troughout the code, these are the 2D points from ClipperLib */
typedef ClipperLib::IntPoint Point;
class IntPoint {
public:
    int X, Y;
    Point p() { return Point(X, Y); }
};

/* Extra operators to make it easier to do math with the 64bit Point objects */
INLINE Point operator+(const Point& p0, const Point& p1) { return Point(p0.X+p1.X, p0.Y+p1.Y); }
INLINE Point operator-(const Point& p0, const Point& p1) { return Point(p0.X-p1.X, p0.Y-p1.Y); }
INLINE Point operator*(const Point& p0, const int32_t i) { return Point(p0.X*i, p0.Y*i); }
INLINE Point operator/(const Point& p0, const int32_t i) { return Point(p0.X/i, p0.Y/i); }

//Point& operator += (const Point& p) { x += p.x; y += p.y; return *this; }
//Point& operator -= (const Point& p) { x -= p.x; y -= p.y; return *this; }

INLINE bool operator==(const Point& p0, const Point& p1) { return p0.X==p1.X&&p0.Y==p1.Y; }
INLINE bool operator!=(const Point& p0, const Point& p1) { return p0.X!=p1.X||p0.Y!=p1.Y; }

INLINE int64_t vSize2(const Point& p0)
{
    return p0.X*p0.X+p0.Y*p0.Y;
}
INLINE float vSize2f(const Point& p0)
{
    return float(p0.X)*float(p0.X)+float(p0.Y)*float(p0.Y);
}

INLINE bool shorterThen(const Point& p0, int32_t len)
{
    if (p0.X > len || p0.X < -len)
        return false;
    if (p0.Y > len || p0.Y < -len)
        return false;
    return vSize2(p0) <= len*len;
}

INLINE int32_t vSize(const Point& p0)
{
    return sqrt((long double)vSize2(p0));
}

INLINE double vSizeMM(const Point& p0)
{
    double fx = double(p0.X) / 1000.0;
    double fy = double(p0.Y) / 1000.0;
    return sqrt(fx*fx+fy*fy);
}

INLINE Point normal(const Point& p0, int32_t len)
{
    int32_t _len = vSize(p0);
    if (_len < 1)
        return Point(len, 0);
    return p0 * len / _len;
}

INLINE Point crossZ(const Point& p0)
{
    return Point(-p0.Y, p0.X);
}
INLINE int64_t dot(const Point& p0, const Point& p1)
{
    return p0.X * p1.X + p0.Y * p1.Y;
}

#define M_E        2.71828182845904523536
#define M_LOG2E    1.44269504088896340736
#define M_LOG10E   0.434294481903251827651
#define M_LN2      0.693147180559945309417
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.785398163397448309616
#define M_1_PI     0.318309886183790671538
#define M_2_PI     0.636619772367581343076
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.707106781186547524401

class PointMatrix
{
public:
    double matrix[4];

    PointMatrix()
    {
        matrix[0] = 1;
        matrix[1] = 0;
        matrix[2] = 0;
        matrix[3] = 1;
    }
    
    PointMatrix(double rotation)
    {
        rotation = rotation / 180 * M_PI;
        matrix[0] = cos(rotation);
        matrix[1] = -sin(rotation);
        matrix[2] = -matrix[1];
        matrix[3] = matrix[0];
    }
    
    PointMatrix(const Point p)
    {
        matrix[0] = p.X;
        matrix[1] = p.Y;
        double f = sqrt((matrix[0] * matrix[0]) + (matrix[1] * matrix[1]));
        matrix[0] /= f;
        matrix[1] /= f;
        matrix[2] = -matrix[1];
        matrix[3] = matrix[0];
    }
    
    Point apply(const Point p) const
    {
        return Point(p.X * matrix[0] + p.Y * matrix[1], p.X * matrix[2] + p.Y * matrix[3]);
    }

    Point unapply(const Point p) const
    {
        return Point(p.X * matrix[0] + p.Y * matrix[2], p.X * matrix[1] + p.Y * matrix[3]);
    }
};

#endif//INT_POINT_H
