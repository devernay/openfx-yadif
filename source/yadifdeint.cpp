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
    YadifDeint(OfxImageEffectHandle handle) : ImageEffect(handle), dstClip_(0), srcClip_(0)
    {
        dstClip_ = fetchClip(kOfxImageEffectOutputClipName);
        srcClip_ = fetchClip(kOfxImageEffectSimpleSourceClipName);

        mode = fetchChoiceParam("mode");
        fieldOrder = fetchChoiceParam("fieldOrder");
        parity = fetchChoiceParam("parity");
    }

private:
    virtual void render(const OFX::RenderArguments &args);

    /** @brief get the clip preferences */
    virtual void getClipPreferences(OFX::ClipPreferencesSetter &clipPreferences) /* OVERRIDE FINAL */;

private:
    // do not need to delete these, the ImageEffect is managing them for us
    OFX::Clip *dstClip_;
    OFX::Clip *srcClip_;

    OFX::ChoiceParam *fieldOrder, *mode, *parity;
    
};



// =========== GNU General Public License code start =================

//#define MIN(a,b) ((a) > (b) ? (b) : (a))
//#define MAX(a,b) ((a) < (b) ? (b) : (a))
//#define ABS(a) ((a) > 0 ? (a) : (-(a)))
#define MIN(a,b) std::min(a,b)
#define MAX(a,b) std::max(a,b)
#define ABS(a) std::abs(a)

#define MIN3(a,b,c) MIN(MIN(a,b),c)
#define MAX3(a,b,c) MAX(MAX(a,b),c)

inline float halven(float f) { return f*0.5f; }
inline int halven(int i) { return i>>1; }

inline int one1(unsigned char *) { return 1; }
inline float one1(float *) { return 1.0f/256.0f; }

template<int ch,typename Comp,typename Diff>
inline void filter_line_c(int mode, Comp *dst, const Comp *prev, const Comp *cur, const Comp *next, int w, long refs, int parity){
    int x;
    const Comp *prev2= parity ? prev : cur ;
    const Comp *next2= parity ? cur  : next;

    for(x=0; x<w; x++){
        Diff c= cur[-refs];
        Diff d= halven(prev2[0] + next2[0]);
        Diff e= cur[+refs];
        Diff temporal_diff0= ABS(prev2[0] - next2[0]);
        Diff temporal_diff1=halven( ABS(prev[-refs] - c) + ABS(prev[+refs] - e) );
        Diff temporal_diff2=halven( ABS(next[-refs] - c) + ABS(next[+refs] - e) );
        Diff diff= MAX3(halven(temporal_diff0), temporal_diff1, temporal_diff2);
        Diff spatial_pred= halven(c+e);
        Diff spatial_score= ABS(cur[-refs-ch] - cur[+refs-ch]) + ABS(c-e)
                          + ABS(cur[-refs+ch] - cur[+refs+ch]) - one1((Comp*)0);

#define CHECK(j)\
    {   Diff score= ABS(cur[-refs-ch+ j] - cur[+refs-ch- j])\
                 + ABS(cur[-refs   + j] - cur[+refs   - j])\
                 + ABS(cur[-refs+ch+ j] - cur[+refs+ch- j]);\
        if(score < spatial_score){\
            spatial_score= score;\
            spatial_pred= halven(cur[-refs  + j] + cur[+refs  - j]);\

        CHECK(-ch) CHECK(-(ch*2)) }} }}
        CHECK( ch) CHECK( (ch*2)) }} }}

        if(mode<2){
            Diff b= halven(prev2[-2*refs] + next2[-2*refs]);
            Diff f= halven(prev2[+2*refs] + next2[+2*refs]);
#if 0
            Diff a= cur[-3*refs];
            Diff g= cur[+3*refs];
            Diff max= MAX3(d-e, d-c, MIN3(MAX(b-c,f-e),MAX(b-c,b-a),MAX(f-g,f-e)) );
            Diff min= MIN3(d-e, d-c, MAX3(MIN(b-c,f-e),MIN(b-c,b-a),MIN(f-g,f-e)) );
#else
            Diff max= MAX3(d-e, d-c, MIN(b-c, f-e));
            Diff min= MIN3(d-e, d-c, MAX(b-c, f-e));
#endif

            diff= MAX3(diff, min, -max);
        }

        if(spatial_pred > d + diff)
           spatial_pred = d + diff;
        else if(spatial_pred < d - diff)
           spatial_pred = d - diff;

        dst[0] = (Comp)spatial_pred;

        dst+=ch;
        cur+=ch;
        prev+=ch;
        next+=ch;
        prev2+=ch;
        next2+=ch;
    }
}

inline void interpolate(unsigned char *dst, const unsigned char *cur0,  const unsigned char *cur2, int w)
{
    int x;
    for (x=0; x<w; x++) {
        dst[x] = (cur0[x] + cur2[x] + 1)>>1; // simple average
    }
}

inline void interpolate(float *dst, const float *cur0,  const float *cur2, int w)
{
    int x;
    for (x=0; x<w; x++) {
        dst[x] = (cur0[x] + cur2[x] )*0.5f; // simple average
    }
}


