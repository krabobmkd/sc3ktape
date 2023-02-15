#include "TMS9918State.h"

using namespace std;
using namespace vchip;

TMS9918State::TMS9918State()
    :_vmem(16384,0)
    ,_regs{0}
    ,_pixelWidth(256)
    ,_pixelHeight(192)
{
    _paletteRGBA = {
        0x000000ff,
        0x010101ff,
        0x3eb849ff,
        0x74d07dff,
        0x5955e0ff,
        0x8076f1ff,
        0xb95e51ff,

        0x65dbefff,
        0xdb6559ff,
        0xff897dff,
        0xfcc35eff,
        0xded087ff,
        0x3aa241ff,
        0xb766b5ff,
        0xccccccff,
        0xffffffff,
    };

}

void TMS9918State::setMode_Graphics2Default()
{
    //meka for screen2: 02 E0 0E FF 03 76 03 05   00 00 FF
    _regs._0._External =0;
    _regs._0._M3_Graphics2 =1 ;

    _regs._1._16k =1;
    _regs._1._ActiveDisplay =1;
    _regs._1._InteruptEnable =1;
    _regs._1._M1_textMode =0;

    _regs._1._M2_Multicolor = 0;
    _regs._1._ = 0;
    _regs._1._SpriteSize = 1;
    _regs._1._SpriteMag = 1;

     // actually bitmap are always 0-> $2000 (8k) = 256*8*3=6144=6kb=$1800
    // entre $1800 et $2000 on a $200 libre, on peut mettre les sprites (16*2) ->16 images ?
    // $2000->$3800 colors (values 1F)
    // $3800->$3B00  $300 3x series
    // $3C00->$3FC0 les char du mode texte.
    _regs._2._NameTableBaseAdress = 0x0e; //* 0x400 = $3800 (size=$300) -> end $3B00
    _regs._3._ColorTableBaseAdress = 0xff; // * 0x40  = $3FC0
    _regs._4._PatternGeneratorBaseAdress = 0x03; // * 0x800 = $1800 (just in mode1 ?)
    _regs._5._SpriteAttribTableBaseAdress = 0x76; // * 0x80 = 3B00
    _regs._6._SpritePatternGeneratorBaseAdress = 0x03; // * 0x800 = $1800

    // all to zero
    _vmem = vector<uint8_t>(16384,0);
    // set colors as black/white by default .
    for( uint16_t j=0x2000 ; j<0x3800 ; j++ ) _vmem[j]= 0x1f;

    // set Name table adress
    size_t i=(size_t)this->adress_NameTable();
    for(size_t j=0;j<256;j++) {
        _vmem[i]= _vmem[i+256]= _vmem[i+512]=(uint8_t)j;
        i++;
    }


//    _vregs={0x02,0xE0,
//           0x0E,0xFF,
//           0x03,0x76,
//           0x03,0x05
//           };
    // implie name tables...
    //_vmem
}
void TMS9918State::graphics2BitmapClear()
{
    for(uint16_t i=0;i<6*1024;i++)
    {
        _vmem[i]=0; // bitmap
        _vmem[i+0x2000]=0xf0; // color

    }
}

