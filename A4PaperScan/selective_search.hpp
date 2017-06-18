#pragma once


#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

#include "headers.h"

static const float PI = 3.141592654;

namespace std
{
	template<>
	class hash<std::pair<int, int>>
	{
	public:
		std::size_t operator()( const std::pair<int, int> &x ) const
		{
			return hash<int>()( x.first ) ^ hash<int>()( x.second );
		}
	};
}



namespace ss
{
    using namespace cimg_library;
    
	inline double square( double a )
	{
		return a*a;
	}

    //Compute L2 Distance on RGB channel
	inline double diff( const CImg<float> &img, int x1, int y1, int x2, int y2 )
	{
        return sqrt( square( img(x1, y1, 0) - img(x2, y2, 0) ) +
                    square( img(x1, y1, 1) - img(x2, y2, 1) +
                           square( img(x1, y1, 2) - img(x2, y2, 2) )));
	}


	struct UniverseElement
	{
		int rank;
		int p;
		int size;

		UniverseElement() : rank( 0 ), size( 1 ), p( 0 ) {}
		UniverseElement( int rank, int size, int p ) : rank( rank ), size( size ), p( p ) {}
	};


	class Universe
	{
	private:
		std::vector<UniverseElement> elements;
		int num;

	public:
		Universe( int num ) : num( num )
		{
			elements.reserve( num );

			for ( int i = 0; i < num; i++ )
			{
				elements.emplace_back( 0, 1, i );
			}
		}

		~Universe() {}

		int find( int x )
		{
			int y = x;
			while ( y != elements[y].p )
			{
				y = elements[y].p;
			}
			elements[x].p = y;

			return y;
		}

		void join( int x, int y )
		{
			if ( elements[x].rank > elements[y].rank )
			{
				elements[y].p = x;
				elements[x].size += elements[y].size;
			}
			else
			{
				elements[x].p = y;
				elements[y].size += elements[x].size;
				if ( elements[x].rank == elements[y].rank )
				{
					elements[y].rank++;
				}
			}
			num--;
		}

		int size( int x ) const { return elements[x].size; }
		int numSets() const { return num; }
	};

    //Element edge, each edge borders two elements a and b
    //The weight is computed as the L2 distance of element a and b
	struct edge
	{
		int a;
		int b;
		double w;
	};

    struct Color
    {
        unsigned char R;
        unsigned char G;
        unsigned char B;
        
        Color(unsigned char _r, unsigned char _g, unsigned char _b) {
            R = _r; G = _g; B = _b;
        }
    };
             
    struct Point
    {
        int x;
        int y;
        
        Point (int _x, int _y) : x(_x), y(_y) {}
        
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

	inline bool operator<( const edge &a, const edge &b )
	{
		return a.w < b.w;
	}


	inline double calThreshold( int size, double scale )
	{
		return scale / size;
	}

    
	inline std::shared_ptr<Universe> segmentGraph( int numVertices, int numEdges, std::vector<edge> &edges, double scale )
	{
        //Sort element edges based on their weights
		std::sort( edges.begin(), edges.end() );
        
        //An auto-release pointer to a newly constructed Universe object
		auto universe = std::make_shared<Universe>( numVertices );

        //size numVertices, filled with scale value
		std::vector<double> threshold( numVertices, scale );

		for ( auto &pedge : edges )
		{
            //Find the segment of a and b
			int a = universe->find( pedge.a );
			int b = universe->find( pedge.b );

            //If a and b is not the same segment
			if ( a != b )
			{
                //If this edge's inter-class difference is lower than the inner-class difference on both side of the edge:
				if ( ( pedge.w <= threshold[a] ) && ( pedge.w <= threshold[b] ) )
				{
                    //Join these two segments
					universe->join( a, b );
					a = universe->find( a );
                    //Update the inner class difference: old-inter-class difference plus a Tolerance factor
                    //The tolerance factor is computed as a constant (scale here) divided by the size of the
                    //segment, the larger the segement, the lower the tolerance factor.
					threshold[a] = pedge.w + calThreshold( universe->size( a ), scale );
				}
			}
		}

		return universe;
	}


    inline void genGaussian(CImg<float>& kernal, float sigma) {
        
        assert (kernal._spectrum == 1);
        
        int xCen = kernal._width;
        int yCen = kernal._height;
        
        for (int x = 0; x < kernal._width; x++) {
            for (int y = 0; y < kernal._width; y++) {
                
                int xNorm = x - xCen;
                int yNorm = y - yCen;
                
                kernal(x, y) = 1 / (2 * PI * square(sigma))
                    * std::exp(-(square(xNorm) + square(yNorm)) /
                               (2 * square(sigma)));
                
            }
        }
        
        kernal.normalize(0, 1);
    }
    
