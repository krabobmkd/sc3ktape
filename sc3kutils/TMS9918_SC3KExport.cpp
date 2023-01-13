#include "TMS9918_SC3KExport.h"
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace std;
using namespace vchip;



TMS_SC2BitmapNormalizer::TMS_SC2BitmapNormalizer(vchip::TMS9918State &tms)
    :_tms(tms)
{}
int TMS_SC2BitmapNormalizer::normalize()
{
    return 0;
}

// 2b code, 64l.

// next is copy n(1-64) bytes "as is".
#define CD_CPY 0
// next is n*( b read 8b from current charset)
#define CD_DIC8 1
// change charset high
#define CD_CHGCHST 2
//...
#define CD_ 3

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

        for(uint16_t maxStreamLength=_minStreamLength; maxStreamLength<=256; maxStreamLength++ )
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
                    _streamScores[sv] = (occurences-1) * (uint32_t)sv._v.size();
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
        for(int j=(int)sorted.size()-1;j>=0;j--)
        {
            StreamValues &strvl =sorted[j]._sv;
            _bin.insert(_bin.end(),strvl._v.begin(),strvl._v.end());
        }
        cout << "dic compiled size:" <<_bin.size() << endl;

    } // end of compile
    void compress( const vector<uint8_t> &vmem, uint16_t adr, uint16_t l, vector<uint8_t> &comp )
    {
        if(_bin.size()==0)
        {
            throw runtime_error("compile before compress");
        }
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
            if(lleft<3)
            {
                // left at end just copy
                //..
                comp.push_back(lleft);
                for(uint16_t j=0;j<lleft;j++) comp.push_back(vmem[adr+i+j]);

                bpart <<=2;
                bpart |= CD_COPY;
                byteTypepart++;
                continue;
            }
            // if same consecutive data, memset mode
            uint8_t v=vmem[adr+i];

            if((v==vmem[adr+i+1]) && v==vmem[adr+i+2])
            {
                uint16_t consl = 3;
                while(consl<256 && (i+consl)<l && vmem[adr+i+consl] == v ) consl++;

                comp.push_back(consl);
                comp.push_back(v);

                i+= consl;
                bpart <<=2;
                bpart |= CD_MEMSET;
                byteTypepart++;

                continue;
            }
            // search next consecutive data or end, and propose copy.
            uint16_t inext=i+1;
            while(inext<l)
            {

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
            comp[flagsAdress] = bpart;
        }

    } // end compress()

}; // end of Dic class
void TMS_Compressor::doExport(std::ostream &ofs)
{

    // compress images in ram table using dictionary

    const vector<uint8_t> &vmem= _tms.vmem();

    CompressedDic bmDic;
  //  CompressedDic clDic;
    //CompressedVMemArea vma1;
    bmDic.feed(vmem,0, 1024*6);
    bmDic.feed(vmem,8*1024, 1024*6);
    bmDic.compile();
    //clDic.compile();
    //feedDic(vmem,dic,vma1, 0, 1024*6);
    //
    vector<uint8_t> bmComp,clComp;
    bmDic.compress(vmem,0,1024*6,bmComp);
    bmDic.compress(vmem,0,1024*6,clComp);
    cout << "final size:" <<(int)(bmDic._bin.size() + bmComp.size() + clComp.size()) << endl;
}
