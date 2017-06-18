//
//  TextRecognition.cpp
//  A4PaperScan
//
//  Created by 王 颖豪 on 6/16/17.
//  Copyright © 2017 TSK. All rights reserved.
//

#include "TextRecognition.hpp"
#include <cstdlib>

svm_model* model;
svm_node* nodes;

CImg<> cimgFromRect(const CImg<>& image, ss::Rect region) {
    
    CImg<> ROI(region.width, region.height, 1, image._spectrum);
    
    cimg_forC(ROI, c) {
        cimg_forXY(ROI, x, y) {
            if (region.x + x < image._width && region.y + y < image._height) {
                ROI(x, y, c) = image(region.x + x, region.y + y, c);
            }
        }
    }
    
    return ROI;
}

CImg<> negative(const CImg<>& img) {
    CImg<> n = img.get_normalize(0, 1);
    
    cimg_forXY(img, x, y) {
        n(x, y) = 1 - img(x, y);
    }
    
    return n;
}

void nodeFromCImg(const CImg<>& img) {
    
    assert(img._spectrum == 1); //Must be grayscaled image
    if (nodes != NULL) {
        free(nodes);
    }
    nodes = (svm_node*) malloc((img.width() * img.height() + 1) * sizeof(svm_node));
//    nodes = (svm_node*) realloc(nodes, (img.width() * img.height() + 1) * sizeof(svm_node));
    
    int i = 0;
    for (; i < img.width() * img.height(); i++) {
        nodes[i].index = i;
        nodes[i].value = img.data()[i];
    }
    //Last dimension set as -1
    nodes[i].index = -1;
    
}

void load_model() {
    
    string filename = "../../model/mnist.model";
    
    cout << "Loading SVM Model" << endl;
    model = svm_load_model(filename.c_str());
    if (model != NULL) {
        cout << "SVM Model Loaded" << endl;
    }
    else {
        cout << "SVM Model Load Fail, please check path." + filename << endl;
        exit(0);
    }
}

void free_model() {
    svm_free_and_destroy_model(&model);
}

vector<int> recognize_num(const CImg<unsigned char>& image, vector<ss::Rect> regions) {
    
    vector<int> recognized_num;
    const int padding = 15;
    
    for (auto& r : regions) {
        
        CImg<> ROI = cimgFromRect(image, r);
        
        //Convert to Gray
        CImg<> gray = ROI.RGBtoYCbCr().get_channel(0);
        //Binarilized
        gray.threshold(128);
        //Negate
        CImg<> neg = negative(gray);
        //Pad Image
        CImg<float> pad(neg.width() + 2*padding, neg.height() + 2*padding, 1, 1, 0);
        cimg_forXY(neg, x, y) {
            pad(x+padding, y+padding) = neg(x, y);
        }
        //Normalize
        pad.resize(28, 28, 1, 1, 3);
        //Rethreshold
        pad.threshold(0.4);
        pad.display();
        
        //Compute feature vector;
        nodeFromCImg(pad);
        
        int predicted = svm_predict(model, nodes);
        cout << "I guess this number is: " << predicted << endl;
        recognized_num.push_back(predicted);
        
    }
    
    return recognized_num;
    
}
