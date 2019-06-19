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


/*
 *	getHist
 *	Calculates a normalized gray image's histogram
 *
 *	@param:
 *	T hist[256]: An array to store the images histogram
 *	CImg<float> img: Image to calculate
 */
template <typename T> void getHist(T hist[256], CImg<float> img);
/*
 *	histgramEq
 *	Equalized a histogram
 *
 *	@param:
 *	T hist[256]: A(n) (image/channel)'s histogram
 *	int size: total pixel number of the image/channel, usually w x h.
 */
template <typename T> void histgramEq(T hist[256], int size);
/*
 *	histgramEq_hsi
 *	Histogram-equalize a color image using the intensity channel
 *	on hsi space.
 *
 *	@param:
 *	CImg<unsigned char> in: Image to perform operation
 *
 *	@return:
 *	CImg<unsigned char>: Histogram equalized image
 */
CImg<unsigned char> histgramEq_hsi(CImg<unsigned char> in);
