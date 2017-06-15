#include "headers.h"
#include "Warping.h"

using namespace cimg_library;
using namespace Eigen;
using namespace std;

struct point;

/*
*	Contructor, initialize image and corners
*	@param
*	i: image
*	e: vector of corner points, size must be 4
*/
Warping::Warping(CImg<unsigned char> i, vector<point> e) {

	img.assign(i);
	edges = e;

	verbose = false;

}

/*
*	Process image with projection transform
*/
CImg<unsigned char> Warping::processWithProjectionTransform() {

	//Check validity of vertice vector
	if (edges.size() != 4) {
		printf("A quadlateral should have 4 edges. Image not processed.\n");
		return warped;
	}

	//Perform edge vector sorting
	printf("Sorting Edges into clockwise order...\n");
	sortEdges();

	//Display sorted edge vector
	if(verbose) {
		printf("Sorted edges\n");
		for (int i = 0; i < edges.size(); i++) {
			printf("%d, %d\n", edges[i].x, edges[i].y);
		}
	}

	//Perform projection Tranform
	printf("Projection Transforming...\n");
	projTransform();

	return warped;

}

/*
*	Process image with change of base interpolation
*/
CImg<unsigned char> Warping::processWithInterpolate() {

	//Check validity of vertice vector
	if (edges.size() != 4) {
		printf("A quadlateral should have 4 edges. Image not processed.\n");
		return warped;
	}

	//Perform edge vector sorting
	printf("Sorting Edges into clockwise order...\n");
	sortEdges();

	//Display sorted edge vector
	if(verbose) {
		printf("Sorted edges\n");
		for (int i = 0; i < edges.size(); i++) {
			printf("%d, %d\n", edges[i].x, edges[i].y);
		}
	}

	if(verbose) img.display();

	//Perform interpolation
	printf("Interpolating...\n");
	interpolate();

	return warped;

}

/*
*	sortEdges: sort vertex vector into clock wise order, that is:
*	top-left, top-right, bottom-right, bottom-left
*	First compute the center of weight, then the four vertices relate to the center should be: 
*	|(<,<)		(>,<)|
*	|		o 		 |
*	|(<,>)		(>,>)|
*	Assume that the origin is top left corner.
*/
void Warping::sortEdges() {

	//Deep copy the edge vector
	vector<point> e(edges);

	//Center of Weight, add all x and y, devide by 4.
	point mean;

	for (int i = 0; i < 4; i++) {
		mean.x += e[i].x;
		mean.y += e[i].y;
	}

	mean.x /= 4.0;
	mean.y /= 4.0;

    
    if (verbose){
        cout << "Geometric center" << endl;
        cout << mean.x << ' ' << mean.y << endl;
    }
	//Single loop over e, sort every vectex into position
    
	for (int i = 0; i < 4; i++) {

		if (verbose) cout << "Edge: " << e[i].x << ' ' << e[i].y << endl;

		if ( e[i].x < mean.x && e[i].y < mean.y ) 
			edges[0] = e[i];

		else if ( e[i].x > mean.x && e[i].y < mean.y )
			edges[1] = e[i];

		else if ( e[i].x > mean.x && e[i].y > mean.y )
			edges[2] = e[i];

		else if ( e[i].x < mean.x && e[i].y > mean.y )
			edges[3] = e[i];
	}

}

/*
*	interpolate: inverse map the warped image to original
*	using a set of normalized base of the original image.
*	With sorted vertices, it is easy to acquire a set of them: 
*	e0 = top_right - top_left
*	e1 = bottom_left - top_left
*
*	Since the new base in warped image is (1,0), (0,1),
*	inverse map of the pixel(x, y) to original should be
*	xe0 + ye1.
*
*	This transform is colinear. But it also assumes that 
*	the four corners in original image are vertices of a parallelogram.
*/

