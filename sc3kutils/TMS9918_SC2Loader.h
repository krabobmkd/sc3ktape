#ifndef _TMS9918SC2LOADER_H_
#define _TMS9918SC2LOADER_H_

#include "TMS9918State.h"
#include <iostream>

class TMS_SC2Loader {
public:
    TMS_SC2Loader(vchip::TMS9918State &tms);

    void load(std::istream &ifs);
protected:
    vchip::TMS9918State &_tms;
};


#endif
