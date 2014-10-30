#ifndef RAYTRACER_H
#define RAYTRACER_H

// Includes
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

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
	Vec3<T> operator - (const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
	Vec3<T> operator + (const Vec3<T> &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
	Vec3<T>& operator += (const Vec3<T> &v) { x += v.x, y += v.y, z += v.z; return *this; }
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
	T phongConstant;
	Sphere() {};

	// compute a ray-sphere intersection using the geometric solution
	bool intersect(const Vec3<T> &rayorig, const Vec3<T> &raydir, T *t) const
	{
		T B = 2 * (raydir.x*rayorig.x - raydir.x*center.x + raydir.y*rayorig.y - raydir.y*center.y - raydir.z*rayorig.z - raydir.z*center.z);
		T C = pow(rayorig, 2) - 2*rayorig.x*center.x + pow(center.x, 2) + pow(rayorig.y, 2) - 2*rayorig.y*center.y
			+ pow(center.y, 2) + pow(rayorig.z, 2) - 2*rayorig.z*center.z + pow(center.z, 2);

		// Compute discriminant D = B^2 -4C
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
	Vec3<T> p1;
	Vec3<T> p2;
	Vec3<T> p3;
	Vec3<T> diffuse, specularHighlight;
	T phongConstant; 
	Triangle() {};

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