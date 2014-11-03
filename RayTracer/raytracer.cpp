/* 
	Simple raytracer for CS 455, Computer Graphics taught by Dr. Paris Egbert at BYU. 
	Author: Caleb Call	
*/

#include "raytracer.h"

template<typename T>
void parseFile(const std::string filename, CameraAndColorInfo &cameraAndColorInfo,
			   std::vector<Sphere<T> *> &spheres, std::vector<Triangle<T> *> &triangles)
{
	std::ifstream infile(filename);
	std::string line;
	while (std::getline(infile, line))
	{
		std::istringstream iss(line);
		std::string word;
		if (!(iss >> word)) { break; } // error

		if (word == "FieldOfView") {
			T fov;
			if (!(iss >> fov)) { break; } 
			cameraAndColorInfo.fieldOfView = fov;
		}
		else if (word == "Sphere")
		{
			Sphere<T> *sphere = new Sphere<T>();
			T x, y, z;
			if (!(iss >> word >> x >> y >> z)) { break; }
			if (word == "Center")
				sphere->center = Vec3<T>(x, y, z);
			if (!(iss >> word >> x)) { break; }
			if (word == "Radius") {
				sphere->radius = x;
				sphere->radius2 = x * x;
			}
			if (!(iss >> word)) { break; }
			if (word == "Material")
			{
				if (!(iss >> word >> x >> y >> z)) { break; }
				if (word == "Diffuse")
				{
					sphere->diffuse = Vec3<T>(x, y, z);
					if (!(iss >> word >> x >> y >> z)) { break; }
					if (word == "SpecularHighlight")
						sphere->specularHighlight = Vec3<T>(x, y, z);
					if (!(iss >> word >> x)) { break; }
					if (word == "PhongConstant")
						sphere->phongConstant = x;	
					else if (word == "Reflective")
						sphere->reflective = Vec3<T>(x, y, z);	
				}
			}

			spheres.push_back(sphere);
		}
		else if (word == "Triangle")
		{
			Triangle<T> *triangle = new Triangle<T>();
			T x, y, z;
			if (!(iss >> x >> y >> z)) { break; }
			triangle->p1 = Vec3<T>(x, y, z);
			if (!(iss >> x >> y >> z)) { break; }
			triangle->p2 = Vec3<T>(x, y, z);
			if (!(iss >> x >> y >> z)) { break; }
			triangle->p3 = Vec3<T>(x, y, z);
			if (!(iss >> word)) { break; }
			if (word == "Material")
			{
				if (!(iss >> word >> x >> y >> z)) { break; }
				if (word == "Diffuse")
				{
					triangle->diffuse = Vec3<T>(x, y, z);
					if (!(iss >> word >> x >> y >> z)) { break; }
					if (word == "SpecularHighlight")
						triangle->specularHighlight = Vec3<T>(x, y, z);
					if (!(iss >> word >> x)) { break; }
					if (word == "PhongConstant")
						triangle->phongConstant = x;	
				}
			}

			triangles.push_back(triangle);
		}
		else {
			T x, y, z;
			if (!(iss >> x >> y >> z)) { break; }
			if (word == "CameraLookAt")
				cameraAndColorInfo.cameraLookAt = Vec3<T>(x, y, z);
			else if (word == "CameraLookFrom")
				cameraAndColorInfo.cameraLookFrom = Vec3<T>(x, y, z);
			else if (word == "CameraLookUp")
				cameraAndColorInfo.cameraLookUp = Vec3<T>(x, y, z);
			else if (word == "DirectionToLight") {
				cameraAndColorInfo.directionToLight = Vec3<T>(x, y, z);
				if (!(iss >> word >> x >> y >> z)) { break; }
				if (word == "LightColor")
					cameraAndColorInfo.lightColor = Vec3<T>(x, y, z);
			}
			else if (word == "AmbientLight")
				cameraAndColorInfo.ambientLight = Vec3<T>(x, y, z);
			else if (word == "BackgroundColor")
				cameraAndColorInfo.backgroundColor = Vec3<T>(x, y, z);
		}
	}
}

template<typename T>
void computeRayDir(const float i, const float j, const int imageHeight, const int imageWidth,
					 const float viewWindowValue, Vec3<T> &rayDir)
{
	float u = (i - 0) * ( (viewWindowValue - (-viewWindowValue)) / ( imageWidth - 0) ) + (-viewWindowValue);
	float v = (j - 0) * ( (viewWindowValue - (-viewWindowValue)) / ( imageHeight - 0) ) + (-viewWindowValue);

	rayDir.x = u;
	rayDir.y = v;
	rayDir.z = 0;

	// s - eye
	rayDir = rayDir - Vec3<float>(0, 0, 1);

	// Normalize
	rayDir.normalize();
}