void TMS9918State::updateRender()
{
    if(_regs._0._M3_Graphics2 !=0)
    {
       updateRender_Mode2();
    }
}
uint32_t TMS9918State::normalizeColor1To0()
{
    uint32_t nbChangedone=0;
    // just set to 0 when unified 8 pixels
    for(uint16_t i=0;i<6*1024;i++)
    {
        bool changed=false;
        uint8_t cl =_vmem[i+0x2000];
        // case where useless BM
        uint8_t cl1 =  cl & 0x0f;
        uint8_t cl2 =  cl>>4;
        if(cl1==1) {
            cl1=0;
            nbChangedone++;
            changed=true;
        }
        if(cl2==1) {
            cl2=0;
            nbChangedone++;
            changed=true;
        }
        if(changed)
        {
            _vmem[i+0x2000] = (cl2<<4) | cl1;
        }
    }
    return nbChangedone;
}
// switch bits if needed, multipaint does nasty things sometimes
uint32_t TMS9918State::normalizeColorForCompression()
{
    uint32_t nbChangedone=0;
    // when all blank color, force an unused alternative color that can be used for plotter effects...
    // which will compress alright anyway.
    uint8_t alternativecolor=14 & 0x0f; // 0-15 values

    // just set to 0 when unified 8 pixels
    for(uint16_t i=0;i<6*1024;i++)
    {
        uint8_t bm =_vmem[i];
        uint8_t cl =_vmem[i+0x2000];
        // case where just one color on BM
        if(bm==0)
        {   // set 0 to unused color field
            //if((cl & 0xf0) !=0)
            {
                cl= (cl & 0x0f)| (alternativecolor<<4);
                _vmem[i+0x2000] =cl;
                nbChangedone++;
            }
        } else if(bm==255)
        {   // rather use color field -not sure if optimize- but normalize
            _vmem[i]=bm =0;
            cl = (cl>>4) | (alternativecolor<<4); // switch color
            _vmem[i+0x2000] = cl;
            nbChangedone++;
        }
        // case where useless BM
        uint8_t cl1 =  cl & 0x0f;
        uint8_t cl2 =  cl>>4;
        if(cl1==cl2 && (bm!=0 && bm !=255))
        {
            _vmem[i] = bm = 0;
            _vmem[i+0x2000] = cl = cl1;
            nbChangedone++;
        }
    }
    return nbChangedone;
}
uint32_t TMS9918State::normalizeHorizontalTiles()
{
    return 0;
}
uint32_t TMS9918State::toVerticalTiles()
{
    uint32_t nbChangedone=0;
    uint16_t nameTableBase =this->adress_NameTable();
    uint16_t bmBase = 0x0000;
    uint16_t clBase = 0x2000;
    for(int ipart=0;ipart<3;ipart++)
    {
        uint8_t nw=0;

        // copy state of previous names for this part
        vector<uint8_t> newnames(256);
        vector<uint8_t> newBm(32*8*8);
        vector<uint8_t> newCl(32*8*8);

        for( int ix=0 ; ix<32 ; ix++ )
        {
            for( int iy=0 ; iy<8 ; iy++ )
            {
                uint8_t nprev = _vmem[nameTableBase+ix+(iy<<5)];

                if(nw != nprev)
                {
                    // mode2  fields
                    uint16_t bmAdr =  (((uint16_t)nw)<<3);
                    uint16_t clAdr =  (((uint16_t)nw)<<3);
                    uint16_t bmprevAdr = bmBase + (((uint16_t)nprev)<<3);
                    uint16_t clprevAdr = clBase + (((uint16_t)nprev)<<3);

                    for(uint8_t j=0;j<8;j++) {
                        newBm[bmAdr] =_vmem[bmprevAdr];
                        newCl[clAdr] =_vmem[clprevAdr];

                        bmAdr++;
                        bmprevAdr++;
                        clAdr++;
                        clprevAdr++;
                    }

                   nbChangedone++;
                }
                // then change name
                newnames[ix+(iy<<5)] = nw;

                nw++;
            }
        }
        // then apply
        for(uint16_t i=0;i<32*8*8;i++) _vmem[bmBase+i]=newBm[i];
        for(uint16_t i=0;i<32*8*8;i++) _vmem[clBase+i]=newCl[i];
        for(uint16_t i=0;i<256;i++) _vmem[nameTableBase+i]=newnames[i];


        nameTableBase += 256;
        bmBase += 32*8*8; // +5+3+3=2kb
        clBase += 32*8*8;
    }

    return nbChangedone;
//            uint8_t bm = _vmem[screenBmShift+(n<<3)+yl]; // 8 2c pixels
            // get the 2 colors not using tiling but yet char per char
           // uint8_t cl = _vmem[0x2000+((xc+(yc<<5))<<3)+yl];
//            uint8_t cl = _vmem[0x2000+screenBmShift+(n<<3)+yl];
}



