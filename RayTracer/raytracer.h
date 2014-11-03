#ifndef RAYTRACER_H
#define RAYTRACER_H

// Includes
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>

#define M_PI 3.141592653589793
#define INFINITY 1e8

// Vector class
template<typename T>
class Vec3 
{
public:
	T x, y, z;
	Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
	Vec3(T xx) : x(xx), y(xx), z(xx) {}
	Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
	Vec3& normalize()
	{
		T nor2 = length2();
		if (nor2 > 0) {
			T invNor = 1 / sqrt(nor2);
			x *= invNor, y *= invNor, z *= invNor;
		}
		return *this;
	}
	Vec3<T> operator * (const T &f) const { return Vec3<T>(x * f, y * f, z * f); }
	Vec3<T> operator * (const Vec3<T> &v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
	T dot(const Vec3<T> &v) const { return x * v.x + y * v.y + z * v.z; }
	// (a2b3 - a3b2, a3b1 - a1b3, a1b2 - a2b1)
	Vec3<T> cross(const Vec3<T> &v) const { return Vec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
	Vec3<T> operator - (const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
	Vec3<T> operator + (const Vec3<T> &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
	Vec3<T>& operator += (const Vec3<T> &v) { x += v.x, y += v.y, z += v.z; return *this; }
	Vec3<T>& operator -= (const Vec3<T> &v) { x -= v.x, y -= v.y, z -= v.z; return *this; }
	Vec3<T>& operator *= (const Vec3<T> &v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
	Vec3<T> operator - () const { return Vec3<T>(-x, -y, -z); }
	T length2() const { return x * x + y * y + z * z; }
	T length() const { return sqrt(length2()); }
	friend std::ostream & operator << (std::ostream &os, const Vec3<T> &v)
	{
		os << "[" << v.x << " " << v.y << " " << v.z << "]";
		return os;
	}
};

// Sphere class
template<typename T>
class Sphere 
{
public:
	Vec3<T> center;                         /// position of the sphere
	T radius, radius2;                      /// sphere radius and radius^2
	Vec3<T> diffuse, specularHighlight, reflective;
	T phongConstant, transparency;
	Sphere() {};

	// compute a ray-sphere intersection using the geometric solution
	bool intersect(const Vec3<T> &rayorig, const Vec3<T> &raydir, T *t) const
	{
		T B = 2 * (raydir.x * rayorig.x - raydir.x * center.x + raydir.y * rayorig.y - raydir.y * center.y + raydir.z * rayorig.z - raydir.z * center.z);
		T C = pow(rayorig.x, 2) - 2 * rayorig.x * center.x + pow(center.x, 2) + pow(rayorig.y, 2) - 2 * rayorig.y * center.y
			+ pow(center.y, 2) + pow(rayorig.z, 2) - 2 * rayorig.z * center.z + pow(center.z, 2) - radius2;

		// Compute discriminant D = B^2 - 4C
		T D = pow(B, 2) - 4 * C;

		if (D < 0)
			return false;

		// Compute smaller root first
		T t0 = (-B - sqrt(D)) / 2;
		if (t0 <= 0)
		{
			// Calculate larger root
			T t1 = (-B + sqrt(D)) / 2;
			if (t1 <= 0)
				return false;
			*t = t1;
		}
		*t = t0;
		return true;
	}

};

// Polygon class
template<typename T>
class Triangle 
{
public:
	Vec3<T> p1, p2, p3, norm;
	Vec3<T> diffuse, specularHighlight;
	T phongConstant; 
	Triangle() {};

	// compute a ray-triangle intersection using the geometric solution
	bool intersect(const Vec3<T> &rayorig, const Vec3<T> &raydir, T *t)
	{
		// First compute plane normal
		Vec3<T> v1 = p1 - p2;
		Vec3<T> v2 = p3 - p2;
		Vec3<T> pn = v2.cross(v1).normalize();
		norm = pn;
		// compute d
		T d = -pn.x * p1.x - pn.y * p1.y - pn.z * p1.z;

		// Ray/Plane intersection
		T vd = pn.dot(raydir);
		if (vd >= 0) return false;
		T v0 = -(pn.dot(rayorig) + d);
		// t = -(ax0 + by0 + cz0 + d) / (axd + byd + czd) 
		*t = v0 / vd;
		if (t < 0) return false;
		if (vd > 0)
		{
			pn = -pn;
			norm = pn;
		}
		Vec3<T> r = rayorig + raydir * *t;

		// Drop largest component
		enum coords { x, y, z };
		int max = pn.x; /* assume x is the largest */
		coords dominateCoord = x;
		if (pn.y > max) { /* if y is larger than max, assign y to max */
			max = pn.y;
			dominateCoord = y;
		} /* end if */
		if (pn.z > max) { /* if z is larger than max, assign z to max */
			max = pn.z;
			dominateCoord = z;
		} /* end if */

		Vec3<T> projP1;
		Vec3<T> projP2;
		Vec3<T> projP3;
		Vec3<T> projR;
		switch (dominateCoord) 
		{
			case x: 
				projP1 = Vec3<T>(p1.y, p1.z, 0);
				projP2 = Vec3<T>(p2.y, p2.z, 0);
				projP3 = Vec3<T>(p3.y, p3.z, 0);
				projR = Vec3<T>(r.y, r.z, 0);
				break;
			case y: 
				projP1 = Vec3<T>(p1.x, p1.z, 0);
				projP2 = Vec3<T>(p2.x, p2.z, 0);
				projP3 = Vec3<T>(p3.x, p3.z, 0);
				projR = Vec3<T>(r.x, r.z, 0);
				break;
			case z: 
				projP1 = Vec3<T>(p1.x, p1.y, 0);
				projP2 = Vec3<T>(p2.x, p2.y, 0);
				projP3 = Vec3<T>(p3.x, p3.y, 0);
				projR = Vec3<T>(r.x, r.y, 0);
				break;
		}

		projP1 -= projR;
		projP2 -= projR;
		projP3 -= projR;
		projR -= projR;

		std::vector<Vec3<T> *> verts;
		verts.push_back(&projP1);
		verts.push_back(&projP2);
		verts.push_back(&projP2);

		int numCrossings = 0;
		int signHolder = projP1.x < 0 ? -1 : 1;
		int n = verts.size();
		int j = 0;
		int nextSignHolder;
		for (int i = 0; i < n; i++)
		{
			if (i == n - 1) j = 0;
			if (verts.at(j + 1)->y < 0)
				nextSignHolder = -1;
			else
				nextSignHolder = 1;

			if ((x < 0) == (y < 0))
			{
				if (verts.at(j)->x > 0 && verts.at(j + 1)->x > 0) 
					numCrossings++;
				else if (verts.at(j)->x > 0 || verts.at(j + 1)->x > 0)
				{
					T ucross = verts.at(j)->x * -verts.at(j)->y
						* (verts.at(j + 1)->x * -verts.at(j)->x) / (verts.at(j + 1)->y * -verts.at(j)->y);
					if (ucross > 0) numCrossings++;
				}
			}
			signHolder = nextSignHolder;
			j++;
		}

		if (numCrossings % 2 != 0) // is odd
			return true;
		else
			return false;
	}
};

// Camera and color info class
struct CameraAndColorInfo 
{
	Vec3<float> cameraLookAt;
	Vec3<float> cameraLookFrom;
	Vec3<float> cameraLookUp;
	float fieldOfView;
	Vec3<float> directionToLight;
	Vec3<float> lightColor;
	Vec3<float> ambientLight;
	Vec3<float> backgroundColor;
};

#endif