//
//  TextRecognition.cpp
//  DigitScanner
//
//  Created by 王 颖豪 on 6/16/17.
//  Copyright © 2017 TSK. All rights reserved.
//

#include "TextRecognition.hpp"

svm_model* model;
svm_node* nodes;

Model tf_model;

CImg<> cimgFromRect(const CImg<>& image, Rect region) {
    
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
    
    string filename = "../../../model/mnist.model";
    
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

vector<int> recognize_num(const CImg<unsigned char>& image, vector<Rect> regions) {
    
    vector<int> recognized_num;
    const int padding = 15;
    
    for (auto& r : regions) {
        
//        cout << r.x << endl;
        if (r.x != -1)
        {
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
            
//            CImg<float> fitResize(28, 28, 1, 1, 0);
//            if (pad.width() > pad.height())
//            {
//                float f = 28. / pad.width();
//                pad.resize(28, pad.height() * f, 1, 1, 3);
//                
//                int offsetY = (28 - pad.height()) / 2;
//                
//                cimg_forXY(pad, x, y)
//                {
//                    fitResize(x, y + offsetY) = pad(x, y);
//                }
//                
//            }
//            else
//            {
//                float f = 28. / pad.height();
//                pad.resize(pad.width() * f, 28, 1, 1, 3);
//                
//                int offsetX = (28 - pad.width()) / 2;
//                
//                cimg_forXY(pad, x, y)
//                {
//                    fitResize(x + offsetX, y) = pad(x, y);
//                }
//            }
//            fitResize.threshold(0.4);
            
            
            //Normalize
            pad.resize(28, 28, 1, 1, 3);
            //Rethreshold
            pad.threshold(0.65);
            
            
            //Compute feature vector;
            nodeFromCImg(pad);
//            nodeFromCImg(fitResize);
            
            int predicted = svm_predict(model, nodes);
//            cout << "I guess this number is: " << predicted << endl;
//            fitResize.display();
//            pad.display();
            cout << predicted << " ";
            recognized_num.push_back(predicted);

        }
        else
        {
            cout << endl;
        }
        
    }
    
    cout << endl;
    return recognized_num;
    
}

void load_tfmodel() {
    
    const char* graph_path = "./model/graph.pb";
    const char* checkpoint_path = "./model/mnist_-94000";
    
    tf_model.load_graph(graph_path);
    tf_model.create_session();
    tf_model.restore_weights(checkpoint_path);
    
}

vector<int> tfrecognize_num(const CImg<unsigned char>& image, vector<Rect> regions) {
    
//    CImg<float> img_f = image.get_normalize(0, 1);
    vector<int> recognized_num;
    
    tf_model.create_output("dense2/BiasAdd", std::vector<float>(10, 0.), {10});
    
    for (auto& r : regions) {
        
        if (r.x != -1)
        {
            CImg<> ROI = cimgFromRect(image, r);
            
            //Convert to Gray
            CImg<> gray = ROI.RGBtoYCbCr().get_channel(0);
            //Binarilized
            gray.threshold(128);
            //Negate
            CImg<> neg = negative(gray);
            //Pad Image
            int padding = 10;
            CImg<float> pad(neg.width() + 2*padding, neg.height() + 2*padding, 1, 1, 0);
            cimg_forXY(neg, x, y) {
                pad(x+padding, y+padding) = neg(x, y);
            }
            
            //Normalize
            pad.resize(28, 28, 1, 1, 3);
            //Rethreshold
            pad.threshold(0.65);

//            pad.display();
            
            
            tf_model.clear_input();
            tf_model.create_input("input_image", vector<float>(pad.data(),
                                                               pad.data() + pad.width() * pad.height() * sizeof(float)),
                                                                {1, 28, 28, 1});
            
            int predicted = tf_model.predict();
            cout << predicted << " ";
            
            recognized_num.push_back(predicted);
            
        }
        else
            cout << endl;
        
    }
    
    cout << endl;
    
    return recognized_num;
    
}
