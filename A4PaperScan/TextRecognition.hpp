//
//  TextRecognition.hpp
//  A4PaperScan
//
//  Created by 王 颖豪 on 6/16/17.
//  Copyright © 2017 TSK. All rights reserved.
//

#ifndef TextRecognition_hpp
#define TextRecognition_hpp

#include "headers.h"
#include "selective_search.hpp"
#include <svm.h>

using namespace std;
using namespace cimg_library;

void load_model();
void free_model();

vector<int> recognize_num(const CImg<unsigned char>& image, vector<ss::Rect> regions);

#endif /* TextRecognition_hpp */
