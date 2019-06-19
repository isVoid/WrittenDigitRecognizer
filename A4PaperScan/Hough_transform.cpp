/*
*	Hough Algorithm
*	Michael Wang SDCS SYSU 2017
*/


#include "headers.h"
#include "Hough_transform.h"
#include "harris.h"
#include <float.h>

using namespace cimg_library;
using namespace std;

int min4(double a, double b, double c, double d) {

	double m = a;
	int mi = 0;

	if (b < m) {
		m = b;
		mi = 1;
	}

	if (c < m) {
		m = c;
		mi = 2;
	}

	if (d < m) {
		m = d;
		mi = 3;
	}

	return mi;

}

Hough_transform::Hough_transform(CImg<unsigned char> _img, CImg<unsigned char> _cny, float rf = 8., bool verbose = false) {

	//Hough Parameter
	voting_thres = 64;
	thres_fac = 0.4;
	filter_thres = 200;

	thAxis = 180;

	resize_fac = rf;

	debug_disp = verbose;

	img = _img;
	resized.assign(img);
	resized.resize(resized._width / resize_fac, resized._height / resize_fac);
	cny = _cny;
	result.assign(img);

}

/*
*	process: hough line detection process
*	Step:
*	1. Traverse Canny image, vote on hough space
*	2. Traverse hough space, threshold usable points
*	3. Perform local filtering, reduce duplicates
*	4. Compute intersects of lines
*	5. Plot Lines and Intersect Points.
*/
 
CImg<unsigned char> Hough_transform::process(int vt, float tf, int ft) {

	voting_thres = vt;
	thres_fac = tf;
	filter_thres = ft;

	toHoughSpace();

	if (debug_disp)
		hough_space.display();

    thresholdInHough();

    if (debug_disp)
        threshold_hough.display();
    
    // printf("performing local filtering...\n" );
    // localFiltering();

    kMeansFiltering();

    if(debug_disp) 
        for (int i = 0; i < filtered.size(); i++) {
            printf("rho: %f, theta: %d\n", filtered[i].rho, int(filtered[i].theta * 57.296));
        }

    computeIntersects();

	if (intersects.size() > 4) {
		printf("intersects more than 4, extracting largest rectangle...\n");
		extractLargestRectangle();
	}
    
	for (int i = 0; i < intersects.size(); i++) {
		int x = intersects[i].x, y = intersects[i].y;
		plotPoint(x, y, result);
	}
    if (debug_disp)
        result.display();
    
	return result;

}

void Hough_transform::toHoughSpace() {
    
	int w = cny._width, h = cny._height;
	int pmax = max(w, h);
	pmax = ceil(1.414 * pmax);

	hough_space.assign(thAxis, 2 * pmax, 1, 1, 0);

	cimg_forXY(cny, x, y) {

		int v = cny(x, y);

		if (v > voting_thres) {
			// printf("x, y: %d %d\n", x, y);
			for (int th = 0; th < thAxis; th++) {

				float angle = (cimg::PI) * th / thAxis;
				float xcos = x * cos(angle);
				float ysin = y * sin(angle);
				float rho = xcos + ysin;
				
				float rho_s = rho + pmax;

				hough_space(th, int(rho_s))++;

			}

		}
	}

}

CImg<int> Hough_transform::getHoughSpace() {
	return hough_space;
}


void Hough_transform::thresholdInHough() {

	int maxL = hough_space.max();
	int w = cny._width, h = cny._height;
	int pmax = max(w, h);
	pmax = ceil(1.414 * pmax);

	int thresh = maxL * thres_fac;

	threshold_hough.assign(hough_space);

	cimg_forXY(threshold_hough, th, rho_norm) {

		if (threshold_hough(th, rho_norm) < thresh){
			threshold_hough(th, rho_norm) = 0;
		}
		else {
			int rho = rho_norm - pmax;
			double theta = cimg::PI * th / 180;
			v.insert(v.end(), param_space_point(rho, theta, threshold_hough(th, rho_norm)));
		}
	}

	if(debug_disp) printf("thres amount: %lu\n", v.size());


}

CImg<int> Hough_transform::getThresHoughSpace() {
	return threshold_hough;
}

/*
*	localFiltering: A naiive implementation of local point filtering, choose the first point
*	encountered in the local area. Going to be replaced by K-means classification
*	@param
*	v: list of hough space points
*	filtered: list of filtered points
*/

