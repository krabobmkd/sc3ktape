#include "TMS9918_SC3KExport.h"
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <iomanip>
using namespace std;
using namespace vchip;

#define VRAMWrite 0x4000


TMS_SC2BitmapNormalizer::TMS_SC2BitmapNormalizer(vchip::TMS9918State &tms)
    :_tms(tms)
{}
int TMS_SC2BitmapNormalizer::normalize()
{       
    return 0;
}

// 2b code, 64l.
/*
// next is copy n(1-64) bytes "as is".
#define CD_CPY 0
// next is n*( b read 8b from current charset)
#define CD_DIC8 1
// change charset high
#define CD_CHGCHST 2
//...
#define CD_ 3
*/
TMS_Compressor::TMS_Compressor(vchip::TMS9918State &tms)
    :_tms(tms)
{

}

// Apply to a rectangle of 32x8 max width,
// with shared dictionnary
struct CompressedVMemArea {
    uint16_t _StartAdress;
    uint8_t _lineWidth; // 8*32 max=256, stand for [1,256]
    uint8_t _nbLines; // char lines, so 24 max.
    vector<uint8_t> _v;
};

// exported data is:
// nbcomp
// ramdic
// img1
//  ...n
// maindic
class CompressedDic {
public:
    // look alikes scores
    struct StreamValues{
        vector<uint8_t> _v;

        StreamValues() {}
        StreamValues(const StreamValues &o) {_v = o._v; }
        StreamValues operator=( const StreamValues& r ) {   _v = r._v;  return *this; }
        bool operator<(const StreamValues& r ) const {return (_v<r._v);}
        bool operator==( const StreamValues& r ) const {return (_v == r._v);}

        inline bool isSame(const vector<uint8_t> &vmem,uint16_t adr, uint16_t l )
        {
            if((uint16_t)_v.size() != l) return false;
            for(uint16_t i=0;i<l;i++)
            {
                if(_v[i] != vmem[adr+i]) return false;
            }
            return true;
        }
        inline bool contains(const StreamValues &sv )
        {
            const vector<uint8_t> &m=sv._v;
            if(m.size()==0) return false;
            if(m.size()>_v.size()) return false;
            for(size_t j=0;j<_v.size()-m.size() ;j++)
            {
                bool isIn=true;
                for(size_t i=0;i<m.size() ;i++)
                {
                    if(_v[j+i] != m[i])
                    {
                        isIn=false;
                        break;
                    }
                }
                if(isIn) return true;
            }
            return false;
        }
    };
    struct StreamSort{
        StreamSort() : _score(0) {}
        StreamSort(const StreamSort &o) {
            _score = o._score;
            _sv = o._sv;
        }
        uint32_t _score; StreamValues _sv;
    };
    //
    map<StreamValues,uint32_t> _streamScores;
    // final dic
    vector<uint8_t> _bin;
    // prefs
    int _minStreamLength=3;

