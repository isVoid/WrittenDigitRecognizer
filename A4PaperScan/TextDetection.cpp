//
//  TextDetection.cpp
//  A4PaperScan
//
//  Created by 王 颖豪 on 6/15/17.
//  Copyright © 2017 TSK. All rights reserved.
//

#include "TextDetection.hpp"

bool rectLRTB(const ss::Rect& a, const ss::Rect& b) {
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

vector<ss::Rect> text_detection(CImg<> image) {
 
    vector<ss::Rect> proposals = ss::selectiveSearch(image, 500, 3, 25, 200, 5000, 5);
    
    filterBySize(image, proposals);
    
//    RectangleAll(image, proposals);
    
    filterByAspectRatio(proposals);
    
//    RectangleAll(image, proposals);
    
    filterByDuplicate(proposals);
    
//    RectangleAll(image, proposals);
    
    sort(proposals.begin(), proposals.end(), rectLRTB);
    
    padRegion(image, proposals, 5);
    
    RectangleAll(image, proposals);
    
    return proposals;
}

void filterBySize(const CImg<>& image, vector<ss::Rect>& proposals) {
    
    int size = image._width * image._height;
    float size_threshold = 0.025;
//    cout << "Threashold Size: " << size * size_threshold << endl;
    
    proposals.erase(remove_if(proposals.begin(), proposals.end(), [size, size_threshold](const ss::Rect r){
//        cout << r.width * r.height << endl;
        return r.width * r.height > size * size_threshold;
    }), proposals.end());
    
}

void filterByAspectRatio(vector<ss::Rect>& p) {
    
    p.erase(remove_if(p.begin(), p.end(), [](const ss::Rect& r){ return float(r.width) / r.height > 3; }), p.end());
    
}

void filterByDuplicate(vector<ss::Rect>& p) {
    
    vector<size_t> duplicates;
    for (int i = 0; i < p.size(); i++) {
        
        int responses = 0;
        
        for (int j = 0; j < p.size(); j++) {
            
            ss::Rect intersect = (p[i] & p[j]);
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
        p.erase(remove_if(p.begin(), p.end(), [&count, &dup_ptr, duplicates](const ss::Rect& r){
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

void padRegion(const CImg<>& image, vector<ss::Rect>& p, int padding) {
    
    for (auto& r : p) {
        
        cout << r.x << " " << r.y << " " << r.width << " " << r.height << endl;
        
        r.x = r.x - padding < 0 ? 0 : r.x - padding;
        r.y = r.y - padding < 0 ? 0 : r.y - padding;
        
        r.width = r.x + r.width + padding > image.width() ?
            image.width() - r.x :
                r.width + padding;
        
        r.height = r.y + r.height + padding > image.height() ?
            image.height() - r.y :
                r.height + padding;
        
        cout << r.x << " " << r.y << " " << r.width << " " << r.height << endl;
        
    }
    
}




