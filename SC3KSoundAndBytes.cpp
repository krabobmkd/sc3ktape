#include "SC3KSoundAndBytes.h"
#include <sstream>
#include <iostream>

using namespace std;

SC3KSoundAndBytes::SC3KSoundAndBytes()
{

}

/** convert external sound vector to bytes */
void SC3KSoundAndBytes::soundToBytes(const std::vector<signed short> &soundVector, unsigned int freq)
{
    soundToBits(soundVector,freq);
    bitsToBytes();
}

void SC3KSoundAndBytes::soundToBits(const std::vector<signed short> &soundVector, unsigned int freq)
{
    // average value that should fit frequency.
    float freqdiv = (float)freq * (1.0f/44100.0f);
    const unsigned int maxLengthForThisFreq =(unsigned int)( 32.0f * freqdiv);
    const unsigned int middleLength = (unsigned int)(15.0f * freqdiv);

    cout << "maxLengthForThisFreq:"<< maxLengthForThisFreq << endl;
    cout << "middleLength:"<< middleLength << endl;

    if(soundVector.size()==0) return;
    // get min max
    signed short  vmin = soundVector[0];
    signed short vmax = vmin;
    for(size_t i=1; i< soundVector.size() ; i++)
    {
        signed short s = soundVector[i];
        if(s<vmin) vmin=s;
        if(s>vmax) vmax=s;
    }
   // cout << "min: " << vmin << "  max: "<< vmax << endl;
    unsigned int lastSwapPos =0;
    bool lastSwapState = false;
    // log statelength
    // 1 as 0101 ,     0 as 0011
    int lastZero=0;
    bool hasFirstZero=false;
    for(size_t i=0; i<soundVector.size() ; i++)
    {
        signed short s = soundVector[i];
        signed short mind = s- vmin;
        signed short maxd = s- vmax;
        if(mind<0) mind=-mind;
        if(maxd<0) maxd=-maxd;
        bool nearmax= (maxd<mind);
        if(nearmax != lastSwapState)
        {
            unsigned int stateLength = i-lastSwapPos;
         //   cout << "sl:"<< stateLength << endl;
            if(stateLength<maxLengthForThisFreq && nearmax) // >: first found or empty data
            {
                if(stateLength<middleLength)
                {
                    // short need 2 pass
                    lastZero++;
                    if(lastZero==2)
                    {
                        if(hasFirstZero) m_streambits.push_back(1); // because long list of 1 at start are just void.
                        lastZero=0;
                    }
                } else{
                    m_streambits.push_back(0);
                    hasFirstZero = true;
                    lastZero = 0;
                }

            }

            lastSwapPos = i;
            lastSwapState = nearmax;

            // for 44100Hz:  stateLength (8,8,9,10) or (17,18,19,20)
        }
    }

    // test thing
//    int nbcl=0;
//    int nbch=0;
//    bool hasfirstone=false;
//    size_t firstOnePos=0;

//    unsigned int nbcSinceFirstOne = 0;
//    for(size_t j=0;j<m_streambits.size(); j++)
//    {
//       // if(nbcSinceFirstOne>400) break;
//         if(m_streambits[j]>0)
//         {
//            hasfirstone = true;
//            firstOnePos = j;
//         }
//        //if(hasfirstone)
//        {
//          //  cout << ((m_streambits[j]==0)?"0":"1") ;

//            nbcl++;
//            nbch++;
//            nbcSinceFirstOne++;
//            if(nbch==8) {
//                nbch=0;
//          //      cout << " ";
//            }
//            if(nbcl==40) {
//                nbcl=0;
//                cout << endl;
//            }
//        }
//    }
//    cout << endl;
}
void SC3KSoundAndBytes::bitsToBytes()
{
    stringstream strreport;
    m_vbytes.clear();
    int bc=0; //0->10 , 11 bits.
    int bi=0; // inside byte.
    unsigned char bimask=1;
    unsigned char bytevalue=0;

    m_vbytes.push_back(vector<unsigned char>());

    vector<unsigned char> *pvbytes = &m_vbytes[0];

    // busy mode is that long one value chain at start used check file start on tape.
    bool busymode=true;
    for(size_t j=0;j<m_streambits.size(); j++)
    {
        char b = m_streambits[j]; // 0,1
        if(bc==0) // 'start bit'
        {
            // must be zero.
            if(b !=0)
            {
                if(busymode) continue; // already in busy mode.
                m_vbytes.push_back(vector<unsigned char>());
                pvbytes = &(m_vbytes.back());
                busymode=true;

                cout << endl;
                continue;

           //  strreport << "stream starter not 0 at byte:"<< (int)m_bytes.size()  << "\n";
            }
            bi = 0;
            bimask = 1;
            bytevalue=0;
            busymode = false;
        } else if(bc==9 || bc==10)
        {
            if(b !=1)
            {
          //   strreport << "byte ender not 1 at byte:"<< (int)m_bytes.size()  << "\n";
            }
            if(bc==9)
            {
                pvbytes->push_back(bytevalue);
             //   cout << hex << (int)bytevalue << endl;
            // cout  << setfill('0') << setw(2) << hex << (int)bytevalue << endl;

                bytevalue = 0;
            }
        } else
        {
            // build byte:
            if(b) bytevalue |= bimask;
            bimask <<=1;
        }
        bc++;
        if(bc==11) bc=0;
    }

    // remove wrong start chunk
    if(m_vbytes.size()>0 && m_vbytes[0].size()==1)
    {
        m_vbytes.erase(m_vbytes.begin(),m_vbytes.begin()+1);
    }

}


/** convert bytes to external sound vector */
void SC3KSoundAndBytes::bytesToSound(std::vector<signed short> &soundVector)
{

}