    void feed( const vector<uint8_t> &vmem, uint16_t adr, uint16_t l )
    {
        // note l should be 2x6kb most of the time
        // only keep those whose score >1

        for(uint16_t maxStreamLength=_minStreamLength; maxStreamLength<=(32+3); maxStreamLength++ )
        {
            uint16_t iae = adr+l;
            for(uint16_t ia=adr ; ia<iae ; ia++ )
            {
                uint16_t sl=maxStreamLength;
                if(sl>(iae-ia)) sl=iae-ia;

                StreamValues sv;
                sv._v = vector<uint8_t>(vmem.begin()+ia,vmem.begin()+ia+sl);
                // - - - if too much of same value (>3), do not use for compression data
                {
                    bool tooMuchContiguous=false;
                    int currl=1;
                    uint8_t lastv= sv._v[0];
                    for(size_t ic=1;ic<sv._v.size();ic++)
                    {
                        if(sv._v[ic] == lastv)
                        {
                            currl++;
                            if(currl==3)
                            {   tooMuchContiguous=true;
                                break;
                            }
                        } else
                        {
                            lastv=sv._v[ic];
                            currl=1;
                        }
                    }
                    if(tooMuchContiguous) continue;
                }
                // - - -
                int occurences=1;
                for(uint16_t ib=ia+sl ; ib<(iae-sl) ; ib++ )
                {
                    if(sv.isSame(vmem,ib,sl)) occurences++;
                }
                if(occurences>1) {
                    // -1 because dic is growing
                    int32_t score = (int32_t)( (occurences-1) * (int32_t)sv._v.size());
                    score -= occurences*3; // minus size of calls needed (actual 2.25)
                    if(score>0)
                    {
                        _streamScores[sv] = (uint32_t) score;
                    }
                    // (occurences-1) * (uint32_t)sv._v.size();
                }
            }
        }
    } // end of feed()
    void compile()
    {
        // sort best scores
         vector<StreamSort> sorted(_streamScores.size());

        size_t i= 0;
        map<StreamValues,uint32_t>::iterator it = _streamScores.begin();
        while(it != _streamScores.end() )
        {
            sorted[i]._score = it->second;
            sorted[i]._sv = it->first;
            it++;
            i++;
        }
        std::sort(sorted.begin(), sorted.end(), []
                            (const StreamSort &a,const StreamSort &b ) {
                return(a._score > b._score);
            });
        cout << "nb stream before cull:"<< sorted.size() << endl;
        // remove useless inner stream versions and increase scores
        for(int is=sorted.size()-1;is>=0;is--)
        {
            StreamSort &s = sorted[is];
            for(int ib=0; ib<is;ib++)
            {
                StreamSort &sb = sorted[ib];
                if(sb._sv.contains(s._sv))
                {
                    // remove s
                    sb._score += s._score;
                    sorted.erase(sorted.begin()+is); // realloc, s and sb are no more correct.
                }
            }
        }
        cout << "nb stream after cull:"<< sorted.size() << endl;
        // re-sort atfer culling because of gain change.
        std::sort(sorted.begin(), sorted.end(), []
                            (const StreamSort &a,const StreamSort &b ) {
                return(a._score > b._score);
            });

        // display
       /* cout << "sorted occurences:" << sorted.size() << endl;
        for(i=0;i<sorted.size() && i<8 ; i++)
        {
            cout << "score:"<<sorted[i]._score << "\t l:"<< sorted[i]._sv._v.size() << "\t [";
            for(size_t j=0;j<sorted[i]._sv._v.size() && j<20 ; j++)
            {
                cout << " ," << (uint32_t) sorted[i]._sv._v[j];
            }
            cout << "]" << endl;
        }*/
        // final dic bin, longer chains at end
        _bin.clear();
        uint32_t finalgain=0;
        for(int j=(int)sorted.size()-1;j>=0;j--)
        {
            StreamValues &strvl =sorted[j]._sv;
            _bin.insert(_bin.end(),strvl._v.begin(),strvl._v.end());
            finalgain += sorted[j]._score;
        }
        cout << "dic compiled size:" <<_bin.size() << " gain expected:"<< finalgain << endl;

    } // end of compile
    void compress( const vector<uint8_t> &vmem, uint16_t adr, uint16_t l, vector<uint8_t> &comp )
    {
        if(_bin.size()==0)
        {
            throw runtime_error("compile before compress");
        }
        // first let jump to next
        comp.push_back( 0);
        comp.push_back( 0);
        // first put start adr

        uint16_t adrw = adr | VRAMWrite; // needed by z80 io port to write video mem.
        comp.push_back( (uint8_t) (adrw & 255));
        comp.push_back( (uint8_t) ((adrw>>8) & 255));

        //note for size repair: 1 charset: 256*8=2kb ->11bits 4k dico 12b

        // 1b gives the 4 next chunk types 4x 2b()->next 4 chunk
        // 0-> 1b n bytes (1,256), 1b value
    #define CD_MEMSET 0
    #define CD_COPY 1
    #define CD_DICO1 2
    #define CD_DICO2 3
        // 1-> 1b nbytes (1,256) , values to copy.
        // 2-> compr1 dico: 2b: 10b address /6b length, ->1kb, 64b copy max
        // 3-> compr2 dico: 2b: 12b adress /4b l ->4k, 16b copy max.

        uint16_t i=0;
        int byteTypepart=0;
        uint8_t bpart=0;

        size_t flagsAdress = comp.size();
        comp.push_back(0);
        while(i<l)
        {
            if(byteTypepart==4)
            {
                byteTypepart=0;
                comp[flagsAdress] = bpart;

                flagsAdress = comp.size();
                comp.push_back(0);
            }


            uint16_t lleft = l-i;
            if(lleft<=3)
            {
                // left at end just copy
                //..
                comp.push_back(lleft-1);
                for(uint16_t j=0;j<lleft;j++) comp.push_back(vmem[adr+i+j]);

                bpart >>=2;
                bpart |= (CD_COPY<<6);
                byteTypepart++;
                i+=lleft;
                continue;
            }
            // if same consecutive data, memset mode
            uint8_t v=vmem[adr+i];

            if((v==vmem[adr+i+1]) && v==vmem[adr+i+2])
            {
                uint16_t consl = 3;
                while(consl<256 && (i+consl)<l && vmem[adr+i+consl] == v ) consl++;

                comp.push_back(consl-1);
                comp.push_back(v);

                cout << "push fill:" << (int)v << " l:" << consl << endl;

                i+= consl;
                bpart >>=2;
                bpart |= (CD_MEMSET<<6);
                byteTypepart++;

                continue;
            }
            // search next consecutive data or end, and propose copy.
            uint16_t inext=i;
            while(inext<l-3)
            {
                uint8_t vt = vmem[adr+inext];
                uint16_t itt=1;
                while(itt<l-3)
                {
                    uint8_t vtt = vmem[adr+inext+itt];
                    if(vtt!=vt) break;
                    itt++;
                }
                if(itt>=3) break;
                inext++;
            }
             cout << "dist to next:" << (inext-i)<< endl;
            if(inext==i) continue; // no copy , fill to fill.
            //  do copy between
            {
                uint16_t il = inext-i;
                if(il>256) il=256;
                comp.push_back( (uint8_t)(il-1));
                for(uint16_t j=0;j<il;j++) comp.push_back(vmem[adr+i+j]);

                cout << "push copy:" << (int)v << " l:" << il << endl;

                bpart >>=2;
                bpart |= (CD_COPY<<6);
                byteTypepart++;
                i+=il;
                continue;
            }
            /*
            // search for most long consecutive data  >2 in dic
            int maxlfound=0;
            int occfound=-1;
            for(int is=0;is<(int)_bin.size()-3;is++)
            {
                int thislength=0;
                while(vmem[adr+i+thislength]== _bin[is+thislength] &&
                      is+thislength<(int)_bin.size() &&
                      i+thislength<l
                      )
                {
                    thislength++;
                }
                if(thislength>maxlfound)
                {
                    maxlfound = thislength;
                    occfound = is;
                }

            }
            if(maxlfound>2)
            {
                // keep dic ref.
                i+= maxlfound;
            } else
            {
                // no dic ref found
            }
            */


//            uint16_t adr2=adr;
            // if not found, copy.

            //
            //
            // long fill cases, search for cons. same values>2
            // search in dic, must be there

        } // end while
        if(byteTypepart>0)
        {
            while(byteTypepart<4){
                bpart>>=2;
                byteTypepart++;
            }
            comp[flagsAdress] = bpart;
        }
        // ultimately set jump size
        uint16_t s = (uint16_t)comp.size() -2;
        comp[0] = (uint8_t)s ;
        comp[1] = (uint8_t)(s>>8) ;

    } // end compress()

