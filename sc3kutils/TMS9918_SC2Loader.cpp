#include "TMS9918_SC2Loader.h"
#include <stdexcept>
using namespace std;
using namespace vchip;

TMS_SC2Loader::TMS_SC2Loader(TMS9918State &tms)
    : _tms(tms)
{}

void TMS_SC2Loader::load(std::istream &ifs)
{
    _tms.setMode_Graphics2Default();
    /*
    sc2 file is essentially a memory dump of the seven (7) video registers
    followed by 14K graphics data in video ram, aka VRAM
    */
// sc2 size are 14343 (7 regs+14kb)
    ifs.seekg(0,ios::end);
    long long bsize = (long long)ifs.tellg();
    if(bsize<(7+14*1024)) throw runtime_error("SC2Loader: file not long enough");

    ifs.seekg(0,ios::beg);
  //  TMS9918State::TMSregs &regs = _tms.regs();
  //  ifs.read((char *)&regs._0,7);
    uint8_t sc2regs[7];
    ifs.read((char *)sc2regs,7);
    _tms.setMode_Graphics2Default();
    ifs.read((char *)_tms.vmem().data(),bsize-7 );
}