	// image segmentation using "Efficient Graph-Based Image Segmentation"
	inline std::shared_ptr<Universe> segmentation( const CImg<float> &img, double scale, double sigma, int minSize )
	{
		const int width = img._width;
		const int height = img._height;

        CImg<float> blurred = img.get_blur(sigma);
//        CImg<float> kernal(5, 5);
//        genGaussian(kernal, sigma);
        
//        CImg<float> blurred = img.get_convolve(kernal);
        
		std::vector<edge> edges( width*height * 4 );

        //Initialize Edges Vector, which stores the current edge for elements.
		int num = 0;
		for ( int y = 0; y < height; y++ )
		{
			for ( int x = 0; x < width; x++ )
			{
                //East neighbour
				if ( x < width - 1 )
				{
					edges[num].a = y * width + x;
					edges[num].b = y * width + ( x + 1 );
					edges[num].w = diff( blurred, x, y, x + 1, y );
					num++;
				}
                
                //South neighbor
				if ( y < height - 1 )
				{
					edges[num].a = y * width + x;
					edges[num].b = ( y + 1 ) * width + x;
					edges[num].w = diff( blurred, x, y, x, y + 1 );
					num++;
				}

                //South East neighbor
				if ( ( x < width - 1 ) && ( y < height - 1 ) )
				{
					edges[num].a = y * width + x;
					edges[num].b = ( y + 1 ) * width + ( x + 1 );
					edges[num].w = diff( blurred, x, y, x + 1, y + 1 );
					num++;
				}

                //North East neighbor
				if ( ( x < width - 1 ) && ( y > 0 ) )
				{
					edges[num].a = y * width + x;
					edges[num].b = ( y - 1 ) * width + ( x + 1 );
					edges[num].w = diff( blurred, x, y, x + 1, y - 1 );
					num++;
				}
			}
		}

		auto universe = segmentGraph( width*height, num, edges, scale );

        //Iterate through the segmented list, if any segments are still smaller than
        //the minSize parameter, force join.
		for ( int i = 0; i < num; i++ )
		{
			int a = universe->find( edges[i].a );
			int b = universe->find( edges[i].b );
			if ( ( a != b ) && ( ( universe->size( a ) < minSize ) || ( universe->size( b ) < minSize ) ) )
			{
				universe->join( a, b );
			}
		}

		return universe;
	}

                    
	inline void visualize( const CImg<float> &img, std::shared_ptr<Universe> universe )
	{
        
		const int height = img._height;
		const int width = img._width;
		std::vector<Color> colors;

		CImg<unsigned char> segmentated( width, height, 1, 3 );

		std::random_device rnd;
		std::mt19937 mt( rnd() );
		std::uniform_int_distribution<> rand256( 0, 255 );

		for ( int i = 0; i < height*width; i++ )
		{
			Color color( rand256( mt ), rand256( mt ), rand256( mt ) );
			colors.push_back( color );
		}

		for ( int y = 0; y < height; y++ )
		{
			for ( int x = 0; x < width; x++ )
			{
                segmentated(x, y, 0) = colors[universe->find( y*width + x )].R;
                segmentated(x, y, 1) = colors[universe->find( y*width + x )].G;
                segmentated(x, y, 2) = colors[universe->find( y*width + x )].B;
//				segmentated.at<cv::Vec3b>( y, x ) = colors[universe->find( y*width + x )];
			}
		}

        segmentated.display();
	}


	struct Region
	{
		int size;
		Rect rect;
		std::vector<int> labels;
		std::vector<float> colourHist;
		std::vector<float> textureHist;

		Region() {}

		Region( const Rect &rect, int label ) : rect( rect )
		{
			labels.push_back( label );
		}

		Region(
			const Rect &rect, int size,
			const std::vector<float> &&colourHist,
			const std::vector<float> &&textureHist,
			const std::vector<int> &&labels
			)
			: rect( rect ), size( size ), colourHist( std::move( colourHist ) ), textureHist( std::move( textureHist ) ), labels( std::move( labels ) )
		{}

		Region& operator=( const Region& region ) = default;

		Region& operator=( Region&& region ) noexcept
		{
			if ( this != &region )
			{
				this->size = region.size;
				this->rect = region.rect;
				this->labels = std::move( region.labels );
				this->colourHist = std::move( region.colourHist );
				this->textureHist = std::move( region.textureHist );
			}

			return *this;
		}

