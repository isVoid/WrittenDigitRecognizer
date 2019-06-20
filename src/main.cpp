/*
*	Hough Algorithm
*	Michael Wang SDCS SYSU 2017
*/

#include "headers.h"
#include "Canny.h"
#include "Hough_transform.h"
#include "Warping.h"
#include "TextDetection.hpp"
#include "TextRecognition.hpp"
#include "util.h"

using namespace cimg_library;
using namespace std;

//Show verbose info
bool debug_disp = false;

//Canny Parameter
int gfs = 5;
double g_sig = 3;
int thres_lo = 20;
int thres_hi = 80;

//Hough Parameter
int voting_thres = 64;
float thres_fac = 0.3;
int filter_thres = 200;
bool do_hist_eq = false;

int main(int argc, char** argv) {

    if (argc != 4) {
        cout << "Usage: ./scan input_path output_path txt_output_path" << endl;
        cout << "Example: ./scan data/input.jpg data/output.jpg output.txt" << endl;
        return -1;
    }

    string original_path = string(argv[1]);
    string out_path = string(argv[2]);
    string txt_out_path = string(argv[3]);
    
    Eigen::initParallel();
    
    load_tfmodel();
    
	//Get image, resize
	CImg<unsigned char> img(original_path.c_str());
    float resize_fac = img._width / 1000;
    if (resize_fac == 0) resize_fac = 1;
    
	CImg<unsigned char> resized(img);
	resized.resize(img._width / resize_fac, img._height / resize_fac);

	//Perfrom Canny Edge Detection
	Canny c(resized);
	c.verbose = debug_disp;
	CImg<unsigned char> cny = c.process(gfs, g_sig, thres_lo, thres_hi);
    
	//Perform Hough Transform and edge extraction
	Hough_transform ht(img, cny, resize_fac, debug_disp);
	CImg<unsigned char> result = ht.process(voting_thres, thres_fac, filter_thres);

    //Compute Intersects from lines
	vector<point> intersects = ht.getIntersects();
    
    //Warp image to 4 corners
	Warping warp(img, intersects);
    warp.verbose = debug_disp;
	CImg<unsigned char> warped_result = warp.processWithProjectionTransform();
    
    //Split text regions
    CImg<unsigned char> eroded = warped_result.get_erode(5);
    vector<ct::Rect> proposals = text_contourDetection(eroded);
    
    //Regocnize texts
    CImg<unsigned char> less_erode = warped_result.get_erode(3);
    vector<int> numbers = tfrecognize_num(less_erode, proposals);
    
    //Writing texts to image
    ofstream ofs(txt_out_path, ofstream::out);
    for (int i = 0, j = 0; i < proposals.size() && j < numbers.size() ; i++, j++) {
        ct::Rect& r = proposals[i];
        if (r.x > 0)
        {
            ofs << numbers[j] << " ";
            unsigned char color = 1;
            char b[256];
            sprintf(b, "%d", numbers[j]);
            less_erode.draw_text(r.x, r.y, b, &color, 0, 1, 23);
        }
        else {
            ofs << endl;
            j--;
        }
    }
    ofs.close();
    
    less_erode.display();
    less_erode.save(out_path.c_str());
    
	return 0;
}
