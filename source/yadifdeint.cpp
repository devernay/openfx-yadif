// Yadif (yet another deinterlacing filter)
// http://avisynth.org.ru/yadif/yadif.html
// http://mplayerhq.hu

// Port to OFX/Vegas by George Yohng http://yohng.com

// All supplemental code is public domain, and
// Yadif algorithm code part itself is licensed GPL 
// (as the original yadif plugin)

// See the comments in this source

#include <cstring>
#include <cmath>
#include <algorithm>
#ifdef DEBUG
// Note: only cout & stdout are visible on the console in DaVinci Resolve, don't use cerr/stderr
#include <iostream>
#include <cstdio>
#endif

#include "ofxsImageEffect.h"
#include "ofxsMultiThread.h"
#include "ofxsProcessing.H"

#define PLUGIN_CLASS         YadifDeint
#define PLUGIN_CLASS_FACTORY YadifDeintFactory
#define PLUGIN_NAME          "Deinterlace (yadif)"
#define PLUGIN_GROUP         "Time"
#define PLUGIN_VERSION_MAJOR 1
#define PLUGIN_VERSION_MINOR 0
#define PLUGIN_IDENT         "hu.mplayerhq:YadifDeint"

class YadifDeint : public OFX::ImageEffect 
{
public:
    YadifDeint(OfxImageEffectHandle handle) : ImageEffect(handle), _dstClip(0), _srcClip(0)
    {
        _dstClip = fetchClip(kOfxImageEffectOutputClipName);
        _srcClip = fetchClip(kOfxImageEffectSimpleSourceClipName);

        mode = fetchChoiceParam("mode");
        fieldOrder = fetchChoiceParam("fieldOrder");
        parity = fetchChoiceParam("parity");
    }

private:
    virtual void render(const OFX::RenderArguments &args);

    /** @brief get the clip preferences */
    virtual void getClipPreferences(OFX::ClipPreferencesSetter &clipPreferences) /* OVERRIDE FINAL */;

    /** Override the get frames needed action */
    virtual void getFramesNeeded(const OFX::FramesNeededArguments &args, OFX::FramesNeededSetter &frames) /*OVERRIDE FINAL*/;

private:
    // do not need to delete these, the ImageEffect is managing them for us
    OFX::Clip *_dstClip;
    OFX::Clip *_srcClip;

    OFX::ChoiceParam *fieldOrder, *mode, *parity;
    
};



// =========== GNU Lesser General Public License code start =================

// Yadif (yet another deinterlacing filter)
// http://avisynth.org.ru/yadif/yadif.html
// http://mplayerhq.hu

// Original port to OFX/Vegas by George Yohng http://yohng.com
// Rewritten after yadif relicensing to LGPL:
// http://git.videolan.org/?p=ffmpeg.git;a=commit;h=194ef56ba7e659196fe554782d797b1b45c3915f


// Copyright notice and licence from the original libavfilter/vf_yadif.c file:
/*
 * Copyright (C) 2006-2011 Michael Niedermayer <michaelni@gmx.at>
 *               2010      James Darnley <james.darnley@gmail.com>
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

//#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
//#define FFMAX(a,b) ((a) < (b) ? (b) : (a))
//#define FFABS(a) ((a) > 0 ? (a) : (-(a)))
#define FFMIN(a,b) std::min(a,b)
#define FFMAX(a,b) std::max(a,b)
#define FFABS(a) std::abs(a)

#define FFMIN3(a,b,c) FFMIN(FFMIN(a,b),c)
#define FFMAX3(a,b,c) FFMAX(FFMAX(a,b),c)

inline float halven(float f) { return f*0.5f; }
inline int halven(int i) { return i>>1; }

inline int one1(unsigned char *) { return 1; }
inline int one1(unsigned short *) { return 1; }
inline float one1(float *) { return 0.f; }

#define CHECK(j)\
    {   Diff score = FFABS(cur[mrefs + ch * (- 1 + (j))] - cur[prefs + ch * (- 1 - (j))])\
                 + FFABS(cur[mrefs   + ch * (j)] - cur[prefs   - ch * (j)])\
                 + FFABS(cur[mrefs + ch * (1 + (j))] - cur[prefs + ch * (1 - (j))]);\
        if(score < spatial_score){\
            spatial_score= score;\
            spatial_pred= halven(cur[mrefs  + ch * (j)] + cur[prefs  - ch * (j)]);\

/* The is_not_edge argument here controls when the code will enter a branch
 * which reads up to and including x-3 and x+3. */