void Hough_transform::localFiltering() {


	filtered.push_back(v[0]);
	vector<int> cnt;
	cnt.push_back(1);

	for (int i = 1; i < v.size(); i++) {

		bool add = true;
		param_space_point p = v[i];

		for (int j = 0; j < filtered.size(); j++) {

			param_space_point p0 = filtered[j];

			if(debug_disp) cout << p.rho << ' ' << p.theta << " | " << p0.rho << ' ' << p0.theta << endl;
			// float dist = sqrt(pow((p.theta - p0.theta) * 57.296f, 2) + pow((p.rho - p0.rho), 2));
			float xdist = abs((p.theta - p0.theta) * 57.296f);
			float ydist = abs(p.rho - p0.rho);
			float dist = xdist + ydist;
			if(debug_disp) cout << "dist: " << dist << endl;

			if (dist < filter_thres ) {
				if(debug_disp) cout << "merging close points" << endl;
				filtered[j].theta = (filtered[j].theta * cnt[j] + p.theta) / float(cnt[j] + 1);
				filtered[j].rho = (filtered[j].rho * cnt[j] + p.rho) / float(cnt[j] + 1);
				cnt[j]++;
				add = false;
				break;
			} 

		}

		if (add) {
			if(debug_disp) cout << "adding new cluster" << endl;
			filtered.push_back(p);
			cnt.push_back(1);
		}

	}

	if(debug_disp) cout << "MERGING double response on vertical lines" << endl;

	//Merge close points in periodic sense.
	for (int i = 0; i < filtered.size(); i++) {

		param_space_point p = filtered[i];

		for (int j = i+1; j < filtered.size(); j++) {

			param_space_point p0 = filtered[j];

			float l1 = p.rho / cos(p.theta);
			float l2 = p0.rho / cos(p0.theta);
			float dist = abs(l1 - l2);

			if (dist < 10 ) {
				if(debug_disp) cout << "merging close points" << endl;
				// filtered[j].theta = (filtered[j].theta * cnt[j] + p.theta * cnt[i]) / float(cnt[j] + cnt[i]);
				// filtered[j].rho = (filtered[j].rho * cnt[j] + p.rho * cnt[i]) / float(cnt[j] + cnt[i]);
				// cnt[j]+=cnt[i];

				filtered.erase(filtered.begin() + i);
				cnt.erase(cnt.begin() + i);
				i--;
				break;

			}
		}

	}


	if(debug_disp) printf("Filtered Param points: %d\n", filtered.size());

}

void Hough_transform::kMeansFiltering() {

	// cout << v.size() << endl;

	
    filtered.reserve(4);
    
    //Ransac choosing the initial kmeans centroid
    {
        int itr = 0;
        
        vector<param_space_point> best_error_init;
        double min_error = DBL_MAX;
        
        while ( itr < 300 )
        {
            double error = 0;
            best_error_init.reserve(4);
            
            //Randomly pick a centroid
            for (int i = 0; i < 4; i++) {
                int j = rand() % v.size();
//                cout << v[j].rho << " " << v[j].theta << endl;
                best_error_init[i] = v[j];
            }
            
            //Classify all points on this classification
            vector<int> classes;
            for (int i = 0; i < v.size(); i++) {
                
                int klass = min4(v[i].L2(best_error_init[0]), v[i].L2(best_error_init[1]), v[i].L2(best_error_init[2]), v[i].L2(best_error_init[3]));
                classes.push_back(klass);
            }
            //Compute the error of this classificaiton
            for (int i = 0; i < v.size(); i++) {
                int k = classes[i];
                
                error += v[i].L2(best_error_init[k]) * v[i].vote;
            }
            if (error < min_error) {
                min_error = error;
                filtered[0] = best_error_init[0];
                filtered[1] = best_error_init[1];
                filtered[2] = best_error_init[2];
                filtered[3] = best_error_init[3];
            }
            
            best_error_init.clear();
            itr++;
        }
    }
    
    double delta = 1e3;
	double error = DBL_MAX;
    double prevError = 0;
	int itr = 0;
	while (abs(error - prevError) > delta) {

		vector<int> classes;
		//Step one: compute the classes of each point.
		for (int i = 0; i < v.size(); i++) {

			int klass = min4(v[i].L2(filtered[0]), v[i].L2(filtered[1]), v[i].L2(filtered[2]), v[i].L2(filtered[3]));
			classes.push_back(klass);
		}

		//Step two: update the center of each classes, with weighted average
		filtered.clear();
		filtered.push_back(param_space_point());
		filtered.push_back(param_space_point());
		filtered.push_back(param_space_point());
		filtered.push_back(param_space_point());

		for (int i = 0; i < v.size(); i++) {
			int k = classes[i];

			filtered[k].rho += v[i].rho * v[i].vote;
			filtered[k].theta += v[i].theta * v[i].vote;
			filtered[k].vote += v[i].vote;
		}

		filtered[0].rho /= double(filtered[0].vote); filtered[0].theta /= filtered[0].vote;
		filtered[1].rho /= double(filtered[1].vote); filtered[1].theta /= filtered[1].vote;
		filtered[2].rho /= double(filtered[2].vote); filtered[2].theta /= filtered[2].vote;
		filtered[3].rho /= double(filtered[3].vote); filtered[3].theta /= filtered[3].vote;

		//Evaluate the error, with weighted error
        prevError = error;
		error = 0;
		for (int i = 0; i < v.size(); i++) {
			int k = classes[i];

			error += v[i].L2(filtered[k]) * v[i].vote;
		}

		itr++;
	}

}


