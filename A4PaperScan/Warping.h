
/*
*	Image Warping from Quadlateral to quadlateral	
*	Implemented in two ways: 
*	Interpolate via change of base (assume quadlateral is parallelogram)
*	Interpolate via projection transformation matrix (better)
*
*	Dependency: Eigen Linear Algebra library, included in ./Include
*/


#include "headers.h"

using namespace cimg_library;
using namespace std;

class Warping {

private:

	CImg<unsigned char> img;

	vector<point> edges;

	CImg<unsigned char> warped;

	//Sort passed in edges in clock-wise order
	void sortEdges();

	//Change of base interpolation (Affine)
	void interpolate();

	//Projection Transform
	void projTransform();

public:

	bool verbose;

	//Initializer, pass in original image, a vector consisting the coordinate of the four vertices.
	Warping(CImg<unsigned char> img, vector<point> edges);

	//Process with change of base interpolation
	CImg<unsigned char> processWithInterpolate();

	//Process with projection transformation
	CImg<unsigned char> processWithProjectionTransform();


};