#define FILTER(start, end, is_not_edge) \
    for (x = start;  x < end; x++) { \
        Diff c= cur[mrefs]; \
        Diff d= halven(prev2[0] + next2[0]); \
        Diff e= cur[prefs]; \
        Diff temporal_diff0= FFABS(prev2[0] - next2[0]); \
        Diff temporal_diff1=halven( FFABS(prev[mrefs] - c) + FFABS(prev[prefs] - e) ); \
        Diff temporal_diff2=halven( FFABS(next[mrefs] - c) + FFABS(next[prefs] - e) ); \
        Diff diff= FFMAX3(halven(temporal_diff0), temporal_diff1, temporal_diff2); \
        Diff spatial_pred= halven(c+e); \
 \
        if (is_not_edge) {\
            Diff spatial_score = FFABS(cur[mrefs-ch] - cur[prefs-ch]) + FFABS(c-e) \
                              + FFABS(cur[mrefs+ch] - cur[prefs+ch]) - one1((Comp*)0); \
            CHECK(-1) CHECK(-2) }} }} \
            CHECK( 1) CHECK( 2) }} }} \
        }\
 \
        if (!(mode&2)) { \
            Diff b = halven(prev2[2 * mrefs] + next2[2 * mrefs]); \
            Diff f = halven(prev2[2 * prefs] + next2[2 * prefs]); \
            Diff max = FFMAX3(d - e, d - c, FFMIN(b - c, f - e)); \
            Diff min = FFMIN3(d - e, d - c, FFMAX(b - c, f - e)); \
 \
            diff = FFMAX3(diff, min, -max); \
        } \
 \
        if (spatial_pred > d + diff) \
           spatial_pred = d + diff; \
        else if (spatial_pred < d - diff) \
           spatial_pred = d - diff; \
 \
        dst[0] = (Comp)spatial_pred; \
 \
        dst += ch; \
        cur += ch; \
        prev += ch; \
        next += ch; \
        prev2 += ch; \
        next2 += ch; \
    }

template<int ch,typename Comp,typename Diff>
inline void filter_line_c(Comp *dst1,
                          const Comp *prev1, const Comp *cur1, const Comp *next1,
                          int w, int prefs, int mrefs, int parity, int mode)
{
    Comp *dst  = dst1;
    const Comp *prev = prev1;
    const Comp *cur  = cur1;
    const Comp *next = next1;
    int x;
    const Comp *prev2 = parity ? prev : cur ;
    const Comp *next2 = parity ? cur  : next;

    /* The function is called with the pointers already pointing to data[3] and
     * with 6 subtracted from the width.  This allows the FILTER macro to be
     * called so that it processes all the pixels normally.  A constant value of
     * true for is_not_edge lets the compiler ignore the if statement. */
    FILTER(0, w, 1)
}

#define MAX_ALIGN 8
template<int ch,typename Comp,typename Diff>
inline void filter_edges(Comp *dst1,
                         const Comp *prev1, const Comp *cur1, const Comp *next1,
                         int w, int prefs, int mrefs, int parity, int mode)
{
    Comp *dst  = dst1;
    const Comp *prev = prev1;
    const Comp *cur  = cur1;
    const Comp *next = next1;
    int x;
    const Comp *prev2 = parity ? prev : cur ;
    const Comp *next2 = parity ? cur  : next;

    /* Only edge pixels need to be processed here.  A constant value of false
     * for is_not_edge should let the compiler ignore the whole branch. */
    FILTER(0, 3, 0)

    dst  = dst1  + (w - 3)*ch;
    prev = prev1 + (w - 3)*ch;
    cur  = cur1  + (w - 3)*ch;
    next = next1 + (w - 3)*ch;
    prev2 = parity ? prev : cur;
    next2 = parity ? cur  : next;

    FILTER(w - 3, w, 0)
}

template<int ch,typename Comp,typename Diff>
static void filter_plane(int mode, Comp *dst, int dst_stride,
                         const Comp *prev0, const Comp *cur0, const Comp *next0,
                         int refs, int w, int h, int parity, int tff)
{
    int pix_3 = 3 * ch;
    for (int y = 0; y < h; ++y) {
        if (((y ^ parity) & 1)) {
            const Comp *prev= prev0 + y*refs;
            const Comp *cur = cur0 + y*refs;
            const Comp *next= next0 + y*refs;
            Comp *dst2= dst + y*dst_stride;
            int mode2 = y == 1 || y + 2 == h ? 2 : mode;

            for (int c = 0; c < ch; ++c) {
                filter_line_c<ch,Comp,Diff>(dst2 + c + pix_3, prev + c + pix_3, cur + c + pix_3, next + c + pix_3, w - 6,
                                            y + 1 < h ? refs : -refs,
                                            y ? -refs : refs,
                                            parity ^ tff, mode2);
                filter_edges<ch,Comp,Diff>(dst2 + c, prev + c, cur + c, next + c, w,
                                           y + 1 < h ? refs : -refs,
                                           y ? -refs : refs,
                                           parity ^ tff, mode2);
            }
        } else {
            memcpy(&dst[y * dst_stride],
                   &cur0[y * refs], w * ch * sizeof(Comp)); // copy original
        }
    }
    
}