		Region( Region&& region ) noexcept
		{
			*this = std::move( region );
		}
	};


	inline std::shared_ptr<Universe> generateSegments( const CImg<float> &img, double scale, double sigma, int minSize )
	{
		auto universe = segmentation( img, scale, sigma, minSize );

		visualize( img, universe );

		return universe;
	}


	inline double calcSimOfColour( const Region &r1, const Region &r2 )
	{
		assert( r1.colourHist.size() == r2.colourHist.size() );

		float sum = 0.0;

		for ( auto i1 = r1.colourHist.cbegin(), i2 = r2.colourHist.cbegin(); i1 != r1.colourHist.cend(); i1++, i2++ )
		{
			sum += std::min( *i1, *i2 );
		}

		return sum;
	}


	inline double calcSimOfTexture( const Region &r1, const Region &r2 )
	{
		assert( r1.colourHist.size() == r2.colourHist.size() );

		double sum = 0.0;

		for ( auto i1 = r1.textureHist.cbegin(), i2 = r2.textureHist.cbegin(); i1 != r1.textureHist.cend(); i1++, i2++ )
		{
			sum += std::min( *i1, *i2 );
		}

		return sum;
	}


	inline double calcSimOfSize( const Region &r1, const Region &r2, int imSize )
	{
		return ( 1.0 - ( double )( r1.size + r2.size ) / imSize );
	}


	inline double calcSimOfRect( const Region &r1, const Region &r2, int imSize )
	{
		return ( 1.0 - ( double )( ( r1.rect | r2.rect ).area() - r1.size - r2.size ) / imSize );
	}


	inline double calcSimilarity( const Region &r1, const Region &r2, int imSize )
	{
		return ( calcSimOfColour( r1, r2 ) + calcSimOfTexture( r1, r2 ) + calcSimOfSize( r1, r2, imSize ) + calcSimOfRect( r1, r2, imSize ) );
	}


    //What's the type of img??
	inline std::vector<float> calcColourHist( const CImg<float> &img, std::shared_ptr<Universe> universe, int label )
	{
        //What's the use of hsv??
		std::array<std::vector<unsigned char>, 3> hsv;

		for ( auto &e : hsv )
		{
//			e.reserve( img.total() );
            e.reserve( img._width * img._height  );
		}

		for ( int y = 0; y < img._height; y++ )
		{
			for ( int x = 0; x < img._width; x++ )
			{
                //Why Find??
				if ( universe->find( y*img._width + x ) != label )
				{
					continue;
				}

				for ( int channel = 0; channel < 3; channel++ )
				{
//					hsv[channel].push_back( img.at<cv::Vec3b>( y, x )[channel] );
                    hsv[channel].push_back( img(x, y, channel) );
				}
			}
		}

		int channels[] = { 0 };
		const int bins = 25;
		int histSize[] = { bins };
		float range[] = { 0, 256 };
		const float *ranges[] = { range };

		std::vector<float> features;
        
        int binWidth = range[1] / bins;
        for (int channel = 0; channel < 3; channel++) {
            
            std::vector<unsigned char> ch = hsv[channel];
            
            std::vector<float> hist(bins);  //Init vector with size of bins and reserve with zeros
            int max = INT_MIN;
            
            for ( int i = 0; i < ch.size(); i++ ) {
                int bin = ch[i] % binWidth;
                hist[bin]++;
                max = hist[bin] > max ? hist[bin] : max;
            }
            
            //Normalize, needs check
            for ( int i = 0; i < hist.size(); i++) {
                hist[i] /= max;
            }
            
            //Concatnate
            if ( features.empty() )
            {
                features = std::move( hist );
            }
            else
            {
                std::copy( hist.begin(), hist.end(), std::back_inserter( features ) );
            }
        }
        
		return features;
	}


	inline int calcSize( const CImg<float> &img, std::shared_ptr<Universe> universe, int label )
	{
		int num = 0;

		for ( int y = 0; y < img._height; y++ )
		{
			for ( int x = 0; x < img._width; x++ )
			{
				if ( universe->find( y * img._width + x ) == label )
				{
					num++;
				}
			}
		}

		return num;
	}

