//
//  Contour.cpp
//  ContourPlay
//
//  Created by 王 颖豪 on 6/25/17.
//  Copyright © 2017 TSK. All rights reserved.
//

#include "Contour.hpp"
#include <stack>

namespace ct {
    
    Contour::Contour(CImg<float>& img) {
        
        //Gray Image, and binarilized is required.
        assert(img._spectrum == 1);
        
        _img.assign(img);
        _img.blur(1.3);
        
        _img.normalize(0, 1);
        _img.threshold(0.5);
        _img = 1 - _img;

        _img.display();
        
        nextLabel = 0;
        
        random_device rnd;
        mt19937 mt( rnd() );
        uniform_int_distribution<> int2w(-10000, -1);

        cimg_forXY(_img, x, y) {
            
            if (_img(x,y) == 1.0) {
                nextLabel = int2w(mt);
                
                vector<ContourNode>* contour = new vector<ContourNode>;
                traverseContour(x, y, contour);
                contours.push_back(contour);
            }
        }
        
    }
    
    void Contour::traverseContour(int x, int y, vector<ContourNode>* contour)
    {
        _img(x ,y) = nextLabel;
        
        stack<Point> traverseOrder;
        traverseOrder.push(Point(x, y));
        
        
        while (!traverseOrder.empty())
        {
            Point t = traverseOrder.top();
            traverseOrder.pop();
            
            _img(t.x, t.y) = nextLabel;
            contour->push_back(ContourNode(Point(t.x, t.y), nextLabel));
//            cout << "Adding to contour: " << contour << " x:" << t.x << " y:" << t.y << endl;
            
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    if ( t.x + i >= 0 && t.y + j >= 0 && t.x + i < _img._width && t.y + j < _img._height )
                        if (!(i == 0 && j == 0) && _img(t.x + i, t.y + j) > 0.9)
                        {
                            //                    traverseContour(x + i, y + j, contour);
                            traverseOrder.push(Point(t.x + i, t.y + j));
                        }
                }
            }
            
        }
    }

    
    vector<Rect> Contour::extractRegions() {
        
        vector<Rect> regions;
        for (int i = 0; i < contours.size(); i++) {
            vector<ContourNode>* vc = contours[i];
            
            int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;
            
            for (int j = 0; j < vc->size(); j++) {
                
                Point p = (*vc)[j].p;
                
                minX = p.x < minX ? p.x : minX;
                maxX = p.x > maxX ? p.x : maxX;
                
                minY = p.y < minY ? p.y : minY;
                maxY = p.y > maxY ? p.y : maxY;
            
            
            }
//            cout << minX << " " << minY << " " << maxX << " " << maxY << endl;
            regions.push_back(Rect(minX, minY, maxX - minX, maxY - minY));
        }
        
        
        return regions;
    }
    
}