void Warping::interpolate() {

	//Compute the 2 bases on the original
	float e0[2] = {static_cast<float>(edges[1].x - edges[0].x), static_cast<float>(edges[1].y - edges[0].y)};
	float e1[2] = {static_cast<float>(edges[3].x - edges[0].x), static_cast<float>(edges[3].y - edges[0].y)};

	if(verbose) cout << "bases, e0: " << e0[0] << ' ' << e0[1] << " e1: " << e1[0] << ' ' << e1[1] << endl;

	//Compute length of vector
	float l0 = sqrt(pow(e0[0], 2) + pow(e0[1], 2));
	float l1 = sqrt(pow(e1[0], 2) + pow(e1[1], 2));

	if(verbose) cout << "lenghs, l0: " << l0 << " l1: " << l1 << endl;

	//Normalized base
	e0[0] /= l0; e0[1] /= l0;
	e1[0] /= l1; e1[1] /= l1;

	//Compute width and height of the A4 paper
	float w = edges[0].L2DistTo(edges[1]), h = edges[0].L2DistTo(edges[3]);

	//Flip width and height if the paper is laid horizontal
	if (w > h) {
		float t = w;
		w = h;
		h = t;

		//Flip bases
		float et[2] = {e0[0], e0[1]};
		e0[0] = e1[0]; e0[1] = e1[1];
		e1[0] = et[0]; e1[1] = et[1];
	}

	if(verbose) cout << "Width and height: " << w << ' ' << h << endl;
	if(verbose) cout << "normalized bases, e0: " << e0[0] << ' ' << e0[1] << " e1: " << e1[0] << ' ' << e1[1] << endl;

	warped.assign(int(w), int(h), 1, 3);

	//Perform inverse mapping
	cimg_forXYC(warped, x, y, c) {

		//Compute inverse mapped coordinates
		int xmap = x * e0[0] + y * e1[0] + edges[0].x;
		int ymap = x * e0[1] + y * e1[1] + edges[0].y;

		//interpolate
		if (xmap >= 0 && xmap < img._width && ymap >= 0 && ymap < img._height) {
			warped(x, y, c) = img(xmap, ymap, c);			
		}

	}

}

/*
*	projTransform: map a quadlateral to another quadlateral using projection transformation
*	Theorem: there exists a projections matrix that maps a convex quadlateral to another convex
*	quadlateral.
*
*	To compute the projection matrix, refer to: report or
*	http://stackoverflow.com/questions/3190483/transform-quadrilateral-into-a-rectangle
*
*	To perform inverse mapping, simply compute the inverse of the projection matrix, apply
*	it to every pixel of the warped image to find it's inverse map.
*/
void Warping::projTransform() {

	float w = edges[0].L2DistTo(edges[1]), h = edges[0].L2DistTo(edges[3]);

	//Half the size on the warped image.
//	w = int(w) / 2;
//	h = int(h) / 2;

	warped.assign(int(w), int(h), 1, 3);

	MatrixXf P(8, 8);
	VectorXf b(8);

	//Matrix P and vector b are to solve the parameters of the projection matrix
	P << 
	edges[0].x, edges[0].y, 1, 0, 0, 0, -(0 * edges[0].x), -(0 * edges[0].y),
	0,	0,	0,	edges[0].x,	edges[0].y, 1, -(0 * edges[0].x), -(0 * edges[0].y),
	edges[1].x, edges[1].y, 1, 0, 0, 0, -((w-1) * edges[1].x), -((w-1) * edges[1].y),
	0,	0,	0,	edges[1].x,	edges[1].y, 1, -(0 * edges[1].x), -(0 * edges[1].y),
	edges[2].x, edges[2].y, 1, 0, 0, 0, -((w-1) * edges[2].x), -((w-1) * edges[2].y),
	0,	0,	0,	edges[2].x,	edges[2].y, 1, -((h-1) * edges[2].x), -((h-1) * edges[2].y),
	edges[3].x, edges[3].y, 1, 0, 0, 0, -(0 * edges[3].x), -(0 * edges[3].y),
	0,	0,	0,	edges[3].x,	edges[3].y, 1, -((h-1) * edges[3].x), -((h-1) * edges[3].y);

	b << 0, 0, w-1, 0, w-1, h-1, 0, h-1;

	if(verbose) cout << "Heres the matrix P:\n" << P << endl;
	if(verbose) cout << "Heres the vector b:\n" << b << endl;

	//x contains the parameters of the projection matrix
	VectorXf x = P.colPivHouseholderQr().solve(b);

	if(verbose) cout << "The solution is:\n" << x << endl;

	//Projection matrix
	Matrix3f PM;
	PM << 
	x[0], x[1], x[2],
	x[3], x[4], x[5],
	x[6], x[7], 1
	;

	if(verbose) cout << "The Projection matrix\n" << PM << endl;

	//To perform inverse mapping, we take the inverse of the projection matrix
	Matrix3f PMI = PM.inverse();

	if(verbose) cout << "Inverted Projection matrix\n" << PMI << endl;

	//Perform inverse mapping
	cimg_forXY(warped, x, y) {

		//Homogenous coordinates in the warped image
		Vector3f coords(x, y, 1);
		//Inverse mapped
		Vector3f map_coords = PMI * coords;
		//Projection devide
		map_coords = map_coords / map_coords[2];

		if (map_coords[0] >= 0 && map_coords[0] < img._width && 
			map_coords[1] >= 0 && map_coords[1] < img._height)
		{
			warped(x, y, 0) = img(map_coords[0], map_coords[1], 0);
			warped(x, y, 1) = img(map_coords[0], map_coords[1], 1);
			warped(x, y, 2) = img(map_coords[0], map_coords[1], 2);
		}
	}

}
















