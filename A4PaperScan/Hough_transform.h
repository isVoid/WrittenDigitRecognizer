/*
*	Hough Algorithm
*	Michael Wang SDCS SYSU 2017
*/


// #include <iostream>
// #include <vector>
// #include "CImg.h"

// using namespace cimg_library;
// using namespace std;

#pragma once

#include "headers.h"

using namespace cimg_library;
using namespace std;

struct param_space_point;
struct vec;
struct point;

class Hough_transform {

private:

	CImg<unsigned char> img;
	CImg<unsigned char> resized;
	CImg<unsigned char> cny;
	CImg<int> hough_space;
	CImg<int> threshold_hough;
	CImg<unsigned char> result;

	vector<param_space_point> v;
	vector<param_space_point> filtered;
	vector<point> intersects;

	//Hough Parameter
	int voting_thres;
	float thres_fac;
	int filter_thres;

	int thAxis;

	float resize_fac;

public:

	bool debug_disp;

	Hough_transform(CImg<unsigned char> _img, CImg<unsigned char> _cny, float resize_fac, bool verbose);

	void toHoughSpace();

	void thresholdInHough();

	/*
	*	localFiltering: A naiive implementation of local point filtering, choose the first point
	*	encountered in the local area.
	*	@param
	*	v: list of hough space points
	*	filtered: list of filtered points
	*/

	void localFiltering();

	void kMeansFiltering();

	/*
	*	computeIntersects: Computing the intersects of different lines and the points in a vector
	*	filtered: hough space points, each represents a line in image space
	*	intersects: list of intersects
	*/
	void computeIntersects();

	void extractLargestRectangle();

	void removeClosePoints();

	/*
	*	hough_line: hough line detection function
	*	Step:
	*	1. Traverse Canny image, vote on hough space
	*	2. Traverse hough space, threshold usable points
	*	3. Perform local filtering, reduce duplicates
	*	4. Compute intersects of lines
	*	5. Plot Lines and Intersect Points.
	*/

	CImg<unsigned char> process(int vt, float tf, int ft);

	CImg<unsigned char> repositionWithHarris();

	CImg<int> getHoughSpace();

	CImg<int> getThresHoughSpace();

	vector<point> getIntersects();

	void plotPoint(int x, int y, CImg<unsigned char>& img);

	void plotLine(vector<param_space_point> filtered, CImg<unsigned char>& result);

};
