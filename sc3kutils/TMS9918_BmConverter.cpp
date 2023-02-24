#include "TMS9918_BmConverter.h"
#include <stdexcept>
#include <algorithm>
using namespace std;
using namespace vchip;

TMS_BmConverter::TMS_BmConverter(TMS9918State &tms)
    :_tms(tms)
{

}



    // sprite used for Bm
    struct MapSprite {
        // - - - this is the usefull hardware data
        uint8_t _x=0;
        uint8_t _y=0;
        uint8_t _name=0;
        uint8_t _color=0;
        // - - - - more data for algo
        uint8_t _maxX=0; // max X used inside sprite during feed, allow to try to slide left
        //? uint8_t _maxY=0;
        // keep a chunky bm of sprite that will be planar-translated at the end.
        vector<uint8_t> _chunk; // 16x16
        inline bool isOn() const {
            return (_color!=0);
        }
        // true if can take this pixel in count
       inline  bool fitForPixel(int x,int y, uint8_t color )
       {
            if(color != _color) return false;
            if(!(y>=_y && y<_y+16)) return false;
            // simple version
            if (x>=_x && x<(_x+16)) return true;
            if(x>=_x+16) return false; // too musch at right for this sprite
            // if at left in such way,can use shifting bitmap left:
            if(x>=(_x+_maxX)-16)
            {
                // case where yes, could scroll sprite and bitmap right
                scrollRight(x-((_x+_maxX)-16));
                return true;
            }
            return false;
       }
       // only if fitForPixel(), in screen coord.
       inline bool setpixel(int x,int y, uint8_t color )
       {
            if(!fitForPixel(x,y,color)) return false;
            if(_chunk.size()<16*16) return false;
            _chunk[ (x-_x) + (y-_y)*16]=1;
            if((x-_x)>_maxX) _maxX=(x-_x);
            return true;
       }
       inline void scrollRight(uint8_t nbxr)
       {
            for(uint8_t y=0;y<16;y++)
            {
                for(uint8_t x=15;x>15-nbxr;x--)
                {
                    _chunk[y*16 +x ]=_chunk[y*16 +x-nbxr ];
                }
                // clear at left
                for(uint8_t x=0;x<16-nbxr;x++)
                {
                    _chunk[y*16 +x ]=0;
                }
            }
            _x -= nbxr;
       }
    };

    class MapSpriteManager
    {
        public:
        MapSpriteManager() : _sprites(32),_horizontality(192) {

        }

        bool trySetToSprite(int x, int y, uint8_t color)
        {
            // test active
            for(uint8_t is=0; is<_iUsedSprites ;is++)
            {
                if( _sprites[is].fitForPixel(x,y,color))
                {
                    _sprites[is].setpixel(x,y,color);
                    return true;
                }
            }
            // try alloc with first pixel:
            return allocNewSprite(x,y,color);
        }
        protected:
        bool allocNewSprite(int x, int y, uint8_t color)
        {
            if(color==0) return false; // unmanageable
             if(_iUsedSprites==32) return false;
             // watch horizontality, at least for first line.
            if(_horizontality[y].size()>=4) return false;

            MapSprite &sprite = _sprites[_iUsedSprites];

            sprite._color = color;
            sprite._chunk = vector<uint8_t>(16*16,0);
            sprite._chunk[0]=1; // topleft.
            sprite._x = x;
            sprite._y = y;
            // horizontal occupation
            for(uint8_t yc=0;yc<16 && (yc+y)<192;yc++) {
                _horizontality[y+yc].push_back(_iUsedSprites);
            }

            _iUsedSprites++;
            return true;
        }

        vector<MapSprite> _sprites;
        vector<vector<uint8_t>> _horizontality;

        uint8_t _iUsedSprites=0;

    };

