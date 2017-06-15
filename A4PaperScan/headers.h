#ifndef _HEADERS_H_
#define _HEADERS_H_

#include <iostream>
#include <vector>
#include <list>
#include <cmath>

#define cimg_use_jpeg
#include "CImg.h"

//Essential to solve X11 conflict with Eigen problem.
//Ref: http://jasonjuang.blogspot.com/2015/02/how-to-solve-eigen-and-x11-success.html
#undef Success
#include <Eigen/Dense>

/*
*	A Point in Hough Param Space
*	Properties:
*	Rho, Integer, from -rho_max to rho_max 
*	Theta, Float, from 0 to 2PI
*	Vote_count, Integer, Positive
*/
struct param_space_point {
	double rho;
	double theta;
	int vote;

	param_space_point(int r = 0, double t = 0, int v = 0) {
		rho = r; theta = t; vote = v;
	}

	double L2(const param_space_point& other) {
		return sqrt(
			rho * rho + other.rho * other.rho - 2 * rho * other.rho * cos(theta - other.theta)
			);
	}
};

struct vec
{
	int x;
	int y;

	vec(int _x = 0, int _y = 0) {
		x = _x;
		y = _y;
	}

	bool isSame(vec other_vec, int tolerance) {
		if (abs(other_vec.x - x) + abs(other_vec.y - y) < tolerance) {
			return true;
		}
		else {
			return false;
		}
	}

	bool isContrary(vec other_vec, int tolerance) {
		if (abs(other_vec.x + x) + abs(other_vec.y + y) < tolerance) {
			return true;
		}
		else {
			return false;
		}
	}

	bool isOrtho(vec other_vec, int tolerance) {
		if (abs(other_vec.x * x + other_vec.y + y) < tolerance) {
			return true;
		}
		else {
			return false;
		}
	}

	float length() {
		return sqrt(x * x + y * y);
	}

};

/*
*	A Point in Image Space
*	Properties:
*	x: Integer, from 0 to image.width
*	y: Integer, from 0 to image.height
*/

struct point
{
	int x;
	int y;

	point(int _x = 0, int _y = 0) {
		x = _x;
		y = _y;
	}	

	float L2DistTo(point p2) {

		return sqrt(pow(x - p2.x, 2) + pow(y - p2.y, 2));

	}

	vec vectorTo(point p2) {
		return vec(p2.x - x, p2.y - y);
	}
};

struct homogenous_points {

	int x;
	int y;
	int z;

	homogenous_points(int _x = 0, int _y = 0, int _z = 1) {
		x = _x;
		y = _y;
		z = _z;
	}

	homogenous_points(const point& p) {

		x = p.x;
		y = p.y;
		z = 1;
		
	}

};


#endif