    // for testing
    void deCompress( vector<uint8_t> &vmem,const vector<uint8_t> &comp )
    {
        if(comp.size()<6) return;

        // get adress of start of decomp
        uint16_t cs = (((uint16_t)comp[0]) |  (((uint16_t)comp[1])<<8))+2;
        uint16_t adr = ((uint16_t)comp[2]) |  (((uint16_t)comp[3])<<8);
       // adr &= (~VRAMWrite);
        // again betterto address the 16k of VDP mem
        adr &= 0x3fff;

        uint16_t i=4;

        uint8_t typeparts4 =0; // scomp[i];
        uint8_t typei = 4;
        while(i<cs)
        {
            if(typei==4)
            {
                typei=0;
                typeparts4 = comp[i]; i++;
            }
            if((typeparts4 & 3)==0 ) // memset
            {
                // fill
                uint16_t l = comp[i]; i++;
                uint8_t v = comp[i]; i++;
                for(uint16_t j=0;j<=l;j++)
                {
                    vmem[adr]=v;
                    adr++;
                }
                typeparts4>>=2;
                typei++;
                continue;
            }
            // else copy n
            //re if((typeparts4 & 3)==CD_COPY) // copy
            {
                uint16_t l = comp[i]; i++;
                for(uint16_t j=0;j<=l;j++)
                {
                    vmem[adr]=comp[i]; i++;
                    adr++;
                }

                typeparts4>>=2;
                typei++;
                continue;
            }

        } // main decomp while loop


    }
}; // end of Dic class

