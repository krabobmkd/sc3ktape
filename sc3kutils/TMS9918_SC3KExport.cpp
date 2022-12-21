#include "TMS9918_SC3KExport.h"
#include <vector>
#include <map>
using namespace std;
using namespace vchip;

// 2b code, 64l.

// next is copy n(1-64) bytes "as is".
#define CD_CPY 0
// next is n*( b read 8b from current charset)
#define CD_DIC8 1
// change charset high
#define CD_CHGCHST 2
//...
#define CD_ 3

TMS_SC3KExport::TMS_SC3KExport(vchip::TMS9918State &tms)
    :_tms(tms)
{

}

void TMS_SC3KExport::doExport(std::ostream &ofs)
{

    // compress images in ram table using dictionary
    vector<uint8_t> dic;

    const vector<uint8_t> &vmem= _tms.vmem();


}
