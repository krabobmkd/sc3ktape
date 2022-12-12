#ifndef _TMS9918STATE_H_
#define _TMS9918STATE_H_

#include <inttypes.h>
#include <vector>

class TMS9918State
{
public:
    TMS9918State();

    void setMode2Default();
    void updateRender();

    inline int pixelWidth() const {
        return _pixelWidth;
    }
    inline int pixelHeight() const {
        return _pixelHeight;
    }
    inline const uint32_t *pixelRGBA() const {
        return _renderedBitmap32.data();
    }

    // - - - - -
    inline uint16_t adress_NameTable() {
        return (uint16_t)_regs._2._NameTableBaseAdress * 0x400 ;
    }
    inline uint16_t adress_ColorTable() {
        return (uint16_t)_regs._3._ColorTableBaseAdress * 0x40 ;
    }
    inline uint16_t adress_PatternGenerator() {
        return (uint16_t)_regs._4._PatternGeneratorBaseAdress * 0x800 ;
    }
    inline uint16_t adress_SpriteAttribs() {
        return (uint16_t)_regs._5._SpriteAttribTableBaseAdress * 0x80 ;
    }
    inline uint16_t adress_SpritePatternGenerator() {
        return (uint16_t)_regs._6._SpritePatternGeneratorBaseAdress * 0x800 ;
    }

protected:
    std::vector<uint8_t> _vmem;
   // std::vector<uint8_t> _vregs;


    struct TMSregs
    {
        struct {
            uint8_t _:6;
            uint8_t _M3_Graphics2:1; // for mode graphics 2 / all M: off graphics1
            uint8_t _External:1;
        } _0;
        struct {
            uint8_t _16k:1;
            uint8_t _ActiveDisplay:1; // blank shows border color.
            uint8_t _InteruptEnable:1;
            uint8_t _M1_textMode:1; // just it for text mode
            uint8_t _M2_Multicolor:1; // just it for multicolor mode
            uint8_t _:1; // reserved
            uint8_t _SpriteSize:1; // 16x16 or 8x8
            uint8_t _SpriteMag:1; // 1 -> sprites X2
        } _1;
        struct {
            uint8_t _:4;
            uint8_t _NameTableBaseAdress:4; // *0x400
        } _2;
        struct {
            uint8_t _ColorTableBaseAdress; // * 0x40
        } _3;
        struct {
            uint8_t _:5;
            uint8_t _PatternGeneratorBaseAdress:3; //* 0x800
        } _4;
        struct {
            uint8_t _:1;
            uint8_t _SpriteAttribTableBaseAdress:7; // * 0x80
        } _5;
        struct {
            uint8_t _:5;
            uint8_t _SpritePatternGeneratorBaseAdress:3; // * 0x800
        } _6;
        struct {
            uint8_t _TextColor:4;
            uint8_t _BackdropColor:4;
        } _7;
        // will usually occupy 2 bytes:
        // 3 bits: value of b1
        // 2 bits: unused
        // 6 bits: value of b2
        // 2 bits: value of b3
        // 3 bits: unused
      //  unsigned char b1 : 3, : 2, b2 : 6, b3 : 2;
    } _regs;

    std::vector<uint32_t> _paletteRGBA;
    std::vector<uint8_t> _renderedBitmap;
    std::vector<uint32_t> _renderedBitmap32;

 //   std::vector<uint8_t> _renderedRGBABitmap;
    int _pixelWidth,_pixelHeight;


    void updateRender_Mode2();
};


#endif