extern  int compressData(const vector<uint8_t> &vdata, vector<uint8_t> & compressed);


void TMS_Compressor::compressGraphics2()
{

    // compress images in ram table using dictionary
    uint32_t nbchanges = _tms.normalizeColor1To0();
    nbchanges += _tms.normalizeColorForCompression();
   // _tms.toVerticalTiles();
    cout << "normalize bitmap nbchange: " << nbchanges << endl;

    const vector<uint8_t> &vmem= _tms.vmem();

    vector<uint8_t> vbm(vmem.begin()+0,vmem.begin()+(6*1024));
    vector<uint8_t> bm_comp;
    int res = compressData(vbm,bm_comp);

    vector<uint8_t> vcl(vmem.begin()+0x2000,vmem.begin()+(0x2000+6*1024));
    vector<uint8_t> cl_comp;
     res = compressData(vcl,cl_comp);
   cout << "final size bm+cl:" <<(int)( bm_comp.size() + cl_comp.size()) << endl;


/*

    CompressedDic bmDic;
  //  CompressedDic clDic;
    //CompressedVMemArea vma1;
    bmDic.feed(vmem,0, 1024*6);
    bmDic.feed(vmem,8*1024, 1024*6);
    bmDic.compile();
    //clDic.compile();
    //feedDic(vmem,dic,vma1, 0, 1024*6);
    //

    bmDic.compress(vmem,0x0000,1024*6,_comp_bm);
    bmDic.compress(vmem,0x2000,1024*6,_comp_cl);
// bmDic._bin.size() +
    cout << "final size:" <<(int)( _comp_bm.size() + _comp_cl.size()) << endl;



    // test decompress
    _tms.graphics2BitmapClear();
    bmDic.deCompress(_tms.vmem(),_comp_bm);
    bmDic.deCompress(_tms.vmem(),_comp_cl);
    */

}
void TMS_Compressor::exportAsm(std::ostream &ofs, std::string labelName)
{
    ofs<<"; Exported from pixer\n";
    ofs << "; compressed bitmap:\n";
    exportAsm(ofs,labelName,_comp_bm,"bm");
    ofs << "; compressed colors:\n";
    exportAsm(ofs,labelName,_comp_cl,"cl");
}

void TMS_Compressor::exportAsm(std::ostream &ofs, std::string labelName,
               const std::vector<uint8_t> &cv, std::string stype)
{
//finnaly no label    ofs<< labelName << "_" << stype << ":\n";
    uint16_t j=0;
    ofs << setfill('0') << setw(2) << right << hex;
    ofs << "\t.db ";
    for(uint32_t i=0;i<(uint32_t)cv.size();i++)
    {
        if(j==64)
        {
            j=0;
            ofs << "\n\t.db ";
        }
        if(j>0) ofs << ",";
        ofs <<"$" << (int)cv[i];

        j++;
    }
    ofs << "\n";
 //no label   ofs<< labelName << "_" << stype << "_end:\n\n";
}