void TMS9918State::updateRender_Mode2()
{
    _pixelWidth = 256;
    _pixelHeight = 192;
    const size_t whsize = _pixelWidth*_pixelHeight;

    if(_renderedBitmap.size() != whsize)
    {
        _renderedBitmap.resize(whsize);
        _renderedBitmap32.resize(whsize);
    }
    // background color replace 0 values:
    uint8_t bgColor = _regs._7._BackdropColor;

    uint16_t nameTableBase =   0x1800 ;
        this->adress_NameTable();

    uint16_t colorbase = (_regs._3._ColorTableBaseAdress & 128)?0x2000:0;


          //  uint8_t _PatternGeneratorBaseAdress; // modce 2: only b 7 used.
            // 0x2000; // adress_ColorTable();
    uint16_t patternbase =(_regs._4._PatternGeneratorBaseAdress & 128)?0x2000:0;


    // scanning line is more emulatorish.
    uint8_t *pwr= _renderedBitmap.data();
    uint32_t *pwr32= _renderedBitmap32.data();
    uint8_t yb=0;

    // - - -translate bitmap - - - -
    for(uint16_t y=0;y<_pixelHeight ; y++)
    {
        uint16_t yl =y&7;
        uint16_t yc =y>>3;
        uint16_t screenBmShift = (yc>>3)<<11; // runs 0,1,2 256*8 each , so 0,2k,4k
        for(uint16_t xc=0;xc<_pixelWidth>>3 ; xc++)
        {
            // get bitmap using tiling
            uint16_t n = (uint16_t) _vmem[nameTableBase+xc+(yc<<5)]; // the char id that point the screen Bm
            uint8_t bm = _vmem[patternbase+screenBmShift+(n<<3)+yl]; // 8 2c pixels
            // get the 2 colors not using tiling but yet char per char
           // uint8_t cl = _vmem[0x2000+((xc+(yc<<5))<<3)+yl];
            uint8_t cl = _vmem[/*0x2000+*/colorbase+screenBmShift+(n<<3)+yl];
            uint8_t bgc = cl>>4;
            if(bgc==0) bgc = bgColor;
            uint8_t fgc = cl & 0x0f;
            if(fgc==0) fgc = bgColor;

            for(uint8_t xl=0;xl<8;xl++ )
            {
                uint8_t c = (bm&128)?bgc:fgc;
                *pwr++ = c;
                *pwr32++ = _paletteRGBA[c];

                bm<<=1;
            }
        }
    }

    // - - - - - - -add sprites

    uint16_t spriteAttribBase = adress_SpriteAttribs();
    uint16_t spritePatternsBase =  adress_SpritePatternGenerator();

    bool isSize1=true;
    for(int is=0; is<32 ;is++)
    {
        // attribs are on 128b aligned in vram 32*4
        uint16_t attribs = spriteAttribBase+ is*4;
        uint16_t yp = (uint16_t) _vmem[attribs];
        uint16_t xp = (uint16_t) _vmem[attribs+1];
        uint8_t n = _vmem[attribs+2];
        uint8_t flags = _vmem[attribs+3];
        uint8_t color = flags & 0x0f;
        bool earlyleft = (flags & 0x80)!=0; //-32 pixel left

        uint16_t pattern =spritePatternsBase+ (n<<3);

        if(isSize1)
        {
            for(uint16_t y=0;y<16 && (y+yp)<192 ;y++)
            {
                uint8_t bm = _vmem[pattern+y]; // 8 2c pixels

                for(uint16_t x=0;x<8 && (x+xp)<256;x++)
                {
                   if(bm&128)
                   {
                       uint16_t ipix = ((y+yp)<<8)+xp+x;
                        _renderedBitmap[ipix] = color;
                        _renderedBitmap32[ipix] = _paletteRGBA[color];
                   }
                   bm<<=1;
                }
            }
            for(uint16_t y=0;y<16 && (y+yp)<192 ;y++)
            {
                uint8_t bm = _vmem[pattern+y+16]; // 8 2c pixels

                for(uint16_t x=8;x<16 && (x+xp)<256;x++)
                {
                   if(bm&128)
                   {
                       uint16_t ipix = ((y+yp)<<8)+xp+x;
                        _renderedBitmap[ipix] = color;
                        _renderedBitmap32[ipix] = _paletteRGBA[color];
                   }
                   bm<<=1;
                }
            }
        }




    } // loop per sprites


}
