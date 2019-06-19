#pragma once
/*
 *      Author: alexanderb
 *		Adapted into CImg dependency by Michael Wang
 */

#include "headers.h"

using namespace cimg_library;
using namespace std;

struct pointData { 
    float cornerResponse;

    point p;
};

struct by_cornerResponse { 
    bool operator()(pointData const &left, pointData const &right) { 
        return left.cornerResponse > right.cornerResponse;
    }
};

struct Derivatives {
	CImg<float> Ix;
	CImg<float> Iy;
	CImg<float> Ixy;
};

class Util {
public:
	static void DisplayImage(CImg<float>& img);
	static void DisplayMat(CImg<float>& img);
	static void DisplayPointVector(vector<point> vp);

	static CImg<float> MarkInImage(CImg<float>& img, vector<pointData> points, int radius);
};
