/* Release code for program 1 CPE 471 */

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <assert.h>

#include "tiny_obj_loader.h"
#include "Image.h"
#include "Vec2.h"
#include "Vec3.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

int g_width, g_height;

struct BBox
{
	int xMin;
	int xMax;
	int yMin;
	int yMax;
};

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

void getTriangleWorld(array<Vec3, 3>& tri, vector<float>& vertBuf, 
	vector<unsigned int>& indBuf, int size, int offset, int n)
{
	// Get triangle indices from the index buffer
	int i0 = indBuf[n * offset];
	int i1 = indBuf[n * offset + 1];
	int i2 = indBuf[n * offset + 2];

	// Get the specified vertices from the vertex buffer
	tri[0] = Vec3{
		vertBuf[i0 * size],
		vertBuf[i0 * size + 1],
		vertBuf[i0 * size + 2]
	};
	tri[1] = Vec3{
		vertBuf[i1 * size],
		vertBuf[i1 * size + 1],
		vertBuf[i1 * size + 2]
	};
	tri[2] = Vec3{
		vertBuf[i2 * size],
		vertBuf[i2 * size + 1],
		vertBuf[i2 * size + 2]
	};
}

void getTriangleScreen(array<Vec2, 3>& triScreen, array<Vec3, 3>& triWorld, Vec2 scale, Vec2 shift)
{
	for (int i = 0; i < 3; i++)
	{
		triScreen[i] = scale * (Vec2)triWorld[i] + shift;
	}
}

float triangleArea(Vec2 a, Vec2 b)
{
	// Compute the area of the triangle using the cross product
	return 0.5f * (a.x * b.y - b.x * a.y);
}

int main(int argc, char **argv)
{
	if(argc < 6) {
		cerr << "Error: Not enough arguments provided" << endl;
		cout << "Usage: ./raster meshfile imagefile width height colormode [r] [g] [b]" << endl;
		cout << "		[optional] r g b: base color for the object" << endl;
		return 0;
	}

	// Process command line arguments
	string meshName(argv[1]);
	string imgName(argv[2]);

	g_width = atoi(argv[3]);
	g_height = atoi(argv[4]);

	int colorMode = atoi(argv[5]);

	// Process optional command line arguments
	Vec3 baseColor(255.0f, 0.0f, 0.0f); // default to red
	if (argc == 9)
	{
		baseColor.x = atof(argv[6]);
		baseColor.y = atof(argv[7]);
		baseColor.z = atof(argv[8]);
	}

	// Create an image
	auto image = make_shared<Image>(g_width, g_height);

	// Create a z-buffer
	vector<float> zBuffer(g_width * g_height, -INFINITY);

	// Define vertex and index buffers
	vector<float> vertBuf;
	int size = 3; // Each vertex has 3 components

	vector<unsigned int> indBuf;
	int offset = 3; // A new triangle starts every 3 indices

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

	// Determine orientation of the window and define the view frustrum
	float left, right, bottom, top;
	if (g_width > g_height)
	{
		left = -g_width / g_height;
		right = g_width / g_height;
		bottom = -1.0f;
		top = 1.0f;
		
	}
	else
	{
		left = -1.0f;
		right = 1.0f;
		bottom = -g_height / g_width;
		top = g_height / g_width;
	}

	// Create the mapping from world space to screen space
	float scaleX = (g_width - 1) / (right - left);
	float scaleY = (g_height - 1) / (top - bottom);
	Vec2 scale(scaleX, scaleY);

	float shiftX = -scaleX * left;
	float shiftY = -scaleY * bottom;
	Vec2 shift(shiftX, shiftY);

	// Define small epsilon value for floating point comparisons
	float epsilon = 0.001f;

	// Loop through and rasterize all the triangles
	for (int n = 0; n < indBuf.size() / 3; n++)
	{
		// Get data from buffers to assemble triangles
		array<Vec3, 3> triWorld;
		getTriangleWorld(triWorld, vertBuf, indBuf, size, offset, n);

		// Convert vertices from world space to screen space
		array<Vec2, 3> triScreen;
		getTriangleScreen(triScreen, triWorld, scale, shift);

		// Get the bounding box of the triangle with screen coordinates
		BBox bbox = {
			min(triScreen[0].x, min(triScreen[1].x, triScreen[2].x)),	// xMin
			max(triScreen[0].x, max(triScreen[1].x, triScreen[2].x)),	// xMax
			min(triScreen[0].y, min(triScreen[1].y, triScreen[2].y)),	// yMin
			max(triScreen[0].y, max(triScreen[1].y, triScreen[2].y))	// yMax
		};

		// Compute the area of the overall triangle
		Vec2 v01 = triScreen[0] - triScreen[1];
		Vec2 v02 = triScreen[0] - triScreen[2];
		float triArea = triangleArea(v01, v02);

		// Color mode 2: Compute the center of the triangle
		Vec2 center;
		if (colorMode == 2)
		{
			center = triScreen[0] * (1.0f / 3.0f) + triScreen[1] * (1.0f / 3.0f) + triScreen[2] * (1.0f / 3.0f);
		}
		
		// Loop through all pixels within the bounding box
		for (int y = bbox.yMin; y <= bbox.yMax; y++)
		{
			for (int x = bbox.xMin; x <= bbox.xMax; x++)
			{
				// Define the point in screen space
				Vec2 p(static_cast<float>(x), static_cast<float>(y));
				// Define vectors for each subtriangle
				Vec2 v0 = triScreen[0] - p;
				Vec2 v1 = triScreen[1] - p;
				Vec2 v2 = triScreen[2] - p;

				// Compute the barycentric coordinates
				float alpha = triangleArea(v1, v2) / triArea;
				float beta = triangleArea(v2, v0) / triArea;
				float gamma = triangleArea(v0, v1) / triArea;

				// Check if alpha, beta, and gamma are inside traingles
				if (-epsilon <= alpha && alpha <= 1 + epsilon &&
					-epsilon <= beta && beta <= 1 + epsilon &&
					-epsilon <= gamma && gamma <= 1 + epsilon)
				{
					// Check the depth of the point on the triangle against the depth buffer at the same pixel
					float z = alpha * triWorld[0].z + beta * triWorld[1].z + gamma * triWorld[2].z;
					if (z > zBuffer[y * g_width + x])
					{
						// Update the depth buffer
						zBuffer[y * g_width + x] = z;

						// Color mode 2: Check if the pixel is within 4 pixels of the center
						if (colorMode == 2 && mag(p - center) > 4.0f)
						{
							// Color pixels outside the circle white
							image->setPixel(x, y, 255.0f, 255.0f, 255.0f);
						}
						else
						{
							// Adjust the base color based on depth and color the pixel
							Vec3 depthColor = baseColor * ((1.0f + z) / 2.0f);
							Vec3 pixelColor = alpha * depthColor + beta * depthColor + gamma * depthColor;
							image->setPixel(x, y, pixelColor.x, pixelColor.y, pixelColor.z);
						}
					}
				}
			}
		}
	}
	
	// Write out the image
	image->writeToFile(imgName);

	return 0;
}
