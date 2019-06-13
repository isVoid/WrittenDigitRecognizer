//
//  TextDetection.cpp
//  A4PaperScan
//
//  Created by 王 颖豪 on 6/15/17.
//  Copyright © 2017 TSK. All rights reserved.
//

#include "TextDetection.hpp"

bool rectLRTB(const Rect& a, const Rect& b) {
//    float same_line_criteria = 10;
//    if (abs(a.y - b.y) < same_line_criteria) {
//        return a.x < b.x;
//    }
//    else {
//        return a.y < b.x;
//    }
//    return false;
    return tie(a.y, a.x) < tie(b.y, b.x);
}

vector<Rect> text_detection(CImg<> image) {
 
    CImg<float> tempImage = image.get_RGBtoYCbCr().get_channel(0);
    ct::Contour contours(tempImage);
    vector<ct::Rect> proposals = contours.extractRegions();
    
//    vector<Rect> proposals = ss::selectiveSearch(image, 500, 3, 25, 200, 5000, 5);
    
    filterBySize(image, proposals);
    
//    RectangleAll(image, proposals);
    
    filterByAspectRatio(proposals);
    
//    RectangleAll(image, proposals);
    
    filterByDuplicate(proposals);
    
//    RectangleAll(image, proposals);
    
//    sort(proposals.begin(), proposals.end(), rectLRTB);
    ScanLineDet sld(image._height);
    ct::vector<Rect> sorted = sld.getSorted(proposals);
    
    padRegion(image, sorted, 5);
    
    RectangleAll(image, sorted);
    
    return sorted;
}

void filterBySize(const CImg<>& image, vector<Rect>& proposals) {
    
    int size = image._width * image._height;
    float size_thres_lower = 0.0005;
    float size_thres_upper = 0.2;
    
    cout << "Threashold Size: " << size * size_thres_lower << " " << size * size_thres_upper << endl;
    
    proposals.erase(remove_if(proposals.begin(), proposals.end(), [size, size_thres_lower, size_thres_upper](const Rect r){
//        cout << r.width * r.height << endl;
        return r.width * r.height < size * size_thres_lower || r.width * r.height > size * size_thres_upper ;
    }), proposals.end());
    
}

void filterByAspectRatio(vector<Rect>& p) {
    
    p.erase(remove_if(p.begin(), p.end(), [](const Rect& r){ return float(r.width) / r.height > 4; }), p.end());
    p.erase(remove_if(p.begin(), p.end(), [](const Rect& r){ return float(r.height) / r.width > 10; }), p.end());
    
}

void filterByDuplicate(vector<Rect>& p) {
    
    vector<size_t> duplicates;
    for (int i = 0; i < p.size(); i++) {
        
        int responses = 0;
        
        for (int j = 0; j < p.size(); j++) {
            
            Rect intersect = (p[i] & p[j]);
            if (intersect == p[i]) {
                responses++;
            }
        }
        
        if (responses > 1) {
            duplicates.push_back(i);
        }
    }
    
    if (!duplicates.empty()) {
        sort(duplicates.begin(), duplicates.end());
        
        size_t count = 0;
        size_t dup_ptr = 0;
        p.erase(remove_if(p.begin(), p.end(), [&count, &dup_ptr, duplicates](const Rect& r){
            if (dup_ptr > duplicates.size())
                return false;
            bool d = duplicates[dup_ptr] == count;
            count++;
            if (d) {
                dup_ptr++;
            }
            return d;
        }), p.end());
    }
    
}

void padRegion(const CImg<>& image, vector<Rect>& p, int padding) {
    
    for (auto& r : p) {
        
        if (r.x != -1)
        {
            //        cout << r.x << " " << r.y << " " << r.width << " " << r.height << endl;
            
            r.x = r.x - padding < 0 ? 0 : r.x - padding;
            r.y = r.y - padding < 0 ? 0 : r.y - padding;
            
            r.width = r.x + r.width + padding > image.width() ?
            image.width() - r.x :
            r.width + padding;
            
            r.height = r.y + r.height + padding > image.height() ?
            image.height() - r.y :
            r.height + padding;
            
            //        cout << r.x << " " << r.y << " " << r.width << " " << r.height << endl;
        }
        
    }
    
}




