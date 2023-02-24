#ifndef _TMS9918PETSCII_H_
#define _TMS9918PETSCII_H_

#include <inttypes.h>
#include <vector>
#include <iostream>
#include "TMS9918State.h"

class TMS_PetsciiLoader {
public:
    TMS_PetsciiLoader(vchip::TMS9918State &tms);

    void load(std::istream &ifs);

    vchip::TMS9918State &_tms;
};


#endif
