/*
 * Voronoi.h
 *
 *  Created on: Jan 24, 2010
 *      Author: katrin.schmid
*/


#ifndef VORONOI_GEN
#define VORONOI_GEN

#include <vector>

typedef enum { RANDOM, GRID, HEX } PointMode;

/*! \class Color
* \brief A simple color class 
*
* provides some simple color math functions
*/
class Color {
public:
  Color() {
   this->zero();
  };
  ~Color() {
  };
   void zero();
   void addColors( Color &that);
   void divColors( float f);
   void average( Color &that);
   void copy( Color &that);
   void print( FILE *fp);
   void cap(float max);
   void randomColor();
   int random255();
   float random01();

   int equals( Color &that);
    float m_r, m_g, m_b;// TODO: make private
};

/*! \class ColoredPoint 
* \brief A colored point in 2d space
*
*/
class ColoredPoint 
{
 public:
    ColoredPoint(float x = 0 , float y = 0)
    {
    setPosition( x, y);
    };
   ~ColoredPoint () {
   };

    void setPosition(float x, float y);
    void randomSize( int w,  int h);
    float distanceSquared( float x, float y, int w, int h);

    Color m_ResultColor;
    Color m_Color;
private:
   float m_x, m_y;
};

/*! \class Voronoi
* \brief Creates different Cellnoise patterns
*
*/
class Voronoi 
{
public:
    Voronoi() 
    {
        previousColor.randomColor();
        m_width = 512;
        m_height = 512;
        m_numberOfPoints = 64;
        m_mode = RANDOM;
        m_pointCountInput = 64;
    };
    ~Voronoi() 
    {
     //m_points.resize(0);
    };

    int doVoronoi(int width,  int height);
    int doVoronoiXY(Color& t_ResultColor, int cMode, 
           int numberPoints,  int width,  int height, float xPosition, float yPosition);
    int doVoronoiLine(std::vector< Color >& t_ResultColor, 
          int cMode, int numberPoints, int width, int height, int xStart, int xEnd, int yPosition);
        
private:
    void initPoints(int numberPoints);
   void closestColor(float x, float y, Color &accumulator);

   int m_width, m_height, m_numberOfPoints, m_pointCountInput, m_antialias;
    PointMode m_mode;
    Color m_current, previousColor;
    std::vector<ColoredPoint> m_points;
};
#endif 

 
