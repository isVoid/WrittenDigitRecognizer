//
//  TextDetection.cpp
//  A4PaperScan
//
//  Created by 王 颖豪 on 6/15/17.
//  Copyright © 2017 TSK. All rights reserved.
//

#include "TextDetection.hpp"

struct Block_1D {
    int begin;
    int end;
};

int argMax(vector<int> v) {
    if (v.empty()) return -1;
    int m = v[0];
    int mi = 0;
    
    for (int i = 0; i < v.size(); i++) {
        if (v[i] > m) {
            mi = i;
            m = v[i];
        }
    }
    return mi;
}

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

//projection

vector<int> vertical_projection(CImg<unsigned char> image_Y, Block_1D l, unsigned char threshold) {
    
    vector<int> v_stat = vector<int>(image_Y.width(), 0);
    
    for (int y = l.begin; y < l.end; y++) {
        for (int x = 0; x < image_Y.width(); x++) {
            if (image_Y(x, y) < threshold) {
                v_stat[x]++;
            }
        }
    }
    
    return v_stat;
    
}

vector<int> horizontal_projection(CImg<unsigned char> image_Y, unsigned char threshold) {
    
    vector<int> h_stat = vector<int>(image_Y.height(), 0);
    
    cimg_forXY(image_Y, x, y) {
        
        if (image_Y(x, y) < threshold) {
            h_stat[y]++;
        }
    }
    
    return h_stat;
    
}

vector<Block_1D> split_by_histogram(vector<int> v, int min_width = 10) {
    
    vector<Block_1D> blocks;
    vector<int> block_height;
    
    //smoothing
    int window_size = 30;
    for (int i = 0; i < v.size() - window_size; i++) {
        int avg = 0;
        for (int j = i; j < i+window_size; j++) {
            avg += v[j];
        }
        avg /= window_size;
        v[i] = avg;
    }
    
    vector<int> dv(v.size(), 0);
    //1st and 2nd derivatives
    for (int i = 0; i < v.size() - 1; i++) {
        dv[i] = v[i+1] - v[i];
    }
    
    int total_average_block_height = 0;
    for (int i = 0; i < dv.size() - 1; i++) {
        //local maxima i
        if (dv[i] >= 0 && dv[i+1] < 0) {
            //Determine by continue 3 0s;
            int a = i - min_width;
            while (a >= 0 && dv[a] > 0) a--;
            int b = i + min_width;
            while (b < dv.size() && dv[b] < 0) b++;
            
            int avg_block_height = 0;
            for (int i = a; i < b; i++) {
                avg_block_height += v[i];
            }
            avg_block_height /= (b - a);
            block_height.push_back(avg_block_height);
            total_average_block_height += avg_block_height;
            
            if (a >= 0 && b < v.size()) {
                blocks.push_back({a, b});
                cout << "Found block: " << a << " " << b << " " << avg_block_height << endl;
            }
            i = b;
        }
    }
    total_average_block_height /= blocks.size();
    
    cout << blocks.size() << endl;
    
    for (int i = 0; i < block_height.size();) {
        if (block_height[i] < total_average_block_height * 0.5) {
            block_height.erase(block_height.begin() + i);
            blocks.erase(blocks.begin() + i);
        }
        else {
            i++;
        }
    }
    
    return blocks;
}

void draw_histogram_line(vector<int> v) {
    
    //Sliding window smoothing
    int window_size = 10;
    for (int i = 0; i < v.size() - window_size; i++) {
        int avg = 0;
        for (int j = i; j < i+window_size; j++) {
            avg += v[j];
        }
        avg /= window_size;
        v[i] = avg;
    }
    
    int max_i = argMax(v);
    CImg<unsigned char> hist(v[max_i] + 1, (int)v.size() + 1, 1, 1, 0);
    
    for (int y = 0; y < v.size(); y++) {
        for (int x = 0; x < v[y]; x++) {
            hist(x, y) = 255;
        }
    }
    
    hist.display();
    
}

void draw_histogram_col(vector<int> v) {
    
    //Sliding window smoothing
    int window_size = 10;
    for (int i = 0; i < v.size() - window_size; i++) {
        int avg = 0;
        for (int j = i; j < i+window_size; j++) {
            avg += v[j];
        }
        avg /= window_size;
        v[i] = avg;
    }
    
    int max_i = argMax(v);
    CImg<unsigned char> hist((int)v.size(), v[max_i] + 1, 1, 1, 0);
    
    for (int x = 0; x < v.size(); x++) {
        for (int y = 0; y < v[x]; y++) {
            hist(x, v[max_i] - y) = 255;
        }
    }
    
    hist.display();
    
}

void draw_histogram_dline(vector<int> v) {
    
    int window_size = 10;
    for (int i = 0; i < v.size() - window_size; i++) {
        int avg = 0;
        for (int j = i; j < i+window_size; j++) {
            avg += v[j];
        }
        avg /= window_size;
        v[i] = avg;
    }
    
    for (int i = 0; i < v.size() - 1; i++) {
        v[i] = v[i+1] - v[i];
        cout << v[i] << " ";
    }
    cout << endl;
    
}


vector<Rect> text_projDetection(CImg<unsigned char> image) {
    
    vector<Rect> proposals;
    CImg<unsigned char> image_Y = image.get_RGBtoYCbCr().get_channel(0);
    
    //Line Split
    unsigned char threshold = 128;
    vector<int> h_stat = horizontal_projection(image_Y, threshold);
    draw_histogram_line(h_stat);
    vector<Block_1D> lines = split_by_histogram(h_stat);
    cout << lines.size() << endl;
    
    //For each line, split text regions
    for (Block_1D l : lines) {
        if (l.begin < 0 || l.end >= image.height()) {
            continue;
        }
        vector<int> v_stat = vertical_projection(image_Y, l, threshold);
        draw_histogram_col(v_stat);
        vector<Block_1D> cols = split_by_histogram(v_stat);
        
        for (Block_1D c : cols) {
            Rect r = {c.begin, l.begin, c.end-c.begin, l.end-l.begin};
            proposals.push_back(r);
        }
    }
    
    RectangleAll(image, proposals);
    
    return proposals;
    
}