/*
*	computeIntersects: Computing the intersects of different lines and the points in a vector
*	filtered: hough space points, each represents a line in image space
*	intersects: list of intersects
*/
void Hough_transform::computeIntersects() {

for (int i = 0; i < filtered.size(); i++) {

		for (int j = i + 1; j < filtered.size(); j++) {

			param_space_point p0 = filtered[i], p1 = filtered[j];
			float a = cos(p0.theta), b = sin(p0.theta), e = p0.rho,
				c = cos(p1.theta), d = sin(p1.theta), f = p1.rho;

			float det = (a*d-b*c);
			if (abs(det) > 0.01) {
			
				float x = (e*d-b*f)/det;
				float y = (a*f-e*c)/det;

                //Reposition the intersects to original image
				x = x * resize_fac;
				y = y * resize_fac;

				if (x >= 0 && x < img._width && y >= 0 && y < img._height) {

					point intersect;
					intersect.x = int(x);
					intersect.y = int(y);

					intersects.insert(intersects.end(), intersect);

				}

			}
		}

	}

	if(debug_disp) printf("Intersects: %lu\n", intersects.size());
}

vector<point> Hough_transform::getIntersects() {
	return intersects;
}

void Hough_transform::removeClosePoints() {

	for (int i = 0; i < intersects.size(); i++) {

		point p = intersects[i];

		for (int j = i + 1; j < intersects.size(); j++) {
			point p2 = intersects[j];

			if (p.L2DistTo(p2) < 100) {
				intersects.erase(intersects.begin() + i);
				i--;
				break;
			}
		}

	}

}

CImg<unsigned char> Hough_transform::repositionWithHarris() {
    
    int local_size = 200;
    
    for (int i = 0; i < intersects.size(); i++) {
        
        // CImg<unsigned char> local(local_size, local_size, 1, 3);
        
        int x0 = intersects[i].x - local_size / 2;
        int y0 = intersects[i].y - local_size / 2;
        
        if (x0 < 0) x0 = 0;
        if (y0 < 0) y0 = 0;
        if (x0 + local_size > img._width) x0 = img._width - local_size;
        if (y0 + local_size > img._height) y0 = img._height - local_size;
        
        CImg<unsigned char> local = img.get_crop(x0, y0, 0, 0, x0 + local_size, y0 + local_size, 0, 3);
        CImg<float> local_gray(local._width, local._height);
        
        // local = local.get_RGBtoYCbCr().get_channel(0);
        
        cimg_forXY(local, x, y) {
            
            local_gray(x, y) = 0.2126 * local(x,y,0) +
            0.7152 * local(x,y,1) +
            0.0722 * local(x,y,2);
        }
        
        CImg<float> medianFilteredGray(local_gray._width, local_gray._width);
        CImg<> N(5,5);
        cimg_forC(local_gray, c)
        cimg_for5x5(local_gray, x, y, c, 0, N, float)
        medianFilteredGray(x,y,c) = N.median();
        
        if(debug_disp) medianFilteredGray.display();
        
        //Harris algorithm parameters
        // Specifies the sensitivity factor of the Harris algorithm (0 < k < 0.25)
        float k = 0.25;
        // Size of the box filter that is applied to the integral images
        int boxFilterSize = 3;
        // dimension of the maxima suppression box around a maxima
        int maximaSuppressionDimension = 3;
        
        int markDimension = 1;
        
        float percentage = 0.0001;
        bool gauss = true;
        
        Harris hrs(medianFilteredGray, k, boxFilterSize, gauss, false);
        
        vector<pointData> resPts = hrs.getMaximaPoints(percentage, boxFilterSize, maximaSuppressionDimension);
        
        vector<pointData> rp;
        // rp.push_back(resPts[resPts.size() / 2]);
        rp.push_back(resPts[0]);
        
        for (int i = 1; i < resPts.size(); i++) {
            rp[0].p.x += resPts[i].p.x;
            rp[0].p.y += resPts[i].p.y;
        }
        
        rp[0].p.x /= float(resPts.size());
        rp[0].p.y /= float(resPts.size());
        
        CImg<float> result = Util::MarkInImage(medianFilteredGray, rp, markDimension);
        
        //		if(debug_disp) result.display();
        
        int x_off = rp[0].p.x - local_size / 2;
        int y_off = rp[0].p.y - local_size / 2;
        
        intersects[i].x += x_off;
        intersects[i].y += y_off;
        
    }
    
    return result;
}

