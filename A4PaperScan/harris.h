/*
 *      Author: alexanderb
 *		Adapted into CImg dependency by Michael Wang
 */

#include "headers.h"
#include "Util.h"

using namespace cimg_library;
using namespace std;

class Harris {
public:
    Harris(CImg<float> img, float k, int filterRange, bool gauss, bool verbose);
	vector<pointData> getMaximaPoints(float percentage, int filterRange, int suppressionRadius);

private:

	bool verbose;

	Derivatives computeDerivatives(CImg<float>& greyscaleImg);	
	Derivatives applyMeanToDerivatives(Derivatives& dMats, int filterRange);
	Derivatives applyGaussToDerivatives(Derivatives& dMats, int filterRange);
	CImg<float> computeHarrisResponses(float k, Derivatives& intMats);

	CImg<float> computeIntegralImg(CImg<float>& img);
	CImg<float> meanFilter(CImg<float>& intImg, int range);
	CImg<float> gaussFilter(CImg<float>& img, int range);

private:
	CImg<float> m_harrisResponses;
};