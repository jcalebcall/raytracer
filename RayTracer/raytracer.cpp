/* 
	Simple raytracer for CS 455, Computer Graphics taught by Dr. Paris Egbert at BYU. 
	Author: Caleb Call	
*/

#include "raytracer.h"

void parseFile(std::string filename)
{
	std::ifstream infile(filename);
	std::string line;
	while (std::getline(infile, line))
	{
		std::istringstream iss(line);
		std::string word;
		if (!(iss >> word)) { break; } // error

		if (word == "FieldOfView") {
			float fov;
			if (!(iss >> fov)) { break; } 
			cameraAndColorInfo.fieldOfView = fov;
		}
		else if (word == "Sphere")
		{
			Sphere<float> sphere;
			float x, y, z;
			if (!(iss >> word >> x >> y >> z)) { break; }
			if (word == "Center")
				sphere.center = Vec3<float>(x, y, z);
			if (!(iss >> word >> x)) { break; }
			if (word == "Radius") {
				sphere.radius = x;
				sphere.radius2 = x * x;
			}
			if (!(iss >> word)) { break; }
			if (word == "Material")
			{
				if (!(iss >> word >> x >> y >> z)) { break; }
				if (word == "Diffuse")
				{
					sphere.diffuse = Vec3<float>(x, y, z);
					if (!(iss >> word >> x >> y >> z)) { break; }
					if (word == "SpecularHighlight")
						sphere.specularHighlight = Vec3<float>(x, y, z);
					if (!(iss >> word >> x)) { break; }
					if (word == "PhongConstant")
						sphere.phongConstant = x;	
					else if (word == "Reflective")
						sphere.reflective = Vec3<float>(x, y, z);	
				}
			}

			spheres.push_back(sphere);
		}
		else if (word == "Triangle")
		{
			Triangle<float> triangle;
			float x, y, z;
			if (!(iss >> x >> y >> z)) { break; }
			triangle.p1 = Vec3<float>(x, y, z);
			if (!(iss >> x >> y >> z)) { break; }
			triangle.p2 = Vec3<float>(x, y, z);
			if (!(iss >> x >> y >> z)) { break; }
			triangle.p3 = Vec3<float>(x, y, z);
			if (!(iss >> word)) { break; }
			if (word == "Material")
			{
				if (!(iss >> word >> x >> y >> z)) { break; }
				if (word == "Diffuse")
				{
					triangle.diffuse = Vec3<float>(x, y, z);
					if (!(iss >> word >> x >> y >> z)) { break; }
					if (word == "SpecularHighlight")
						triangle.specularHighlight = Vec3<float>(x, y, z);
					if (!(iss >> word >> x)) { break; }
					if (word == "PhongConstant")
						triangle.phongConstant = x;	
				}
			}

			triangles.push_back(triangle);
		}
		else {
			float x, y, z;
			if (!(iss >> x >> y >> z)) { break; }
			if (word == "CameraLookAt")
				cameraAndColorInfo.cameraLookAt = Vec3<float>(x, y, z);
			else if (word == "CameraLookFrom")
				cameraAndColorInfo.cameraLookFrom = Vec3<float>(x, y, z);
			else if (word == "CameraLookUp")
				cameraAndColorInfo.cameraLookUp = Vec3<float>(x, y, z);
			else if (word == "DirectionToLight") {
				cameraAndColorInfo.directionToLight = Vec3<float>(x, y, z);
				if (!(iss >> word >> x >> y >> z)) { break; }
				if (word == "LightColor")
					cameraAndColorInfo.lightColor = Vec3<float>(x, y, z);
			}
			else if (word == "AmbientLight")
				cameraAndColorInfo.ambientLight = Vec3<float>(x, y, z);
			else if (word == "BackgroundColor")
				cameraAndColorInfo.backgroundColor = Vec3<float>(x, y, z);
		}
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "usage: program [filename]";
		return(-1);
	}

	// Parse arguments for filename
	std::string filename = argv[1];
	parseFile(filename);

	std::string stop = "stop here";
}