void addToIntersects(vector<point>& intersects, point p) {

	bool add = true;

	for (int i = 0; i < intersects.size(); i++) {
		if (intersects[i].x == p.x && intersects[i].y == p.y)
			add = false;
	}

	if (add)
		intersects.push_back(p);

}

void Hough_transform::extractLargestRectangle() {

	int tolerance = 200;

	int max = -1;

	vector<point> temp_intersects = intersects;
	intersects.clear();

	for (int i = 0; i < temp_intersects.size(); i++) {
		for (int j = i + 1; j < temp_intersects.size(); j++) {
			for (int k = j + 1; k < temp_intersects.size(); k++) {
				for (int m = k + 1; m < temp_intersects.size(); m++) {

					point p0 = temp_intersects[i], p1 = temp_intersects[j],
					p2 = temp_intersects[k], p3 = temp_intersects[m];

					point mean;

					mean.x = (p0.x + p1.x + p2.x + p3.x) / 4.;
					mean.y = (p0.y + p1.y + p2.y + p3.y) / 4.;

					float l0 = mean.L2DistTo(p0), l1 = mean.L2DistTo(p1),
					l2 = mean.L2DistTo(p2), l3 = mean.L2DistTo(p3);

					float dist = abs(l0 - l1);

					printf("dist: %f, max: %d\n", dist, max);

					if (abs(l0 - l1) < tolerance && abs(l0 - l2) < tolerance && abs(l0 - l3) < tolerance &&
						abs(l1 - l2) < tolerance && abs(l1 - l3) < tolerance &&
						abs(l2 - l3) < tolerance &&
						dist > max) {

						max = dist;
						intersects.clear();

						addToIntersects(intersects, p0);
						addToIntersects(intersects, p1);
						addToIntersects(intersects, p2);
						addToIntersects(intersects, p3);


					}

				}
			}
		}
	}

}


/*
*	plotLine function: A function to plot line on the image, using full image scan 
*	and hough param formula as line formula, O(wh)
*	@param
*	filtered: a vector of hough prameter space points, each represents a line in image space
*	result: the result image to plot on
*/

void Hough_transform::plotLine(vector<param_space_point> filtered, CImg<unsigned char>& result) {

	for (int i = 0; i < filtered.size(); i++) {

		param_space_point p = filtered[i];

			cimg_forXY(result, x, y) {
				if (abs(p.rho - (x/resize_fac) * cosf(p.theta) - (y/resize_fac) * sinf(p.theta)) < 1) {
					result(x, y, 0) = 255;
					result(x, y, 1) = 0;
					result(x, y, 2) = 0;
					// result(x, y) = 255;
						
				}
			}

			if(debug_disp) cout << "Finish Plotting Line" << endl;

	}
}




/*
*	plotPoint function: A function to plot points on the image
*	@param
*	x: x coordinate
*	y: y coordinate
*	img: image to plot on
*/


void Hough_transform::plotPoint(int x, int y, CImg<unsigned char>& img) {

	int r = 3 * resize_fac;

	// printf("Plotting... x:%d, y:%d\n", x, y);
	// printf("w: %d, h: %d\n", img._width, img._height);

	for (int i = x - r; i < x + r; i++) {
		for (int j = y - r; j < y + r; j++) {
			if (i >= 0 && i < img._width && j >= 0 && j < img._height) {
				if (sqrt(pow(i - x, 2) + pow(j - y, 2)) <= r) {
					// printf("Draw... i:%d, j:%d\n", i, j);
					img(i, j, 0) = 255;
					img(i, j, 1) = 255;
					img(i, j, 2) = 0;
				}
			}

		}
	}

	// img.display();

}













