//
//  ScanLineDetermination.hpp
//  A4PaperScan
//
//  Created by 王 颖豪 on 7/8/17.
//  Copyright © 2017 TSK. All rights reserved.
//

#ifndef ScanLineDetermination_hpp
#define ScanLineDetermination_hpp

#include "Contour.hpp"
#include <vector>
#include <algorithm>
#include "CImg.h"

using namespace cimg_library;
using namespace ct;

struct RectWLine
{
    int line;
    Rect rect;
};

inline bool rectSort(const RectWLine& r1, const RectWLine& r2)
{
    if (r1.line < r2.line)
        return true;
    else if (r1.line > r2.line)
        return false;
    else if (r1.rect.x < r2.rect.x)
        return true;
    else
        return false;
}

class ScanLineDet
{
public:
    ScanLineDet(int h) : height(h) {}
    
    vector<Rect> getSorted(vector<Rect> regions)
    {
        
        vector<int> lineRectHist;
        for (int i = 0; i < height; i++)
        {
            int vote = 0;
            for (auto& r : regions)
            {
//                cout << i << endl;
//                cout << r.y << " " << r.height << endl;
                if (r.y <= i && r.y + r.height >= i)
                    vote++;
            }
//            cout << vote << endl;
            lineRectHist.push_back(vote);
        }
//        for (int i = 0 ; i < lineRectHist.size(); i++)
//        {
//            cout << lineRectHist[i] << endl;
//        }
        
        
        vector<int> dLineRectHist;
        dLineRectHist.push_back(-1);
        for (int i = 1; i < lineRectHist.size(); i++)
        {
            dLineRectHist.push_back(lineRectHist[i] - lineRectHist[i-1]);
        }

//        vector<int> lohipair;
//        bool pairStart = false;
//        for (int i = 1; i < lineRectHist.size()-1; i++)
//        {
//            if (lineRectHist[i] > lineRectHist[i-1] && dLineRectHist[i] == 0 && !pairStart) {
//                lohipair.push_back(i);
//                pairStart = true;
//            }
//            
//            if (lineRectHist[i] < lineRectHist[i+1] && dLineRectHist[i] == 0 && pairStart) {
//                lohipair.push_back(i);
//                pairStart = false;
//            }
//
//        }
        
        vector<int> medPair;
        int lo = -1, hi = -1;
        for (int i = 0 ; i < dLineRectHist.size(); i++)
        {
            
            if (dLineRectHist[i] > 0)
            {
                lo = i;
            }
            
            if (dLineRectHist[i] < 0 && lo != -1)
            {
                hi = i;
                medPair.push_back((lo + hi) / 2);
//                cout << "Line: " << (lo + hi) / 2 << endl;
                lo = -1;
                hi = -1;
            }
        }
        
        
//        for (int i = 0; i + 1 < lohipair.size(); i++) {
//            
//            int line = (lohipair[i] + lohipair[i+1]) / 2.;
//            medPair.push_back(line);
//            cout << line << endl;
//        }
        
        vector<RectWLine> rectLinePairs;
        for (int i = 0; i < medPair.size(); i++)
        {
            for (int j = 0; j < regions.size(); j++)
            {
                if (regions[j].y <= medPair[i] && regions[j].y + regions[j].height >= medPair[i])
                {
                    RectWLine r;
                    r.line = medPair[i];
                    r.rect = regions[j];
//                    cout << "LineRect: " << r.line << " "  << r.rect.x << " " << r.rect.y << endl;
                    rectLinePairs.push_back(r);
                }
            }
        }
        
        //Sort By Line Num;
        sort(rectLinePairs.begin(), rectLinePairs.end(), rectSort);
        
        vector<Rect> sorted;
        int lastLine = -1;
        for (auto rlp : rectLinePairs)
        {
            if (lastLine == -1)
            {
                lastLine = rlp.line;
            }
            else
            {
                if (lastLine != rlp.line)
                {
                    Rect delim(-1, -1, -1, -1);
//                    delim.x = -1;
//                    delim.y = -1;
//                    cout << "Delim x: " << delim.x << endl;
                    sorted.push_back(delim);
                    
                    lastLine = rlp.line;
                }
                
            }
            sorted.push_back(rlp.rect);
            
        }
        
        return sorted;
    }
    
    int height;
};

#endif /* ScanLineDetermination_hpp */
