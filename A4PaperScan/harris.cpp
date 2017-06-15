/*
 *      Author: alexanderb
 *		Adapted into CImg dependency by Michael Wang
 */

#include "harris.h"

Harris::Harris(CImg<float> img, float k, int filterRange, bool gauss, bool vrbs) {

    verbose = vrbs;

    // (1) Convert to greyscalescale image
    // CImg<float> greyscaleImg = img.get_normalize(0,255).get_RGBtoYCbCr().get_channel(0);
    CImg<float> greyscaleImg(img);


    if(verbose) greyscaleImg.display();

    // (2) Compute Derivatives
    Derivatives derivatives = computeDerivatives(greyscaleImg);

    if(verbose) derivatives.Ix.display();
    if(verbose) derivatives.Iy.display();
    if(verbose) derivatives.Ixy.display();

    // (3) Median Filtering
    Derivatives mDerivatives;
    if(gauss) {
        mDerivatives = applyGaussToDerivatives(derivatives, filterRange);
    } else {
        mDerivatives = applyMeanToDerivatives(derivatives, filterRange);
    }

    if(verbose) mDerivatives.Ix.display();
    if(verbose) mDerivatives.Iy.display();
    if(verbose) mDerivatives.Ixy.display();

    // (4) Compute Harris Responses
    CImg<float> harrisResponses = computeHarrisResponses(k, mDerivatives);

    if(verbose) harrisResponses.display();

    CImg<unsigned char> debugHarrisResponse(harrisResponses);
    cimg_forXY(debugHarrisResponse, x, y) {
        if (harrisResponses(x,y) > 255) {
            debugHarrisResponse(x,y) = 255;
        } else {
            debugHarrisResponse(x,y) = 0;
        }
    }

    if(verbose) debugHarrisResponse.display();

    m_harrisResponses = harrisResponses;
}

//-----------------------------------------------------------------------------------------------
vector<pointData> Harris::getMaximaPoints(float percentage, int filterRange, int suppressionRadius) {
    // Declare a max suppression matrix
    // CImg<unsigned char> maximaSuppressionMat(m_harrisResponses._width, m_harrisResponses._height);
    int maximaSuppressionMat[1024][1024];
    memset(maximaSuppressionMat, 0, sizeof(int) * 1024 * 1024);

    // Create a vector of all Points
    std::vector<pointData> points;
    for (int r = 0; r < m_harrisResponses._width; r++) {
        for (int c = 0; c < m_harrisResponses._height; c++) {
            point p(r,c); 

            pointData d;
            d.cornerResponse = m_harrisResponses(r,c);         

            d.p = p;

            // printf("x:%d, y:%d, Response: %f\n", d.p.x, d.p.y, d.cornerResponse);   

            points.push_back(d);
        }
    }

    // Sort points by corner Response
    sort(points.begin(), points.end(), by_cornerResponse());

    // for (int i = 0; i < 10; i++)
    //     printf("x:%d, y:%d, Response: %f\n", points[i].p.x, points[i].p.y, points[i].cornerResponse);

    // Get top points, given by the percentage
    // int numberTopPoints = m_harrisResponses._width * m_harrisResponses._height * percentage;
    int numberTopPoints = 100;
    std::vector<pointData> topPoints;

    int i=0;
    while(topPoints.size() < numberTopPoints) {
        if(i == points.size())
            break;

        // int supRows = maximaSuppressionMat._height;
        // int supCols = maximaSuppressionMat._width;
        int supRows = m_harrisResponses._height;
        int supCols = m_harrisResponses._width;

        // Check if point marked in maximaSuppression matrix
        // if(maximaSuppressionMat.atXY(points[i].p.x,points[i].p.y) == 0) {
        if(maximaSuppressionMat[points[i].p.x][points[i].p.y] == 0){

            for (int r = -suppressionRadius; r <= suppressionRadius; r++) {
                for (int c = -suppressionRadius; c <= suppressionRadius; c++) {

                    int sx = points[i].p.x+c;
                    int sy = points[i].p.y+r;

                    // bound checking
                    if(sx > supCols - 1)
                        sx = supCols - 1;
                    if(sx < 0)
                        sx = 0;
                    if(sy > supRows - 1)
                        sy = supRows - 1;
                    if(sy < 0)
                        sy = 0;

                    // maximaSuppressionMat.atXY(sx, sy) = 1;
                    maximaSuppressionMat[sx][sy] = 1;
                }
            }

            // Convert back to original image coordinate system 
            points[i].p.x += 1 + filterRange;
            points[i].p.y += 1 + filterRange;

            if(verbose) printf("x:%d, y:%d, Response: %f\n", points[i].p.x, points[i].p.y, points[i].cornerResponse);

            topPoints.push_back(points[i]);
        }

        i++;
    }

    return topPoints;
}


