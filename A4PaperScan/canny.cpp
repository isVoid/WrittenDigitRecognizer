//
//  canny.cpp
//  Canny Edge Detector
//
//  Created by Hasan Akgün on 21/03/14.
//  Modifed by Michael Wang on 10/03/17.
//  Software is released under GNU-GPL 2.0
//

#include "headers.h"
#include "canny.h"

typedef unsigned char uchar;

using namespace cimg_library;
using namespace std;


canny::canny(CImg<unsigned char> _img)
{
    img = _img;

    verbose = false;
}

void canny::displayandsave(bool on) {

        if (on){
            // img.display("Original"); //Original Image
            // img_disp = CImgDisplay(img, "Original");
            // gray_disp = CImgDisplay(grayscaled, "Grayscale");

            // CImgList<unsigned char> fil_img_list = CImgList<unsigned char>(gFiltered, sFiltered, nonMaxSupped, thres);
            // fil_img_disp = CImgDisplay(fil_img_list, "Each Steps");
            img_disp = CImgDisplay(thres, "result");

            // while (!img_disp.is_closed() && !gray_disp.is_closed() && !fil_img_disp.is_closed()) {
            while (!img_disp.is_closed()) {
                img_disp.wait();
            }
        }

        char path[256] = "./output/";
        char name[256];
        sprintf(name, "%s_gfs%d_gsig%.2f_threslo%d_threshi%d.jpeg", _name, _gfs, _g_sig, _thres_lo, _thres_hi);

        strcat(path, name);
        cout << "saving to " << path << endl;
        thres.save_jpeg(path, 70);

}

CImg<unsigned char> canny::process(int gfs = 3, double g_sig = 1, int thres_lo = 20, int thres_hi = 40) {

    _gfs = gfs;
    _g_sig = g_sig;
    _thres_lo = thres_lo;
    _thres_hi = thres_hi;

    vector< vector<double> > filter = createFilter(gfs, gfs, g_sig);

    //Print filter
    // for (int i = 0; i<filter.size(); i++) 
    // {
    //     for (int j = 0; j<filter[i].size(); j++) 
    //     {
    //         cout << filter[i][j] << " ";
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    if(verbose) cout << "gFilter created" << endl;
    toGrayScale(); //Grayscale the image
    if(verbose) cout << "perform median filter" << endl;
    useMedianFilter();
    if(verbose) cout << "image converted to grayscale" << endl;
    useFilter(grayscaled, filter); //Gaussian Filter
    if(verbose) cout << "image filtered with Gaussian filter" << endl;
    sobel(); //Sobel Filter
    if(verbose) cout << "image filtered with sobel filter" << endl;

    nonMaxSupp(); //Non-Maxima Suppression
    if(verbose) cout << "image filtered with non-maxima Suppression" << endl;
    threshold(nonMaxSupped, thres_lo, thres_hi); //Double Threshold and Finalize
    if(verbose) cout << "image filtered with double thresholding" << endl;

    return thres;
}

void canny::toGrayScale()
{
    grayscaled.assign(img._width, img._height); //To one channel
    cimg_forXY(img, x, y) {
        
        int r = img(x,y,0);
        int g = img(x,y,1);
        int b = img(x,y,2);
        double newValue = (r * 0.2126 + g * 0.7152 + b * 0.0722);

        grayscaled(x,y) = (unsigned char)(newValue);
    }

}

void canny::useMedianFilter() {

    medianFiltered.assign(grayscaled);
    CImg<> N(5,5);
    cimg_forC(grayscaled, c) {
        cimg_for5x5(grayscaled, x, y, 0, c, N, uchar) {
            // printf("%d, %d\n", x, y);
            medianFiltered(x, y, c) = N.median();

        }
    }



    grayscaled.assign(medianFiltered);

}

vector< vector<double> > canny::createFilter(int row, int column, double sigmaIn)
{
	vector< vector<double> > filter;

	for (int i = 0; i < row; i++)
	{
        vector<double> col;
        for (int j = 0; j < column; j++)
        {
            col.push_back(-1);
        }
		filter.push_back(col);
	}

	float coordSum = 0;
	float constant = 2.0 * sigmaIn * sigmaIn;

	// Sum is for normalization
	float sum = 0.0;

	for (int x = - row/2; x <= row/2; x++)
	{
		for (int y = -column/2; y <= column/2; y++)
		{
			coordSum = (x*x + y*y);
			filter[x + row/2][y + column/2] = (exp(-(coordSum) / constant)) / (M_PI * constant);
			sum += filter[x + row/2][y + column/2];
		}
	}

	// Normalize the Filter
	for (int i = 0; i < row; i++)
        for (int j = 0; j < column; j++)
            filter[i][j] /= sum;

	return filter;

}

void canny::useFilter(CImg<unsigned char> img_in, vector< vector<double> > filterIn)
{
    int size = (int)filterIn.size()/2;
    gFiltered = CImg<unsigned char>(img_in._width - 2*size, img_in._height - 2*size);
	for (int i = size; i < img_in._width - size; i++)
	{
		for (int j = size; j < img_in._height - size; j++)
		{
			double sum = 0;
            
			for (int x = 0; x < filterIn.size(); x++)
				for (int y = 0; y < filterIn.size(); y++)
				{
                    sum += filterIn[x][y] * (double)(img_in(i + y - size, j + x - size));
				}
            
            gFiltered(i-size, j-size) = sum;
		}

	}

}

