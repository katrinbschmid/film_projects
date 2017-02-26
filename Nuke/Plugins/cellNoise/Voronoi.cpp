/** \defgroup Voronoi
 * @file
 * @author  katrin schmid <info@lo-motion.de>
 * @version 0.2.0
 *
 * 
 * @section DESCRIPTION
 * \brief Simple Cell noise/Voroni noise generator (http://de.wikipedia.org/wiki/Voronoi-Diagramm)
 *
 * inspired by http://code.google.com/p/brianin3d-misc/source/browse/trunk/random/cxx/Voronoi.cpp. 
*/
#ifdef debug
#undef debug
#endif
//#define debug 
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Voronoi.h"

inline float roundFloatTo(float j, int digits)
{
    float f = (int)(j*100);
    f = f / 100.;
    return f;
}

PointMode toPointMode(int n)
{
    return (1 == n ? GRID : (2 == n ? HEX : (RANDOM)));
}

PointMode toPointMode(char *s)
{
   return toPointMode(atoi(s));
}

/// Color member functions
void Color::addColors(Color &that)
{
    this->m_r += that.m_r;
    this->m_g += that.m_g;
    this->m_b += that.m_b;
}

//set to white
void Color::zero() 
{
    this->m_r = this->m_g = this->m_b = 0.;
}

float Color::random01() 
{
    float f = roundFloatTo((rand() / float(RAND_MAX)), 3);
    return f;
}

int Color::random255() 
{
    return rand() % 255;
}

int Color::equals(Color &that)
{
    return this->m_r == that.m_r && this->m_g == that.m_g && this->m_b == that.m_b;
}

void Color::randomColor() 
{
    this->m_r = this->random01();
    this->m_g = this->random01();
    this->m_b = this->random01();
}

void Color::divColors(float f)
{
    if (f > 0)
    {
        m_r = m_r / f;
        m_g = m_g / f;
        m_b = m_b / f;
    }
}

void Color::average(Color &that)
{
    this->m_r = (this->m_r + that.m_r) / 2.f;
    this->m_g = (this->m_g + that.m_g) / 2.f;
    this->m_b = (this->m_b + that.m_b) / 2.f;
}

void Color::copy(Color &that)
{
    this->m_r = that.m_r;
    this->m_g = that.m_g;
    this->m_b = that.m_b;
}

void Color::print(FILE *fp)
{
    fprintf(fp, "%c%c%c", m_r, m_g, m_b);
}

void Color::cap(float max)
{
    if (m_r > max) m_r = max;
    if (m_g > max) m_g = max;
    if (m_b > max) m_b = max;

    if (m_r < 0.f) m_r = 0.f;
    if (m_g < 0.f) m_g = 0.f;
    if (m_b < 0.f) m_b = 0.f;
}

/// ColorPoint member functions
void ColoredPoint::setPosition(float x, float y)
{
    this->m_x = x;
    this->m_y = y;
    this->m_Color.randomColor();
}

void ColoredPoint::randomSize(int w,  int h)
{
    this->setPosition(rand() % w, rand() % h);
}

/// this distance calculation wraps around
float ColoredPoint::distanceSquared(float x, float y, int w, int h)
{
    float xdist = fabs(this->m_x - x);
    float ydist = fabs(this->m_y - y);

    if (xdist > float(w)/2)
    {
        if (this->m_x < x)
           xdist = this->m_x + (float(w) - x);
        else
            xdist = x + (float(w) - this->m_x);
    }
    
    if (ydist > float(h) / 2)
    {
        if (this->m_y < y)
            ydist = this->m_y + (h - y);
        else
            ydist = y + (float(h) - this->m_y);
    }
    return xdist * xdist + ydist * ydist;
}

/// Voronoi member functions
void Voronoi::closestColor(float x, float y, Color &accumulator)
{
    float d = 0.f;
    float closestDistance = 0.f;
    std::vector< float > distance;

    accumulator = previousColor;
    int n = m_points.size();
    distance.resize(n);

    // find the closest point to this point (x,y)
    for (int p = 0; p < n; p++)
    {
        distance[p] = m_points[p].distanceSquared(x, y, m_width, m_height);
        if (0 == p || distance[p] <= closestDistance)
        {
            closestDistance = distance[p];
            accumulator = (m_points[p].m_Color);
        }
    }

    // sometimes a point is closer to more than one point
    for (int i = 0; i < n; i++)
    {
        if (distance[i] == closestDistance)
        {
            //accumulator = (m_points[i].color);
            if (d == 0.f)
                accumulator.zero();
            d += 1.f;
            accumulator.addColors(m_points[i].m_Color);
        }
    }
    previousColor.copy(accumulator);

    if (d > 1.f)
    {
        //accumulator.cap(1.f);
        //accumulator.divColors(d);///
        accumulator.m_r = accumulator.m_r / d;
        accumulator.m_g = accumulator.m_g / d;
        accumulator.m_b = accumulator.m_b / d;
    }

    accumulator.cap(1.f);
}