// spritecolors in sprite mode
void TMS_BmConverter::mode2MapIndexedPixelBm(const BmIndexPix &bmi, const vector<uint8_t> &spriteColors)
{
    _tms.setMode_Graphics2Default();


    uint16_t clbase = _tms.adress_ColorTable();
    uint16_t bmbase = _tms.adress_PatternGenerator();

    vector<uint8_t> &vmem = _tms.vmem();

    MapSpriteManager sprites;

    struct SecondChanceSpritePixels
    {
        uint8_t _x,_y,_col;
    };
    vector<SecondChanceSpritePixels> scsp;

    // consider already mapped to palette
    for(int y=0;y<bmi.height();y++)
    {

        for(int x=0;x<bmi.width()/8;x++)
        {
            // count colors
            vector<uint8_t> counts(16,0);
            for(int xl=0;xl<8;xl++)
            {
                uint8_t icol = bmi.pix((x<<3)+xl,y);
                if(icol>15 )
                {
                   throw runtime_error("pixel with value>15");
                }
                counts[icol]++;
            }
            // if only 2
            int nbcUsed=0;
            int ibestcol1 = 0;
            int ibestcolcount1 = 0;
            int ibestcol2 = 0;
            int ibestcolcount2 = 0;

            for(int icol=0;icol<16;icol++)
            {
                nbcUsed++;
                if(counts[icol]>0)
                {   // since we need just the 2 best, no sort...
                    if(counts[icol]>ibestcolcount1)
                    {   // this get 2 best
                        ibestcol2=ibestcol1;
                        ibestcolcount2 = ibestcolcount1;

                        ibestcol1 = icol;
                        ibestcolcount1 = counts[icol];
                    }
                }
            }
            // - - - - - remap 2 best colors
            {
                uint8_t cl = (ibestcol2<<4) | ibestcol1 ;
                uint8_t bm = 0;
                uint8_t bitmask = 128;
                for(int xl=0;xl<8;xl++)
                {
                    uint8_t icol = bmi.pix((x<<3)+xl,y);
                    if(icol == ibestcol1) bm |= bitmask;
                    bitmask>>=1;
                }
                uint16_t vmemadr = ( ((uint16_t)(y>>3))<<8) + ((uint16_t)(x & 0x1f)<<3) + (y&0x07) ;
                vmem[clbase+vmemadr] = cl;
                vmem[bmbase+vmemadr] = bm;
            }
            if(nbcUsed<=2)
            {
                continue;
            }
            // else may apply extra colors to sprites
            for(int xl=0;xl<8;xl++)
            {
                uint8_t icol = bmi.pix((x<<3)+xl,y);
                if(icol == ibestcol1 || icol == ibestcol2 )
                {
                    continue; // already managed in background bitmap.
                }
                // if happens to be already manageable by some allocated sprite,
                // use for sprite prioritary.
                bool isAsked = false;
                for(uint8_t ct : spriteColors) if(icol==ct) isAsked=true;
                if(isAsked) {
                    sprites.trySetToSprite((x<<3)|xl,y,icol);
                } else
                {   // list of unmatched pixels
                    SecondChanceSpritePixels p;
                    p._x = (x<<3)|xl;
                    p._y = y;
                    p._col = icol;
                    scsp.push_back(p);
                }
            }


            // watch if
// sprites

        } // end loop per x/8
    }
    // try less prio sprites
    for(SecondChanceSpritePixels &p : scsp)
    {
        sprites.trySetToSprite(p._x,p._y,p._col);
    }

}

// - - - -- - old not used.
/*
typedef uint16_t errt;

inline errt colorDif(const uint8_t *pRgb,uint32_t palettec )
{
    int16_t rp = (int16_t) pRgb[0];
    int16_t gp = (int16_t) pRgb[1];
    int16_t bp = (int16_t) pRgb[2];
    int16_t rr = rp - (palettec & 255);
    int16_t gg = gp - ((palettec>>8) & 255);
    int16_t bb = bp - ((palettec>>16) & 255);
    if(rr<0) rr=-rr;
    if(gg<0) gg=-gg;
    if(bb<0) bb=-bb;
    return (uint16_t)rr+gg+bb;
}

void TMS_BmConverter::map8PixelsToTMS(const uint8_t *pRgba,uint8_t &bm,uint8_t &cl, uint16_t &errorRate)
{

    const errt maxErr = 256*4;
    // map to palette
    vector<uint8_t> directRemap(8);
    vector<uint16_t> directRemapError(8);
    uint16_t directRemapErrorTotal = 0;

    const vector<uint32_t> &palette = _tms.paletteRGBA();
    vector<uint16_t> paletteScore(palette.size(),0);

    int mostUsedA=0;
    errt mostUsedAerr = maxErr;
    int mostUsedB=0;
    for(size_t i=0;i<8;i++)
    {                
        int bestc=0; // can't be 0 with TMS.
        errt besterr = 256*4;
        // 0 is transpa
        for(int ic=1;ic<(int)palette.size();ic++)
        {
            errt err = colorDif(pRgba,palette[ic]);
            if(err<besterr) bestc=ic;
        }
        directRemap[i]=bestc;
        directRemapError[i]=besterr;
        directRemapErrorTotal += besterr;

        // note: another approach would be find average color of region,...


        // sort out 2 most used colors
        paletteScore[bestc]++;

        pRgba+=4;
    }

    // get 2 dominants


    // map bm and color


}

void TMS_BmConverter::mode2Map(const BmRGBA &bm)
{

}
*/