void canny::sobel()
{

    //Sobel X Filter
    double x1[] = {-1.0, 0, 1.0};
    double x2[] = {-2.0, 0, 2.0};
    double x3[] = {-1.0, 0, 1.0};

    vector< vector<double> > xFilter(3);
    xFilter[0].assign(x1, x1+3);
    xFilter[1].assign(x2, x2+3);
    xFilter[2].assign(x3, x3+3);
    
    //Sobel Y Filter
    double y1[] = {1.0, 2.0, 1.0};
    double y2[] = {0, 0, 0};
    double y3[] = {-1.0, -2.0, -1.0};
    
    vector< vector<double> > yFilter(3);
    yFilter[0].assign(y1, y1+3);
    yFilter[1].assign(y2, y2+3);
    yFilter[2].assign(y3, y3+3);
    
    //Limit Size
    int size = (int)xFilter.size()/2;
    
    sFiltered = CImg<unsigned char>(gFiltered._width - 2*size, gFiltered._height - 2*size);
    
    angles = CImg<unsigned char>(gFiltered._width - 2*size, gFiltered._height - 2*size); //AngleMap

	for (int i = size; i < gFiltered._height - size; i++)
	{
		for (int j = size; j < gFiltered._width - size; j++)
		{
			double sumx = 0;
            double sumy = 0;
            
			for (int x = 0; x < xFilter.size(); x++)
				for (int y = 0; y < xFilter.size(); y++)
				{
                    sumx += xFilter[x][y] * (double)(gFiltered(j + y - size, i + x - size)); //Sobel_X Filter Value
                    sumy += yFilter[x][y] * (double)(gFiltered(j + y - size, i + x - size)); //Sobel_Y Filter Value
				}
            double sumxsq = sumx*sumx;
            double sumysq = sumy*sumy;
            
            double sq2 = sqrt(sumxsq + sumysq);
            
            if(sq2 > 255) //Unsigned Char Fix
                sq2 =255;
            sFiltered(j-size, i-size) = sq2;
 
            if(sumx==0) //Arctan Fix
                angles(j-size, i-size) = 90;
            else
                angles(j-size, i-size) = atan(sumy/sumx) * 57.296f;
		}
	}
    
}


void canny::nonMaxSupp()
{
    nonMaxSupped = CImg<unsigned char>(sFiltered._width-2, sFiltered._height-2);
    for (int i=1; i< sFiltered._width - 1; i++) {
        for (int j=1; j<sFiltered._height - 1; j++) {
            float Tangent = angles(i,j);
            // cout << Tangent << ' ';
            nonMaxSupped(i-1, j-1) = sFiltered(i,j);
            //Horizontal Edge
            if (((-22.5 < Tangent) && (Tangent <= 22.5)) || ((157.5 < Tangent) && (Tangent <= -157.5)))
            {
                if ((sFiltered(i,j) < sFiltered(i+1,j)) || (sFiltered(i,j) < sFiltered(i-1,j)))
                    nonMaxSupped(i-1, j-1) = 0;
            }
            //Vertical Edge
            if (((-112.5 < Tangent) && (Tangent <= -67.5)) || ((67.5 < Tangent) && (Tangent <= 112.5)))
            {
                if ((sFiltered(i,j) < sFiltered(i,j+1)) || (sFiltered(i,j) < sFiltered(i,j-1)))
                    nonMaxSupped(i-1, j-1) = 0;
            }
            
            //-45 Degree Edge
            if (((-67.5 < Tangent) && (Tangent <= -22.5)) || ((112.5 < Tangent) && (Tangent <= 157.5)))
            {
                if ((sFiltered(i,j) < sFiltered(i+1,j+1)) || (sFiltered(i,j) < sFiltered(i-1,j-1)))
                    nonMaxSupped(i-1, j-1) = 0;

            }
            
            //45 Degree Edge
            if (((-157.5 < Tangent) && (Tangent <= -112.5)) || ((22.5 < Tangent) && (Tangent <= 67.5)))
            {
                if ((sFiltered(i,j) < sFiltered(i-1,j+1)) || (sFiltered(i,j) < sFiltered(i+1,j-1)))
                    nonMaxSupped(i-1, j-1) = 0;
            }
        }
        // cout << '\n';
    }

}

void canny::threshold(CImg<unsigned char> imgin,int low, int high)
{
    if(low > 255)
        low = 255;
    if(high > 255)
        high = 255;
    
    thres = CImg<unsigned char>(imgin._width, imgin._height);
    
    for (int i=0; i<imgin._width; i++) 
    {
        for (int j = 0; j<imgin._height; j++) 
        {
            thres(i,j) = imgin(i,j);
            if(thres(i,j) > high)
                thres(i,j) = 255;
            else if(thres(i,j) < low)
                thres(i,j) = 0;
            else
            {
                bool anyHigh = false;
                bool anyBetween = false;
                for (int x=i-1; x < i+2; x++) 
                {
                    for (int y = j-1; y<j+2; y++) 
                    {
                        //Wang Note: a missing "x" in Hasan's code.
                        if(x <= 0 || y <= 0 || x > thres._width || y > thres._height) //Out of bounds
                            continue;
                        else
                        {
                            if(thres(x,y) > high)
                            {
                                thres(i,j) = 255;
                                anyHigh = true;
                                break;
                            }
                            else if(thres(x,y) <= high && thres(x,y) >= low)
                                anyBetween = true;
                        }
                    }
                    if(anyHigh)
                        break;
                }
                if(!anyHigh && anyBetween)
                    for (int x=i-2; x < i+3; x++) 
                    {
                        for (int y = j-1; y<j+3; y++) 
                        {
                            if(x < 0 || y < 0 || x > thres._width || y > thres._height) //Out of bounds
                                continue;
                            else
                            {
                                if(thres(x,y) > high)
                                {
                                    thres(i,j) = 255;
                                    anyHigh = true;
                                    break;
                                }
                            }
                        }
                        if(anyHigh)
                            break;
                    }
                if(!anyHigh)
                    thres(i,j) = 0;
            }
        }
    }

}