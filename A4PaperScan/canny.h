//
//  canny.h
//  Canny Edge Detector
//
//  Created by Hasan Akgün on 21/03/14.
//  Modifed by Michael Wang on 10/03/17.
//  Software is released under GNU-GPL 2.0
//


// #ifndef _CANNY_
// #define _CANNY_
// #include <string>
// #include <vector>

// #define cimg_use_jpeg

// #include "CImg.h"

// using namespace cimg_library;
// using namespace std;

#pragma once

#include "headers.h"

using namespace cimg_library;
using namespace std;


class canny {
private:

    CImg<unsigned char> img; //Original Image
    CImg<unsigned char> grayscaled; // Grayscale
    CImg<unsigned char> medianFiltered; //Median filtered image
    CImg<unsigned char> gFiltered; // Gradient
    CImg<unsigned char> sFiltered; //Sobel Filtered
    CImg<float> angles; //Angle Map
    CImg<unsigned char> nonMaxSupped; // Non-maxima supp.
    CImg<unsigned char> thres; //Double threshold and final

    CImgDisplay img_disp;
    CImgDisplay gray_disp;
    CImgDisplay fil_img_disp;

    char _name[256];
    int _gfs, _thres_lo, _thres_hi;
    double _g_sig;

public:

    bool verbose;

    //Constructor
    //@param const char* name : name of the image, path will be auto concatenated to "./output/name.bmp". 
    canny(CImg<unsigned char>);

    void toGrayScale();

    void useMedianFilter();

    vector< vector<double> > createFilter(int row, int col, double sigma_in); //Creates a gaussian filter

    void useFilter(CImg<unsigned char>, vector< vector<double> >); //Use some filter

    void sobel(); //Sobel filtering

    void nonMaxSupp(); //Non-maxima supp.
    
    void threshold(CImg<unsigned char>, int, int); //Double threshold and finalize picture

    //Main Process Function with different parameter setting.

    //@param
    //int gfs: gaussian filter size, odd number only!!!
    //double g_sig: gaussian sigma
    //int thres_lo: lower bound of double thresholding, should be less than higher bound, 0-255.
    //int thres_hi: uppoer bound of double thresholding, should be higher than lower bound, 0-255.

    //@return
    //CImg<unsigned char> final result of processed image.
    CImg<unsigned char> process(int gfs, double g_sig, int thres_lo, int thres_hi);

    //Boolean to control if needed display or output only
    //True for display and save
    //False for save only
    void displayandsave(bool on);

};

// #endif