//-----------------------------------------------------------------------------------------------
Derivatives Harris::applyGaussToDerivatives(Derivatives& dMats, int filterRange) {
    if(filterRange == 0)
        return dMats;

    Derivatives mdMats;

    mdMats.Ix = gaussFilter(dMats.Ix, filterRange);
    mdMats.Iy = gaussFilter(dMats.Iy, filterRange);
    mdMats.Ixy = gaussFilter(dMats.Ixy, filterRange);

    return mdMats;
}

//-----------------------------------------------------------------------------------------------
Derivatives Harris::applyMeanToDerivatives(Derivatives& dMats, int filterRange) {
    if(filterRange == 0)
        return dMats;

    Derivatives mdMats;

    CImg<float> mIx = computeIntegralImg(dMats.Ix);
    CImg<float> mIy = computeIntegralImg(dMats.Iy);
    CImg<float> mIxy = computeIntegralImg(dMats.Ixy);

    mdMats.Ix = meanFilter(mIx, filterRange);
    mdMats.Iy = meanFilter(mIy, filterRange);
    mdMats.Ixy = meanFilter(mIxy, filterRange);

    return mdMats;
}

//-----------------------------------------------------------------------------------------------
Derivatives Harris::computeDerivatives(CImg<float>& greyscaleImg) {
    // Helper Mats for better time complexity
    CImg<float> sobelHelperV( greyscaleImg._width, greyscaleImg._height-2 );
    for(int r=1; r<greyscaleImg._height-1; r++) {
        for(int c=0; c<greyscaleImg._width; c++) {

            float a1 = greyscaleImg(c,r-1);
            float a2 = greyscaleImg(c,r);
            float a3 = greyscaleImg(c,r+1);

            sobelHelperV(c,r-1) = a1 + a2 + a2 + a3;
        }
    }

    CImg<float> sobelHelperH( greyscaleImg._width-2, greyscaleImg._height);
    for(int r=0; r<greyscaleImg._height; r++) {
        for(int c=1; c<greyscaleImg._width-1; c++) {

            float a1 = greyscaleImg(c-1,r);
            float a2 = greyscaleImg(c,r);
            float a3 = greyscaleImg(c+1,r);

            sobelHelperH(c-1,r) = a1 + a2 + a2 + a3;
        }
    }

    // Apply Sobel filter to compute 1st derivatives
    CImg<float> Ix( greyscaleImg._width-2, greyscaleImg._height-2 );
    CImg<float> Iy( greyscaleImg._width-2, greyscaleImg._height-2 );
    CImg<float> Ixy( greyscaleImg._width-2, greyscaleImg._height-2 );

    for(int r=0; r<greyscaleImg._height-2; r++) {
        for(int c=0; c<greyscaleImg._width-2; c++) {
            Ix(c,r) = sobelHelperH(c,r) - sobelHelperH(c,r+2);
            Iy(c,r) = - sobelHelperV(c,r) + sobelHelperV(c+2,r);
            Ixy(c,r) = Ix(c,r) * Iy(c,r);
        }
    }

    Derivatives d;
    d.Ix = Ix;
    d.Iy = Iy;
    d.Ixy = Iy;

    return d;
}