    inline CImgList<> cartToPolar( const CImg<float>& x, const CImg<float>& y )
    {
        assert(x._width == y._width && x._height == y._height);
        
        CImg<float> magnitude(x._width, x._height, 1, x._spectrum);
        CImg<float> angle(x._width, x._height, 1, x._spectrum);
        
        cimg_forC(magnitude, _c_) {
            cimg_forXY(magnitude, _x_, _y_) {
                
                magnitude(_x_, _y_, _c_) = sqrt(square(x(_x_, _y_, _c_) + square(y(_x_, _y_, _c_))));
                angle(_x_, _y_, _c_) = atanf(y(_x_, _y_, _c_) / x(_x_, _y_, _c_)) * 57.2958;    //To degrees
                
            }
        }
        
        CImgList<float> polar(magnitude, angle);
        return polar;
    }

	inline CImg<float> calcTextureGradient( const CImg<float> &img )
	{
        
        CImg<float> sobelX = img.get_gradient("x", 2)(0).get_normalize(0, 1);
        CImg<float> sobelY = img.get_gradient("y", 2)(0).get_normalize(0, 1);
        
        return cartToPolar(sobelX, sobelY)(1);
	}

    
	inline std::vector<float> calcTextureHist( const CImg<float> &img, const CImg<float> &gradient, std::shared_ptr<Universe> universe, int label )
	{
		const int orientations = 8;

        CImg<unsigned char> ubImg = img.get_normalize(0, 255);
        
		std::array<std::array<std::vector<unsigned char>, orientations>, 3> intensity;

		for ( auto &e : intensity )
		{
			for ( auto &ee : e )
			{
                ee.reserve( img._width * img._height );
			}
		}

		for ( int y = 0; y < img._height; y++ )
		{
			for ( int x = 0; x < img._width; x++ )
			{
				if ( universe->find( y * img._width + x ) != label )
				{
					continue;
				}

				for ( int channel = 0; channel < 3; channel++ )
				{
                    int angle = ( int )( gradient(x, y, channel) / 22.5 ) % orientations;
                    intensity[channel][angle].push_back( ubImg(x, y, channel) );
				}
			}
		}

		int channels[] = { 0 };
		const int bins = 10;
		int histSize[] = { bins };
		float range[] = { 0, 256 };
		const float *ranges[] = { range };

		std::vector<float> features;

        int binWidth = range[1] / bins;
        for (int channel = 0; channel < 3; channel++)
        {
            for (int angle = 0; angle < orientations; angle++)
            {
                
                std::vector<unsigned char> I = intensity[channel][angle];
                std::vector<float> histogram(bins); //alloc 10 cells for histogram and pop with 0
                int max = INT_MIN;
                
                //Binned Histogram
                for ( int i = 0; i < I.size(); i++ ) {
                    
                    int bin = I[i] / binWidth;
                    histogram[bin]++;
                    
                    max = histogram[bin] > max ? histogram[bin] : max;
                }
                
                //Normalize
                for ( int i = 0; i < histogram.size(); i++ ) {
                    histogram[i] /= max;
                }
                
                //Concatnate
                if ( features.empty() )
                {
                    features = std::move( histogram );
                }
                else
                {
                    std::copy( histogram.begin(), histogram.end(), std::back_inserter( features ) );
                }
            }
        }

		return features;
	}


	inline std::map<int, Region> extractRegions( const CImg<float> &img, std::shared_ptr<Universe> universe )
	{
		std::map<int, Region> R;

		for ( int y = 0; y < img._height; y++ )
		{
			for ( int x = 0; x < img._width; x++ )
			{
                //Get Label for current pixel
				int label = universe->find( y*img._width + x );

                //If current label is not enlisted, add to it.
				if ( R.find( label ) == R.end() )
				{
					R[label] = Region( Rect( 100000, 100000, 0, 0 ), label );
				}
                
                //Expand this region, to the top-left bound.
				if ( R[label].rect.x > x )
				{
					R[label].rect.x = x;
				}

				if ( R[label].rect.y > y )
				{
					R[label].rect.y = y;
				}

                //Expand this region, to the bottom-right bound.
				if ( R[label].rect.br().x < x )
				{
					R[label].rect.width = x - R[label].rect.x + 1;
				}

				if ( R[label].rect.br().y < y )
				{
					R[label].rect.height = y - R[label].rect.y + 1;
				}
			}
		}

		CImg<float> gradient = calcTextureGradient( img );

		CImg<float> hsv = img.get_RGBtoHSV();

		for ( auto &labelRegion : R )
		{
			labelRegion.second.size = calcSize( img, universe, labelRegion.first );
			labelRegion.second.colourHist = calcColourHist( hsv, universe, labelRegion.first );
			labelRegion.second.textureHist = calcTextureHist( img, gradient, universe, labelRegion.first );
		}

		return R;
	}


