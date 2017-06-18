/*
 *      Author: alexanderb
 *		Adapted into CImg dependency by Michael Wang
 */

#include "util.h"

void Util::DisplayImage(CImg<float>& img) {

	//Display image
    img.display();

}

//-----------------------------------------------------------------------------------------------
void Util::DisplayMat(CImg<float>& img) {
	cout << "-= Display Matrix =-";

	int rowCount = 0;

    for (int r = 0; r < img._height; r++) {
    	cout << endl;

    	int colCount = 0;

        for (int c = 0; c < img._width; c++) {
            cout << img(c,r) << ", ";
            colCount++;
        }

        rowCount++;

        cout << " -> " << colCount << "cols";
    }

    cout << "-> " << rowCount << "rows" << endl;
}

//-----------------------------------------------------------------------------------------------
void Util::DisplayPointVector(vector<point> vp) {
    vector<point>::iterator pIterator;

    for(int i=0; i<vp.size(); i++) {
        point p = vp[i];

        cout << p.x << "," << p.y << "; ";
    }
}

//-----------------------------------------------------------------------------------------------
CImg<float> Util::MarkInImage(CImg<float>& img, vector<pointData> points, int r) {
	CImg<float> retImg(img);

	for(vector<pointData>::iterator it = points.begin(); it != points.end(); ++it) {
		point center = (*it).p;

		// // down
		// for(int r=-radius; r<radius; r++) {
		// 	retImg(Point(center.y+r,center.x+radius)) = Vec3b(0, 0, 255);
		// }

		// // up
		// for(int r=-radius; r<radius; r++) {
		// 	retImg.at<Vec3b>(Point(center.y+r,center.x-radius)) = Vec3b(0, 0, 255);
		// }

		// // left
		// for(int c=-radius; c<radius; c++) {
		// 	retImg.at<Vec3b>(Point(center.y-radius,center.x+c)) = Vec3b(0, 0, 255);
		// }

		// // right
		// for(int c=-radius; c<radius; c++) {
		// 	retImg.at<Vec3b>(Point(center.y+radius,center.x+c)) = Vec3b(0, 0, 255);
		// }

		// retImg.at<Vec3b>(Point(center.y,center.x)) = Vec3b(0, 255, 0);
		int x = center.x;
		int y = center.y;
		for (int i = x - r; i < x + r; i++) {
			for (int j = y - r; j < y + r; j++) {
				if (i >= 0 && i < img._width && j >= 0 && j < img._height) {
					if (sqrt(pow(i - x, 2) + pow(j - y, 2)) <= r) {
						// printf("Draw... i:%d, j:%d\n", i, j);
						retImg(i, j, 0) = 255;
						retImg(i, j, 1) = 255;
						retImg(i, j, 2) = 0;
					}
				}

			}
		}

	}

	return retImg;
}


/*
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












