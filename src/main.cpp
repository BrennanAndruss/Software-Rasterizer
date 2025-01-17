/* Release code for program 1 CPE 471 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <assert.h>

#include "tiny_obj_loader.h"
#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

int g_width, g_height;

/*
   Helper function you will want all quarter
   Given a vector of shapes which has already been read from an obj file
   resize all vertices to the range [-1, 1]
*/
void resize_obj(std::vector<tinyobj::shape_t> &shapes)
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	float scaleX, scaleY, scaleZ;
	float shiftX, shiftY, shiftZ;
	float epsilon = 0.001;

	minX = minY = minZ = 1.1754E+38F;
	maxX = maxY = maxZ = -1.1754E+38F;

	// Go through all vertices to determine min and max of each dimension
	for (size_t i = 0; i < shapes.size(); i++) 
	{
		for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) 
		{
			if(shapes[i].mesh.positions[3*v+0] < minX) minX = shapes[i].mesh.positions[3*v+0];
			if(shapes[i].mesh.positions[3*v+0] > maxX) maxX = shapes[i].mesh.positions[3*v+0];

			if(shapes[i].mesh.positions[3*v+1] < minY) minY = shapes[i].mesh.positions[3*v+1];
			if(shapes[i].mesh.positions[3*v+1] > maxY) maxY = shapes[i].mesh.positions[3*v+1];

			if(shapes[i].mesh.positions[3*v+2] < minZ) minZ = shapes[i].mesh.positions[3*v+2];
			if(shapes[i].mesh.positions[3*v+2] > maxZ) maxZ = shapes[i].mesh.positions[3*v+2];
		}
	}

	// From min and max compute necessary scale and shift for each dimension
	float maxExtent, xExtent, yExtent, zExtent;
	xExtent = maxX-minX;
	yExtent = maxY-minY;
	zExtent = maxZ-minZ;
	if (xExtent >= yExtent && xExtent >= zExtent) 
	{
		maxExtent = xExtent;
	}
	if (yExtent >= xExtent && yExtent >= zExtent) 
	{
		maxExtent = yExtent;
	}
	if (zExtent >= xExtent && zExtent >= yExtent) 
	{
		maxExtent = zExtent;
	}
	scaleX = 2.0 /maxExtent;
	shiftX = minX + (xExtent/ 2.0);
	scaleY = 2.0 / maxExtent;
	shiftY = minY + (yExtent / 2.0);
	scaleZ = 2.0/ maxExtent;
	shiftZ = minZ + (zExtent)/2.0;

	// Go through all verticies shift and scale them
	for (size_t i = 0; i < shapes.size(); i++) 
	{
		for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) 
		{
			shapes[i].mesh.positions[3*v+0] = (shapes[i].mesh.positions[3*v+0] - shiftX) * scaleX;
			assert(shapes[i].mesh.positions[3*v+0] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3*v+0] <= 1.0 + epsilon);
			shapes[i].mesh.positions[3*v+1] = (shapes[i].mesh.positions[3*v+1] - shiftY) * scaleY;
			assert(shapes[i].mesh.positions[3*v+1] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3*v+1] <= 1.0 + epsilon);
			shapes[i].mesh.positions[3*v+2] = (shapes[i].mesh.positions[3*v+2] - shiftZ) * scaleZ;
			assert(shapes[i].mesh.positions[3*v+2] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3*v+2] <= 1.0 + epsilon);
		}
	}
}

struct Vec3
{
	float x;
	float y;
	float z;
};

struct Triangle
{
	Vec3 v0;
	Vec3 v1;
	Vec3 v2;
};

Triangle getTriangle(vector<float>& vertBuf, vector<unsigned int>& indBuf, int n)
{
	int size = 3; // Each vertex has 3 components
	int offset = 3; // A new triangle starts every 3 indices

	// Get triangle indices from the index buffer
	int i0 = indBuf[n * offset];
	int i1 = indBuf[n * offset + 1];
	int i2 = indBuf[n * offset + 2];

	// Get the specified vertices from the vertex buffer
	Vec3 v0 = {
		vertBuf[i0 * size],
		vertBuf[i0 * size + 1],
		vertBuf[i0 * size + 2]
	};
	Vec3 v1 = {
		vertBuf[i1 * size],
		vertBuf[i1 * size + 1],
		vertBuf[i1 * size + 2]
	};
	Vec3 v2 = {
		vertBuf[i2 * size],
		vertBuf[i2 * size + 1],
		vertBuf[i2 * size + 2]
	};

	// Return the triangle containing the three vertices
	return Triangle{ v0, v1, v2 };
}

int main(int argc, char **argv)
{
	if(argc < 6) {
		cerr << "Error: Not enough arguments provided" << endl;
		cout << "Usage: ./raster [meshfile] [imagefile] [width] [height] [colormode]" << endl;
		return 0;
	}

	// Process command line arguments
	string meshName(argv[1]);
	string imgName(argv[2]);

	g_width = atoi(argv[3]);
	g_height = atoi(argv[4]);

	int colorMode = atoi(argv[5]);

	// create an image
	auto image = make_shared<Image>(g_width, g_height);

	// Define vertex and index buffers
	vector<float> vertBuf;
	vector<unsigned int> indBuf;

	// Some obj files contain material information.
	// We'll ignore them for this assignment.
	vector<tinyobj::shape_t> shapes; // geometry
	vector<tinyobj::material_t> objMaterials; // material
	string errStr;
	
	bool rc = tinyobj::LoadObj(shapes, objMaterials, errStr, meshName.c_str());
	/* error checking on read */
	if(!rc) 
	{
		cerr << errStr << endl;
	} 
	else 
	{
 		// keep this code to resize your object to be within -1 -> 1
		resize_obj(shapes); 
		vertBuf = shapes[0].mesh.positions;
		indBuf = shapes[0].mesh.indices;
	}
	cout << "Number of vertices: " << vertBuf.size() / 3 << endl;
	cout << "Number of triangles: " << indBuf.size() / 3 << endl;

	// for debugging purposes
	bool printVBuf = false;
	if (printVBuf)
	{
		for (float p : vertBuf)
		{
			cout << p << " ";
		}
		cout << endl;
	}
	bool printIBuf = false;
	if (printIBuf)
	{
		for (unsigned int t : indBuf)
		{
			cout << t << " ";
		}
		cout << endl;
	}

	// Loop through and rasterize all the triangles
	for (int n = 0; n < indBuf.size() / 3; n++)
	{
		Triangle tri = getTriangle(vertBuf, indBuf, n);
	}
	
	// write out the image
	image->writeToFile(imgName);

	return 0;
}