void Voronoi::initPoints(int n) 
{
    float sqrtn = floor(sqrt(double(n)));
    int xi = static_cast <int>(m_width / sqrtn);
    int yi = static_cast <int>(m_height / sqrtn);
    int q = 0;
    m_points.resize(0);
       
    switch (m_mode) {
        case GRID:
            for (int y = 0; y < m_height; y += yi)
            {
                for (int x = 0; x < m_width; x += xi)
                m_points.push_back(ColoredPoint(x + xi / 2, y + yi / 2));
            }
            break;
        case HEX:
            for (int y = 0; y < m_height; y += yi)
                for (int x = (0 == q++ % 2 ? 0 : xi / 2); x < m_width; x += xi) {
                    m_points.push_back(ColoredPoint(x, y));
            }
            break;
        default:         
            ColoredPoint point;
            for (int i = 0; i < n; i++) {
                point.randomSize(m_width, m_height);
                m_points.push_back(point);
            }
        }
        m_numberOfPoints = m_points.size();
#ifdef debug
        std::cout << "Init points: n " << n << " m_n : "<< m_n << " : "<< m_points.size() <<std::endl;
#endif
}

// TODO: m_antialias
int Voronoi::doVoronoi(int w1,  int h1)
{
    Color previous;
    /*
    if (rotation != 0)
    {
       float a_cos = cos(rotation);
       float a_sin = sin(rotation);
   for (int i = 0; i < m_points.size(); i++)
   {
            m_points[i].rotate(a_cos, a_sin, m_w, m_h);
        }
    }
    */
    initPoints(5);
    for (int y = 0; y < m_height; y++)
    {
        for (int x = 0; x < m_width; x++)
        {
            closestColor(x, y, m_current);
      /*
           if (m_antialias && 0  != x)
      {
          // sort of a trailing average to "m_antialias"
          m_current.average(previous);
          previous.copy(m_current);
                }
       */
           // m_ResultColor.push_back(m_current);
        }
    }
    return 0;
}


int Voronoi::doVoronoiXY(Color& t_ResultColor, int cMode, int numberPoints, int width, int height, float x, float y) 
{
    if (m_pointCountInput != numberPoints || m_points.size() != m_numberOfPoints
      || m_width  != width || m_height  != height || PointMode(cMode) != m_mode)// m_points.size()< n1 |
    {
        m_pointCountInput = numberPoints;
        m_height = height;
        m_width = width;
        m_mode = toPointMode(cMode);
        initPoints(numberPoints);
    }

    Color c_current;
    c_current.zero();
    closestColor(x, y, c_current);
#ifdef debug
   std::cout << "doVoronoiXY : " << x <<  ": " << y <<  ": "<< m_current.m_r
      <<  ": "<< m_current.m_g <<  ": " << m_current.m_b << std::endl;
#endif
    t_ResultColor.copy(c_current);
    
    return 0;
}

int Voronoi::doVoronoiLine(std::vector<Color>& t_ResultColor, int cMode,
              int numberPoints, int width,
              int height, int xStart, int xEnd, int y)
{
    t_ResultColor.resize(0);
    if (m_pointCountInput != numberPoints || m_points.size() != m_numberOfPoints ||
           m_width != width || m_height  !=  height || cMode != m_mode)//m_points.size() != n1 ||
    {   
        m_pointCountInput = numberPoints;
        m_height = height;
        m_width = width;
        m_mode = toPointMode(cMode);
        initPoints(numberPoints);
    }
    
    Color previous, c_current;
    for (int i = xStart; i <= xEnd; i++)
    {
        c_current.zero();
        closestColor(i, y, c_current);
      /*
      if (m_antialias && 0.  != i)
      {
         // sort of a trailing average to "m_antialias"
         //m_current.average(previous);

         previous.copy(c_current);
        }
   */
        t_ResultColor.push_back(c_current);
    }

    return 0;
}
