#ifndef _TMS9918SC3KEXPORT_H_
#define _TMS9918SC3KEXPORT_H_

#include "TMS9918State.h"
#include <iostream>

class TMS_SC3KExport {
public:
    TMS_SC3KExport(vchip::TMS9918State &tms);

    void doExport(std::ostream &ofs);
protected:
    const vchip::TMS9918State &_tms;
};

#endif
