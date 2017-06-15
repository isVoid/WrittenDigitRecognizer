/*
*	Hough Algorithm
*	Michael Wang SDCS SYSU 2017
*/


// #include <iostream>

// #include "CImg.h"
// #include "Hough_transform.h"
// #include "canny.h"
// #include "imgutil.h"

// using namespace cimg_library;

#include "headers.h"
#include "canny.h"
#include "Hough_transform.h"
#include "Warping.h"

using namespace cimg_library;
using namespace std;


bool debug_disp = false;

double PI = 3.1415;

/*
*	@Override
*	getHist
*	Calculates a normalized gray image's histogram
*	
*	@param:
*	T hist[256]: An array to store the images histogram
*	CImg<float> img: Image to calculate
*/
template <typename T> void getHist(T hist[256], CImg<float> img) {
	cimg_forXY(img, x, y) {
		hist[int(img(x,y) * 255)] += 1;
	}
}

/*
*	histgramEq
*	Equalized a histogram
*	
*	@param:
*	T hist[256]: A(n) (image/channel)'s histogram
*	int size: total pixel number of the image/channel, usually w x h.
*/
template <typename T> void histgramEq(T hist[256], int size) {
	
	//Caculate cummulative density function
	for(int i = 1; i < 256; i++) {
		hist[i] = hist[i] + hist[i-1];
	}

	//Compute color transform function for equalization
	for(int i = 0; i < 256; i++) {
		hist[i] = float(hist[i]) / size;
		hist[i] = (unsigned int)(255 * hist[i]);
	}

}

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
CImg<unsigned char> histgramEq_hsi(CImg<unsigned char> in) {

	CImg<float>in_hsi = in.get_RGBtoHSI();

	int size = in._width * in._height;

	float hist_i[256] = {};

	CImg<float> intensity = in_hsi.get_channel(2);
	getHist(hist_i, intensity);

	histgramEq(hist_i, size);

	//Applying color transform function to image
	cimg_forXY(intensity, x, y) {
		in_hsi.atXYZC(x,y,1,2) = hist_i[int(intensity(x,y) * 255)] / 255.0;
	}

	CImg<unsigned char> out = in_hsi.get_HSItoRGB();

	return out;
}



/*
*	Main Function: Provides input of canny and hough parameters.
*	Parameter List, from left to right:
*
*	Path_Image_to_detect: string
*	Perform_Histogram_equalization: bool(0, 1)
*	Gaussian_filter_size: odd int
*	Gaussian_sigma: float
*	threshold_lower: int
*	threshold_higher: int
*	voting_threshold: int
*	threshold_factor: float
*	filtering_treshold: int
*	Show_debug_windows: bool(0, 1)
*	Path_Result_to_save: string
*
*	Sample usage: ./A4PaperScan "./data/5.jpg" 0 5 1.5 40 60 64 0.4 30 8 0 "./output/5.jpg"
*
*	For more information, refer to manual/report
*/

float resize_fac = 3;
//Canny Parameter
int gfs = 5;
double g_sig = 3;
int thres_lo = 40;
int thres_hi = 60;

//Hough Parameter
int voting_thres = 64;  //Stub, does not have real effect since canny image is binarilized.
float thres_fac = 0.3;  //Threashold factor for hough space
int filter_thres = 200; //Filtering radius only for local filtering. Does not have effect now.

int main(int argc, char** argv) {

    char* original_path = "../../data/4.jpg";
    char* out_path = "../../output/4.jpg";
    
    bool do_hist_eq = false;
    debug_disp = true;
    
	//Get image
	CImg<unsigned char> img(original_path);
    resize_fac = img._width / 1000;
    cout << resize_fac << endl;
    if (resize_fac == 0) resize_fac = 1;
    
    
	CImg<unsigned char> resized(img);
	resized.resize(img._width / resize_fac, img._height / resize_fac);

	//Perform historgram equalization (optional)
	if (do_hist_eq) {
		resized = histgramEq_hsi(resized);	
	}

	if (debug_disp)
		resized.display();

	//Perfrom Canny Edge Detection
	canny c(resized);
	c.verbose = debug_disp;

	CImg<unsigned char> cny = c.process(gfs, g_sig, thres_lo, thres_hi);

	//Perform Hough Transform and edge extraction
	Hough_transform ht(img, cny, resize_fac, debug_disp);

	CImg<unsigned char> result = ht.process(voting_thres, thres_fac, filter_thres);

	vector<point> intersects = ht.getIntersects();

	Warping warp(img, intersects);
    warp.verbose = true;
    
	CImg<unsigned char> warped_result = warp.processWithProjectionTransform();

	warped_result.display();

	warped_result.save_jpeg(out_path, 70);
	printf("Result saved to ./output/\n");

	return 0;
}