//-----------------------------------------------------------------------------------------------
CImg<float> Harris::computeHarrisResponses(float k, Derivatives& d) {
    CImg<float> M( d.Ix._width, d.Iy._height );

    for(int r=0; r<d.Iy._height; r++) {  
        for(int c=0; c<d.Iy._width; c++) {
            float   a11, a12,
                    a21, a22;

            a11 = d.Ix(c,r) * d.Ix(c,r);
            a22 = d.Iy(c,r) * d.Iy(c,r);
            a21 = d.Ix(c,r) * d.Iy(c,r);
            a12 = d.Ix(c,r) * d.Iy(c,r);

            float det = a11*a22 - a12*a21;
            float trace = a11 + a22;

            M(c,r) = abs(det - k * trace*trace);

            // printf("c:%d, r:%d, det: %f, trace: %f, M(c,r): %f\n",c, r, det, trace, M(c,r));
        }
    }

    return M;
}

//-----------------------------------------------------------------------------------------------
CImg<float> Harris::computeIntegralImg(CImg<float>& img) {

    CImg<float> integralMat(img._width, img._height);

    integralMat(0,0) = img(0,0);

    for (int j = 1; j < img._width; j++) {
        integralMat(j,0) = 
            integralMat(j-1,0) 
            + img(j,0);
    }    

    for (int i = 1; i < img._height; i++) {
        integralMat(0,i) = 
            integralMat(0,i-1) 
            + img(0,i);
    }

    for (int i = 1; i < img._height; i++) {
        for (int j = 1; j < img._width; j++) {
            integralMat(j,i) = 
                img(j,i)
                + integralMat(j-1,i)
                + integralMat(j,i-1)
                - integralMat(j-1,i-1);
        }
    }

    return integralMat;
}

//-----------------------------------------------------------------------------------------------
CImg<float> Harris::meanFilter(CImg<float>& intImg, int range) {

    CImg<float> medianFilteredMat( intImg._width-range*2, intImg._height-range*2 );

    for (int r = range; r < intImg._width-range; r++) {
        for (int c = range; c < intImg._height-range; c++) {
            medianFilteredMat(r-range, c-range) = 
                intImg(r+range, c+range)
                + intImg(r-range, c-range)
                - intImg(r+range, c-range)
                - intImg(r-range, c+range);
        }
    }

    // CImg<float> meanFilterdMat(intImg._width, intImg._width);

    // CImg<> N(5, 5);
    // cimg_for5x5(intImg, x, y, 0, 0, N, float) {
    //     meanFilterdMat(x, y) = N.sum() / 25;
    // }

    return medianFilteredMat;
    // return meanFilterdMat;
}

CImg<float> Harris::gaussFilter(CImg<float>& img, int range) {
    // Helper Mats for better time complexity

    CImg<float> gaussHelperV( img._width-range*2, img._height-range*2 );

    for(int r=range; r<img._width-range; r++) {
        for(int c=range; c<img._height-range; c++) {
            float res = 0;

            for(int x = -range; x<=range; x++) {
                float m = 1/sqrt(2*M_PI)*exp(-0.5*x*x);

                res += m * img(r-range,c-range);
            }

            gaussHelperV(r-range,c-range) = res;
        }
    }

    CImg<float> gauss( img._width-range*2, img._height-range*2 );

    for(int r=range; r<img._width-range; r++) {
        for(int c=range; c<img._height-range; c++) {
            float res = 0;

            for(int x = -range; x<=range; x++) {
                float m = 1/sqrt(2*M_PI)*exp(-0.5*x*x);

                res += m * gaussHelperV(r-range,c-range);
            }

            gauss(r-range,c-range) = res;
        }
    }

    // CImg<float> gauss = img.blur(1, false, false);

    return gauss;
}


















