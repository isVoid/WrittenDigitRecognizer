/*
*	Hough Algorithm
*	Michael Wang SDCS SYSU 2017
*/

#include "headers.h"
#include "canny.h"
#include "Hough_transform.h"
#include "Warping.h"
#include "TextDetection.hpp"
#include "TextRecognition.hpp"
#include "util.h"

using namespace cimg_library;
using namespace std;


bool debug_disp = false;

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

    //Load Predict Model First
    load_model();
    
    char* original_path = "../../data/5.jpg";
    char* out_path = "../../output/5.jpg";
    
    bool do_hist_eq = false;
    debug_disp = false;
    
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
    warp.verbose = false;
    
	CImg<unsigned char> warped_result = warp.processWithProjectionTransform();

    CImg<unsigned char> eroded = warped_result.get_erode(3);

    vector<ss::Rect> proposals = text_detection(eroded);
    
    vector<int> numbers = recognize_num(eroded, proposals);
    
    cout << "Drawing results" << endl;
    for (int i; i < numbers.size(); i++) {
        ss::Rect& r = proposals[i];
//        unsigned char red[] = {255, 0, 0};
        unsigned char color = 1;
        char b[256];
        sprintf(b, "%d", numbers[i]);
        eroded.draw_text(r.x, r.y, b, &color, 0, 1, 23);
    }
    
    eroded.display();
	printf("Result saved to ./output/\n");

    free_model();
    
	return 0;
}
