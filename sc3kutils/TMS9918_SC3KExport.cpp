#include "TMS9918_SC3KExport.h"
#include <vector>

using namespace std;
using namespace vchip;


TMS_SC3KExport::TMS_SC3KExport(vchip::TMS9918State &tms)
    :_tms(tms)
{

}

void TMS_SC3KExport::doExport(std::ostream &ofs)
{

    // compress images in ram table using dictionary
    vector<uint8_t> dic;




}
