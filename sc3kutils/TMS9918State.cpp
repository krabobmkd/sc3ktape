#include "TMS9918State.h"

using namespace std;

TMS9918State::TMS9918State()
    :_vmem(16384,0)
    ,_regs{0}
    ,_pixelWidth(0)
    ,_pixelHeight(0)
{
    _paletteRGBA = {
        0xff000000,
        0xff010101,
        0xff3eb849,
        0xff74d07d,
        0xff5955e0,
        0xff8076f1,
        0xffb95e51,
        0xff65dbef,
        0xffdb6559,
        0xffff897d,
        0xffccc35e,
        0xffded087,
        0xff3aa241,
        0xffb766b5,
        0xffcccccc,
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

void TMS9918State::updateRender()
{
    if(_regs._0._M3_Graphics2 !=0)
    {
       updateRender_Mode2();
    }
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

    size_t nameTableBase =(size_t)this->adress_NameTable();

    // scanning line is more emulatorish.
    uint8_t *pwr= _renderedBitmap.data();
    uint32_t *pwr32= _renderedBitmap32.data();
    uint8_t yb=0;


    for(uint16_t y=0;y<_pixelHeight ; y++)
    {
        uint16_t yl =y&3;
        uint16_t yc =y>>3;
        uint16_t screenBmShift = (yc>>3)<<11; // runs 0,1,2 256*8 each , so 0,2k,4k
        for(uint16_t xc=0;xc<_pixelWidth>>3 ; xc++)
        {
            // get bitmap using tiling
            uint16_t n = (uint16_t) _vmem[nameTableBase+xc+(yc<<5)]; // the char id that point the screen Bm
            uint8_t bm = _vmem[screenBmShift+(n<<3)+yl]; // 8 2c pixels
            // get the 2 colors not using tiling but yet char per char
            uint8_t cl = _vmem[0x2000+((xc+(yc<<5))<<8)+yl];
            uint8_t bgc = cl>>4;
            if(bgc==0) bgc = bgColor;
            uint8_t fgc = cl & 0x0f;
            if(fgc==0) fgc = bgColor;

            for(uint8_t xl=0;xl<8;xl++ )
            {
                uint8_t c = (bm&128)?fgc:bgc;
                *pwr++ = c;
                *pwr32++ = _paletteRGBA[c];
                bm<<=1;
            }
        }
    }

}
