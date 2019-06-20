//
//  Contour.hpp
//  ContourPlay
//
//  Created by 王 颖豪 on 6/25/17.
//  Copyright © 2017 TSK. All rights reserved.
//

#ifndef Contour_hpp
#define Contour_hpp

#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <random>
#include "CImg.h"

namespace ct {
    using namespace cimg_library;
    using namespace std;
    
    struct Point
    {
        int x;
        int y;
        
        Point (int _x = 0, int _y = 0) : x(_x), y(_y) {}
        
        bool operator>( const Point& other) {
            return x > other.x && y > other.y;
        }
    };
    
    
    struct Rect
    {
        int x;
        int y;
        int width;
        int height;
        
        Rect () : x(0), y(0), width(0), height(0) {}
        Rect ( int _x, int _y, int _w, int _h ) : x(_x), y(_y), width(_w), height(_h) {}
        Rect ( Point tl, Point br ) : x(tl.x), y(tl.y), width(br.x-tl.x), height(br.y-tl.y) {}
        
        void set(Point tl, Point br) {
            x = tl.x;
            y = tl.y;
            width = br.x - tl.x;
            height = br.y - tl.y;
        }
        
        float area() {
            return width * height;
        }
        //Return the bottom right point
        Point br() const    //Telling compiler that this function will not modify the class.
        {
            return (Point(x + width, y + height));
        }
        
        friend Rect operator|(const Rect& a, const Rect& b) {
            
            Point tl(
                     std::min(a.x, b.x),
                     std::min(a.y, b.y)
                     );
            
            Point br(
                     std::max(a.br().x, b.br().x),
                     std::max(a.br().y, b.br().y)
                     );
            
            Rect unionRect(tl, br);
            
            return unionRect;
        }
        
        friend Rect operator&(const Rect& a, const Rect& b) {
            
            Point tl(
                     std::min(a.x, b.x),
                     std::min(a.y, b.y)
                     );
            
            Point br(
                     std::max(a.br().x, b.br().x),
                     std::max(a.br().y, b.br().y)
                     );
            
            Rect intersectRect;
            if (br > tl) {
                intersectRect.set(tl, br);
            }
            return intersectRect;
        }
        
        friend bool operator==(const Rect& a, const Rect& b) {
            return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
        }
    };
    
    class ContourNode{
        
    public:
        Point p;
        float label;

        ContourNode () {}
        ContourNode(Point _p, float _l):p(_p),label(_l) {}
        
    };
    
    
    class Contour {
        
    private:
        CImg<float> _img;
        CImg<float> _map;
        
        vector<vector<ContourNode>*> contours;
        
        int nextLabel;
        
    private:
        void traverseContour(int x, int y, vector<ContourNode>* contour);
        
    public:
        
        //Gray Image, and binarilized is required.
        Contour(CImg<float>& img);
        
        vector<Rect> extractRegions();
        
    };
    
}

#endif /* Contour_hpp */