template<int ch,typename Comp,typename Diff>
static void filter_plane(int mode, Comp *dst, long dst_stride, const Comp *prev0, const Comp *cur0, const Comp *next0, long refs, int w, int h, int parity, int tff)
{
	int y=0;

    if(((y ^ parity) & 1)) {
        memcpy(dst, cur0 + refs, w*ch*sizeof(Comp));// duplicate 1
    }else{
        memcpy(dst, cur0, w*ch*sizeof(Comp));
    }

    y=1;
    
    if(((y ^ parity) & 1)) {
        interpolate(dst + dst_stride, cur0, cur0 + refs*2, w*ch);   // interpolate 0 and 2
    }else{
        memcpy(dst + dst_stride, cur0 + refs, w*ch*sizeof(Comp)); // copy original
    }

    for(y=2; y<h-2; y++) {
        if(((y ^ parity) & 1)) {
            const Comp *prev= prev0 + y*refs;
            const Comp *cur = cur0 + y*refs;
            const Comp *next= next0 + y*refs;
            Comp *dst2= dst + y*dst_stride;

            for(int k=0;k<ch;k++)
                filter_line_c<ch,Comp,Diff>(mode, dst2+k, prev+k, cur+k, next+k, w, refs, (parity ^ tff));
        }else{
            memcpy(dst + y*dst_stride, cur0 + y*refs, w*ch*sizeof(Comp)); // copy original
        }
    }

    y=h-2;

    if(((y ^ parity) & 1)) {
        interpolate(dst + (h-2)*dst_stride, cur0 + (h-3)*refs, cur0 + (h-1)*refs, w*ch);   // interpolate h-3 and h-1
    }else{
      memcpy(dst + (h-2)*dst_stride, cur0 + (h-2)*refs, w*ch*sizeof(Comp)); // copy original
    }

    y=h-1;
    
    if(((y ^ parity) & 1)){
        memcpy(dst + (h-1)*dst_stride, cur0 + (h-2)*refs, w*ch*sizeof(Comp)); // duplicate h-2
    }else{
        memcpy(dst + (h-1)*dst_stride, cur0 + (h-1)*refs, w*ch*sizeof(Comp)); // copy original
    }
}

// =========== GNU General Public License code end =================

void YadifDeint::render(const OFX::RenderArguments &args)
{
    OFX::BitDepthEnum       dstBitDepth    = dstClip_->getPixelDepth();
    OFX::PixelComponentEnum dstComponents  = dstClip_->getPixelComponents();

    std::auto_ptr<OFX::Image> dst(dstClip_->fetchImage(args.time));

    std::auto_ptr<OFX::Image> src(srcClip_->fetchImage(args.time)),
                              srcp(srcClip_->fetchImage(args.time-1.0)),
                              srcn(srcClip_->fetchImage(args.time+1.0));
                                                                                        
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
                   (unsigned char*)dst->getPixelAddress(0,1)-(unsigned char*)dst->getPixelAddress(0,0),
                   (unsigned char*)(&*srcp?srcp->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (unsigned char*)src->getPixelAddress(0,0),
                   (unsigned char*)(&*srcn?srcn->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (unsigned char*)src->getPixelAddress(0,1)-(unsigned char*)src->getPixelAddress(0,0),
                   width,height,iparity,ifieldOrder); // parity, tff
     
                break;

            case OFX::eBitDepthFloat : 
                filter_plane<4,float,float>(imode, // mode
                   (float*)dst->getPixelAddress(0,0),
                   (float*)dst->getPixelAddress(0,1)-(float*)dst->getPixelAddress(0,0),
                   (float*)(&*srcp?srcp->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (float*)src->getPixelAddress(0,0),
                   (float*)(&*srcn?srcn->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (float*)src->getPixelAddress(0,1)-(float*)src->getPixelAddress(0,0),
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
                   (unsigned char*)dst->getPixelAddress(0,1)-(unsigned char*)dst->getPixelAddress(0,0),
                   (unsigned char*)(&*srcp?srcp->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (unsigned char*)src->getPixelAddress(0,0),
                   (unsigned char*)(&*srcn?srcn->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (unsigned char*)src->getPixelAddress(0,1)-(unsigned char*)src->getPixelAddress(0,0),
                   width,height,iparity,ifieldOrder); // parity, tff
     
                break;

            case OFX::eBitDepthFloat : 
                filter_plane<1,float,float>(imode, // mode
                   (float*)dst->getPixelAddress(0,0),
                   (float*)dst->getPixelAddress(0,1)-(float*)dst->getPixelAddress(0,0),
                   (float*)(&*srcp?srcp->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (float*)src->getPixelAddress(0,0),
                   (float*)(&*srcn?srcn->getPixelAddress(0,0):src->getPixelAddress(0,0)), 
                   (float*)src->getPixelAddress(0,1)-(float*)src->getPixelAddress(0,0),
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
    // set the fielding of dstClip_
    clipPreferences.setOutputFielding(OFX::eFieldNone);
}

mDeclarePluginFactory(PLUGIN_CLASS_FACTORY, {}, {});

using namespace OFX;

void PLUGIN_CLASS_FACTORY::describe(OFX::ImageEffectDescriptor &desc)
{
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

}

void PLUGIN_CLASS_FACTORY::describeInContext(OFX::ImageEffectDescriptor &desc, OFX::ContextEnum context)
{
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

}

OFX::ImageEffect* PLUGIN_CLASS_FACTORY::createInstance(OfxImageEffectHandle handle, OFX::ContextEnum context)
{
    return new PLUGIN_CLASS(handle);
}

namespace OFX { namespace Plugin { 

void getPluginIDs(OFX::PluginFactoryArray &ids) 
{
    static PLUGIN_CLASS_FACTORY p(PLUGIN_IDENT, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
    ids.push_back(&p);
} 

} } // namespace