// =========== GNU Lesser General Public License code end =================

void YadifDeint::render(const OFX::RenderArguments &args)
{
    OFX::BitDepthEnum       dstBitDepth    = _dstClip->getPixelDepth();
    OFX::PixelComponentEnum dstComponents  = _dstClip->getPixelComponents();

    std::auto_ptr<OFX::Image> dst(_dstClip->fetchImage(args.time));

    std::auto_ptr<OFX::Image> src(_srcClip->fetchImage(args.time)),
                              srcp(_srcClip->fetchImage(args.time-1.0)),
                              srcn(_srcClip->fetchImage(args.time+1.0));
                                                                                        
    OfxRectI rect = dst->getBounds();

    int width=rect.x2-rect.x1;
    int height=rect.y2-rect.y1;

    int imode       = 0;
    int ifieldOrder = 2;
    int iparity     = 0;

    mode->getValueAtTime(args.time,imode);
    fieldOrder->getValueAtTime(args.time,ifieldOrder);
    parity->getValueAtTime(args.time,iparity);

    imode*=2;

    if (ifieldOrder==2)
    {
        if (width>1024) 
            ifieldOrder=1;
        else
            ifieldOrder=0;
    }


    if (height>10)
    {
        if(dstComponents == OFX::ePixelComponentRGBA) 
        {
            switch(dstBitDepth) 
            {
            case OFX::eBitDepthUByte :
                filter_plane<4,unsigned char,int>(imode, // mode
                   (unsigned char*)dst->getPixelAddress(0,0),
                   (int)((unsigned char*)dst->getPixelAddress(0,1)-(unsigned char*)dst->getPixelAddress(0,0)),
                   (unsigned char*)(&*srcp?srcp->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (unsigned char*)src->getPixelAddress(0,0),
                   (unsigned char*)(&*srcn?srcn->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (int)((unsigned char*)src->getPixelAddress(0,1)-(unsigned char*)src->getPixelAddress(0,0)),
                   width,height,iparity,ifieldOrder); // parity, tff
     
                break;

            case OFX::eBitDepthFloat : 
                filter_plane<4,float,float>(imode, // mode
                   (float*)dst->getPixelAddress(0,0),
                   (int)((float*)dst->getPixelAddress(0,1)-(float*)dst->getPixelAddress(0,0)),
                   (float*)(&*srcp?srcp->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (float*)src->getPixelAddress(0,0),
                   (float*)(&*srcn?srcn->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (int)((float*)src->getPixelAddress(0,1)-(float*)src->getPixelAddress(0,0)),
                   width,height,iparity,ifieldOrder); // parity, tff
                break;

            default:
                break;
            }
        }else
        if(dstComponents == OFX::ePixelComponentAlpha) 
        {
            switch(dstBitDepth) 
            {
            case OFX::eBitDepthUByte :
                filter_plane<1,unsigned char,int>(imode, // mode
                   (unsigned char*)dst->getPixelAddress(0,0),
                   (int)((unsigned char*)dst->getPixelAddress(0,1)-(unsigned char*)dst->getPixelAddress(0,0)),
                   (unsigned char*)(&*srcp?srcp->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (unsigned char*)src->getPixelAddress(0,0),
                   (unsigned char*)(&*srcn?srcn->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (int)((unsigned char*)src->getPixelAddress(0,1)-(unsigned char*)src->getPixelAddress(0,0)),
                   width,height,iparity,ifieldOrder); // parity, tff
     
                break;

            case OFX::eBitDepthFloat : 
                filter_plane<1,float,float>(imode, // mode
                   (float*)dst->getPixelAddress(0,0),
                   (int)((float*)dst->getPixelAddress(0,1)-(float*)dst->getPixelAddress(0,0)),
                   (float*)(&*srcp?srcp->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (float*)src->getPixelAddress(0,0),
                   (float*)(&*srcn?srcn->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (int)((float*)src->getPixelAddress(0,1)-(float*)src->getPixelAddress(0,0)),
                   width,height,iparity,ifieldOrder); // parity, tff
                break;

            default:
                break;
            }
        }
    }else
    {
        for(int y=0;y<height;y++)
        {
            memcpy(dst->getPixelAddress(0,y),src->getPixelAddress(0,y),abs(src->getRowBytes()));
        }
    }
}

/* Override the clip preferences */
void
YadifDeint::getClipPreferences(OFX::ClipPreferencesSetter &clipPreferences)
{
    // set the fielding of _dstClip
    clipPreferences.setOutputFielding(OFX::eFieldNone);
}


void
YadifDeint::getFramesNeeded(const OFX::FramesNeededArguments &args,
                            OFX::FramesNeededSetter &frames)
{
    OfxRangeD range;
    range.min = args.time - 1;
    range.max = args.time + 1;
    frames.setFramesNeeded(*_srcClip, range);
}

mDeclarePluginFactory(PLUGIN_CLASS_FACTORY, {}, {});

using namespace OFX;

void PLUGIN_CLASS_FACTORY::describe(OFX::ImageEffectDescriptor &desc)
{
    //printf("describe!\n");
    const char *name  = PLUGIN_NAME;
    const char *group = PLUGIN_GROUP;

    desc.setLabels(name,name,name);
    desc.setPluginGrouping(group);
    desc.setPluginDescription("Port of YADIF (Yet Another DeInterlacing Filter) from MPlayer by Michael Niedermayer (http://www.mplayerhq.hu). It check pixels of previous, current and next frames to re-create the missed field by some local adaptive method (edge-directed interpolation) and uses spatial check to prevent most artifacts. Port to OFX/Vegas by George Yohng http://yohng.com");

    // add supported context
    desc.addSupportedContext(eContextFilter);

    // add supported pixel depths
    desc.addSupportedBitDepth(eBitDepthUByte);
    desc.addSupportedBitDepth(eBitDepthFloat);

    // set a few flags
    desc.setSingleInstance(false);
    desc.setHostFrameThreading(false);
    desc.setSupportsMultiResolution(true);
    desc.setSupportsTiles(false);
    desc.setTemporalClipAccess(true);
    desc.setRenderTwiceAlways(false);
    desc.setSupportsMultipleClipPARs(false);
    desc.setRenderThreadSafety(OFX::eRenderFullySafe);
    //printf("describe! OK\n");
}

void PLUGIN_CLASS_FACTORY::describeInContext(OFX::ImageEffectDescriptor &desc, OFX::ContextEnum /*context*/)
{
    //printf("describeInContext!\n");
    // Source clip only in the filter context
    // create the mandated source clip
    ClipDescriptor *srcClip = desc.defineClip(kOfxImageEffectSimpleSourceClipName);
    srcClip->addSupportedComponent(ePixelComponentRGBA);
    srcClip->addSupportedComponent(ePixelComponentAlpha);
    srcClip->setTemporalClipAccess(true);
    srcClip->setSupportsTiles(false);
    srcClip->setIsMask(false);
    srcClip->setFieldExtraction(OFX::eFieldExtractBoth);

    // create the mandated output clip
    ClipDescriptor *dstClip = desc.defineClip(kOfxImageEffectOutputClipName);
    dstClip->addSupportedComponent(ePixelComponentRGBA);
    dstClip->addSupportedComponent(ePixelComponentAlpha);
    dstClip->setSupportsTiles(false);


    PageParamDescriptor *page = desc.definePageParam("Controls");

    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam("mode");
        param->setLabels("Processing Mode", "Mode", "Processing Mode");
        param->setScriptName("mode");
        param->setHint("Mode of checking fields");
        param->appendOption("Temporal & spatial");
        param->appendOption("Temporal only");
        param->setDefault(0);
        param->setAnimates(true); // can animate
        page->addChild(*param);
    }

    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam("fieldOrder");
        param->setLabels("Field Order", "Fields", "Field Order");
        param->setScriptName("fieldOrder");
        param->setHint("Interlaced field order");
        param->appendOption("Lower field first");
        param->appendOption("Upper field first");
        param->appendOption("HD=upper,SD=lower");
        param->setDefault(2);
        param->setAnimates(true); // can animate
        page->addChild(*param);
    }

    {
        ChoiceParamDescriptor *param = desc.defineChoiceParam("parity");
        param->setLabels("Parity", "Parity", "Parity");
        param->setScriptName("parity");
        param->setHint("Interpolate which field");
        param->appendOption("Lower");
        param->appendOption("Upper");
        param->setDefault(0);
        param->setAnimates(true); // can animate
        page->addChild(*param);
    }
    //printf("describeInContext! OK\n");
}

OFX::ImageEffect* PLUGIN_CLASS_FACTORY::createInstance(OfxImageEffectHandle handle, OFX::ContextEnum /*context*/)
{
    //printf("createInstance!\n");
    return new PLUGIN_CLASS(handle);
}

static PLUGIN_CLASS_FACTORY p(PLUGIN_IDENT, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
mRegisterPluginFactoryInstance(p)