template<typename T>
Vec3<T> trace(const Vec3<T> &rayorig, const Vec3<T> &raydir, const std::vector<Sphere<T> *> &spheres,
			  const std::vector<Triangle<T> *> &triangles, const int &depth, const CameraAndColorInfo &info)
{
	T tnear = INFINITY;
	const Sphere<T> *sphere = NULL;
	const Triangle<T> *triangle = NULL;
	// find intersection of this ray with the spheres in the scene
	for (unsigned i = 0; i < spheres.size(); ++i) {
		T t;
		if (spheres[i]->intersect(rayorig, raydir, &t)) {
			if (t < tnear) {
				tnear = t;
				sphere = spheres[i];
			}
		}
	}
	// find intersection of this ray with the triangles in the scene
	for (unsigned i = 0; i < triangles.size(); ++i) {
		T t;
		if (triangles[i]->intersect(rayorig, raydir, &t)) {
			if (t < tnear) {
				tnear = t;
				triangle = triangles[i];
				sphere = NULL;
			}
		}
	}

	// if there's no intersection return background color
	if (!sphere && !triangle) return info.backgroundColor;
	Vec3<T> surfaceColor = 0; // color of the ray/surfaceof the object intersected by the ray
	Vec3<T> phit = rayorig + raydir * tnear; // point of intersection
	Vec3<T> nhit;
	if (sphere)
		nhit = phit - sphere->center; // normal at the intersection point
	else if (triangle)
		nhit = phit + triangle->norm; // normal at the intersection point
	 
	nhit.normalize(); // normalize normal direction
	
	T bias = 1e-4; // add some bias to the point from which we will be tracing
	// Shoot shadow rays to see if point is in shadow
	int notInShadow = 1;
	for (unsigned i = 0; i < spheres.size(); ++i) {
		T t;
		if (spheres[i]->intersect(phit + nhit * bias, info.directionToLight, &t))
			notInShadow = 0;
	}
	for (unsigned i = 0; i < triangles.size(); ++i) {
		T t;
		if (triangles[i]->intersect(phit + nhit * bias, info.directionToLight, &t))
			notInShadow = 0;
	}

	// Determine intensity information for intersecting point using illumination model
	Vec3<float> refldir = nhit * 2 * (nhit.dot(info.directionToLight)) - (info.directionToLight);
	
	if (sphere)
	{
		surfaceColor += sphere->diffuse * info.ambientLight + (info.lightColor
			* (sphere->diffuse * std::max<float>(0.0, nhit.dot(info.directionToLight))
			+ sphere->specularHighlight * pow(std::max<float>(0.0, (-raydir).dot(refldir)), sphere->phongConstant)))
			* notInShadow;
	}
	else if (triangle)
	{
		surfaceColor += triangle->diffuse * info.ambientLight + (info.lightColor
			* (triangle->diffuse * std::max<float>(0.0, nhit.dot(info.directionToLight))
			+ triangle->specularHighlight * pow(std::max<float>(0.0, (-raydir).dot(refldir)), triangle->phongConstant)))
			* notInShadow;
	}

	return surfaceColor;
}

template<typename T>
void render(const CameraAndColorInfo &info, const std::vector<Sphere<T> *> &spheres,
			const std::vector<Triangle<T> *> &triangles)
{
	const unsigned imageHeight = 513;
	const unsigned imageWidth = 514;
	Vec3<T> *image = new Vec3<T>[imageWidth * imageHeight], *pixel = image;
	float viewWindowValue = 1/(1 / tan(info.fieldOfView * (M_PI/180)));

	float height = -.5;
	// Shoot a ray through every pixel
	for (int j = 0; j < imageHeight; j++)
	{
		height += 1;
		float width = -.5;
		for (int i = 0; i < imageWidth; i++, pixel++)
		{
			width += 1;
			// Compute primary ray direction
			Vec3<T> rayDir;
			computeRayDir<float>(width, height, imageHeight, imageWidth, viewWindowValue, rayDir);

			// Shoot prim ray into scene and search for intersection
			*pixel = trace(info.cameraLookFrom, rayDir, spheres, triangles, 0, info);
		}
	}

	// Save result to a PPM image (keep these flags if you compile under Windows)
	std::ofstream ofs("./diffuse.ppm", std::ios::out | std::ios::binary);
	ofs << "P3\n" << imageWidth << " " << imageHeight << "\n255\n";
	int widthCounter = 0;
	for (unsigned i = 0; i < imageWidth * imageHeight; ++i) {
		if (widthCounter == imageWidth - 1) {
			widthCounter = 0;
			ofs << '\n';
		}
		ofs << (short)(std::min(T(1), image[i].x) * 255) << ' ' << 
		(short)(std::min(T(1), image[i].y) * 255) << ' ' <<
		(short)(std::min(T(1), image[i].z) * 255) << ' '; 
		widthCounter++;
	}
	ofs.close();
	delete [] image;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "usage: program [filename]";
		return(-1);
	}

	// Parse the file
	std::string filename = argv[1];
	CameraAndColorInfo cameraAndColorInfo;
	std::vector<Sphere<float> *> spheres;
	std::vector<Triangle<float> *> triangles;
	parseFile<float>(filename, cameraAndColorInfo, spheres, triangles);

	// Render the scene
	render<float>(cameraAndColorInfo, spheres, triangles);
}