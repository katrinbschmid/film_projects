/** \defgroup Cellnoise
 * @file
 * @author  katrin schmid <info@lo-motion.de>
 * @version 0.2.0
 *
 *
 * @section DESCRIPTION
 * \brief
 * Simple cell noise generator node for nuke, tested with v 5.2 only
 * Has random, grid and heaxgon mode and cell count and range values.
 *
 * To install: copy to Nuke\plugins directory, load from "etc" plugins menu
 * 0.2.0 Fixed some issues with format
 * 0.1.0 Initial release
*/

#ifdef debug
#undef debug
#endif
//#define debug

static const char* const CLASS = "Cellnoise";
static const char* const HELP = "Generates cell noise";

#include "DDImage/PixelIop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"
#include "DDImage/Knob.h"
#include "DDImage/DDMath.h"
#include "DDImage/NukeWrapper.h"

using namespace DD::Image;
#include "Voronoi.h"

const char* const _modeList[] =
{
    "Random",
    "Grid",
    "Hexagon",
     NULL
};

inline float remap(float min, float max, float value)
{
    return (min +(max-min)*value);
}

/*! \class Cellnoise
* \brief PixelIop that generates Cellnoise pattern
*
*/
class Cellnoise : public PixelIop
{
    Voronoi m_diagram;

public:
    Channel m_channel;
    int m_Nmode, m_numc;
    int m_size, m_w, m_h;
    float m_min, m_max;

    Cellnoise(Node* node) : PixelIop(node)
    {
   // default values
        m_min = 0.f;
        m_max = 1.f;
        m_numc = 64;
        m_Nmode = 2;
        m_size = format().width();
    }

    void knobs(Knob_Callback f)
    {
        Obsolete_knob(f, "proxy_format", 0);
        Input_Channel_knob(f, &m_channel, 1, 0, "m_channel");

        Knob* e = Enumeration_knob(f, &m_Nmode, _modeList, "mode_selector");
        Int_knob(f, &m_numc, "cells" );
        Float_knob(f, &m_max, "range max" );
        Float_knob(f, &m_min, "range min" );
    }

    void _validate(bool for_real)
    {
        PixelIop::_validate(for_real);// order can not change
        info_.set( format() );
        info_.channels(Mask_RGBA);
        set_out_channels(Mask_RGBA);
    }

    void in_channels(int input, ChannelSet& mask) const
    {
   // mask is unchanged
    }

    void pixel_engine(const Row& in, int y, int xx, int r,
          ChannelMask channels, Row& row)
    {
       Color resultColor;
       resultColor.zero();
       float curColor[4];
       curColor[0] = curColor[1] = curColor[2] = curColor[3] = 1.f;
       float* outptr = 0;

       foreach (z, channels)
       {
          if (z < 1 || z > 3)// no color
             row.copy(in, z, xx, r);
          else // 1,2,3 rgb
          {
             for ( int i = xx; i < r; i++)
             {
                if (curColor[z - 1] == -1.f)
                {
                   m_diagram.doVoronoiXY(resultColor, m_Nmode, m_numc, info_.format().width(), info_.format().height(), i, y);
                   curColor[0] = remap( m_min, m_max, resultColor.m_r);
                   curColor[1] = remap( m_min, m_max, resultColor.m_g);
                   curColor[2] = remap( m_min, m_max, resultColor.m_b);
                }
                outptr = row.writable(z) + i;
                *outptr++ = curColor[z - 1];
                curColor[z - 1] = -1.f;
             }
          }
       }
   };

    const char* Class() const { return CLASS; }
    const char* displayName() const { return "CellNoise"; }
    const char* node_help() const { return HELP; }
    static const Description desc;
};

static Iop* constructor(Node* node) { return new NukeWrapper(new Cellnoise(node)); }

//static Iop* constructor(Node* node) { return new Cellnoise(node); }
const Iop::Description Cellnoise::desc(CLASS, "Image/CellNoise", constructor);