	inline bool isIntersecting( const Region &a, const Region &b )
	{
		return ( ( a.rect & b.rect ).area() != 0 );
	}


	using LabelRegion = std::pair<int, Region>;
	using Neighbour = std::pair<int, int>;


	inline std::vector<Neighbour> extractNeighbours( const std::map<int, Region> &R )
	{
		std::vector<Neighbour> neighbours;
		neighbours.reserve( R.size()*( R.size() - 1 ) / 2 );

		for ( auto a = R.cbegin(); a != R.cend(); a++ )
		{
			auto tmp = a;
			tmp++;

			for ( auto b = tmp; b != R.cend(); b++ )
			{
				if ( isIntersecting( a->second, b->second ) )
				{
					neighbours.push_back( std::make_pair( std::min( a->first, b->first ), std::max( a->first, b->first ) ) );
				}
			}
		}

		return neighbours;
	}


	inline std::vector<float> merge( const std::vector<float> &a, const std::vector<float> &b, int asize, int bsize )
	{
		std::vector<float> newVector;
		newVector.reserve( a.size() );

		for ( auto ai = a.begin(), bi = b.begin(); ai != a.end(); ai++, bi++ )
		{
			newVector.push_back( ( ( *ai ) * asize + ( *bi ) * bsize ) / ( asize + bsize ) );
		}

		return newVector;
	};


	inline Region mergeRegions( const Region &r1, const Region &r2 )
	{
		assert( r1.colourHist.size() == r2.colourHist.size() );
		assert( r1.textureHist.size() == r2.textureHist.size() );

		int newSize = r1.size + r2.size;

		std::vector<int> newLabels( r1.labels );
		std::copy( r2.labels.begin(), r2.labels.end(), std::back_inserter( newLabels ) );

		return Region( r1.rect | r2.rect,
			newSize,
			std::move( merge( r1.colourHist, r2.colourHist, r1.size, r2.size ) ),
			std::move( merge( r1.textureHist, r2.textureHist, r1.size, r2.size ) ),
			std::move( newLabels )
			);
	}


	inline std::vector<Rect> selectiveSearch( const CImg<float> &img, double scale = 1.0, double sigma = 0.8, int minSize = 50, int smallest = 1000, int largest = 270000, double distorted = 5.0 )
	{
		assert( img._spectrum == 3 );

        //Efficient Graph based image segmentation, IJCV 2004
		auto universe = generateSegments( img, scale, sigma, minSize );

        int imgSize = img._width * img._height;

		auto R = extractRegions( img, universe );

		auto neighbours = extractNeighbours( R );

		std::unordered_map<std::pair<int, int>, double> S;

		for ( auto &n : neighbours )
		{
			S[n] = calcSimilarity( R[n.first], R[n.second], imgSize );
		}

		using NeighbourSim = std::pair<std::pair<int, int>, double >;

		while ( !S.empty() )
		{
			auto cmp = []( const NeighbourSim &a, const NeighbourSim &b ) { return a.second < b.second; };

			auto m = std::max_element( S.begin(), S.end(), cmp );

			int i = m->first.first;
			int j = m->first.second;
			auto ij = std::make_pair( i, j );

			int t = R.rbegin()->first + 1;
			R[t] = mergeRegions( R[i], R[j] );

			std::vector<std::pair<int, int>> keyToDelete;

			for ( auto &s : S )
			{
				auto key = s.first;

				if ( ( i == key.first ) || ( i == key.second ) || ( j == key.first ) || ( j == key.second ) )
				{
					keyToDelete.push_back( key );
				}
			}

			for ( auto &key : keyToDelete )
			{
				S.erase( key );

				if ( key == ij )
				{
					continue;
				}

				int n = ( key.first == i || key.first == j ) ? key.second : key.first;
				S[std::make_pair( n, t )] = calcSimilarity( R[n], R[t], imgSize );
			}
		}

		std::vector<Rect> proposals;
		proposals.reserve( R.size() );

		for ( auto &r : R )
		{
			// exclude same rectangle (with different segments)
			if ( std::find( proposals.begin(), proposals.end(), r.second.rect ) != proposals.end() )
			{
				continue;
			}

			// exclude regions that is smaller/larger than assigned size
			if ( r.second.size < smallest || r.second.size > largest )
			{
				continue;
			}

			double w = r.second.rect.width;
			double h = r.second.rect.height;

			// exclude distorted rects
			if ( ( w / h > distorted ) || ( h / w > distorted ) )
			{
				continue;
			}

			proposals.push_back( r.second.rect );
		}

		return proposals;
	}
}
