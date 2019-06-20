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


class Canny {
private:

    CImg<unsigned char> img; //Original Image
    CImg<unsigned char> grayscaled; // Grayscale
    CImg<unsigned char> medianFiltered; //Median filtered image
    CImg<unsigned char> gFiltered; // Gradient
    CImg<unsigned char> sFiltered; //Sobel Filtered
    CImg<float> angles; //Angle Map
    CImg<unsigned char> nonMaxSupped; // Non-maxima suppression.
    CImg<unsigned char> thres; //Double threshold and final

    char _name[256];
    int _gfs, _thres_lo, _thres_hi;
    double _g_sig;

public:

    bool verbose;

    Canny(CImg<unsigned char>);

    void toGrayScale();

    void useMedianFilter();

    vector< vector<double> > createFilter(int row, int col, double sigma_in); //Creates a gaussian filter

    void useFilter(CImg<unsigned char>, vector< vector<double> >); //Apply filter to image

    void sobel_anglemap(); //Sobel filtering and compute angle map

    void nonMaxSupp(); //Non-maxima suppression in 8 directions
    
    void threshold(CImg<unsigned char>, int, int); //Binarize image

    /**
     *  Main Process Function
     *  @param
     *  gfs: gaussian filter size, odd number only!!!
     *  g_sig: gaussian sigma
     *  thres_lo: lower bound of threshold, should be less than higher bound, 0-255.
     *  thres_hi: uppoer bound of threshold, should be higher than lower bound, 0-255.
     *  @return
     *  Canny filtered image
     */
    
    CImg<unsigned char> process(int gfs, double g_sig, int thres_lo, int thres_hi);


};

// #endif
