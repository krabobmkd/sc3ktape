#ifndef _TMS9918SC3KEXPORT_H_
#define _TMS9918SC3KEXPORT_H_

#include "TMS9918State.h"
#include <iostream>

/** \class TMS_BitmapNormalizer
    assume that each screen2 bitmap bytes has 4 pixels on maximum,
    else invert both the BM and the colors.
    Would help compression
*/
class TMS_SC2BitmapNormalizer {
    TMS_SC2BitmapNormalizer(vchip::TMS9918State &tms);
    // invert if needed,
    int normalize();
protected:
    const vchip::TMS9918State &_tms;
};

class TMS_Compressor {
public:
    TMS_Compressor(vchip::TMS9918State &tms);

    void doExport(std::ostream &ofs);
protected:
    const vchip::TMS9918State &_tms;
};

#endif
