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

std::string timeFromSample(uint64_t i,unsigned int freq)
{
    stringstream ss;
    uint64_t nbs = i/freq;
    uint64_t decs = i-(nbs*freq);
    double frest = (double)decs/ (double)freq;
    if(nbs>59)
    {
        uint64_t nbm = nbs/60;
        nbs -= (nbm*60);
        ss << nbm << "m " << nbs << "s ";
    } else
    {
        ss << nbs << "s ";
    }
     ss << frest;
     return ss.str();

}

void SC3KSoundAndBytes::soundToBits(const std::vector<signed short> &soundVector, unsigned int freq)
{
    // average value that should fit frequency.
    //float freqdiv = (float)freq * (1.0f/44100.0f);
  //  const unsigned int maxLengthForThisFreq =(unsigned int)( 32.0f * freqdiv);
  //  const unsigned int middleLength = (unsigned int)(15.0f * freqdiv);

    // 2400Hz for one
    // give perfect numbers for 44100 and 22050
//    const uint64_t tapefreq = 2400*2; // *2 because need half wavelength length
//    const uint64_t tapefreqS16 = tapefreq<<16;
//    uint64_t samplePerWaveS16 = tapefreqS16 / freq;
//    uint64_t freqrunS16=0;

    uint32_t magfreq = (freq<<8);
    uint32_t freqstep = (magfreq / (2400*2))>>8; // (1176 for 22050) // frq/4
    uint32_t freqstep2 = (magfreq / (1200*2))>>8;
    uint32_t midLength = (freqstep+ freqstep2)>>1;
//    uint32_t idealLength1 = (uint32_t) (magstep>>8);

    cout << "maxLengthForThisFreq:"<< freqstep << endl;
   // cout << "middleLength:"<< middleLength << endl;


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

    enum sMode {
        sm_SearchBusy=0,
        sm_SearchFirstZero,
        sm_Data
    };

    sMode busymode=sm_SearchBusy; // means search for new ones chain to synchrnoize, no data.


   // cout << "min: " << vmin << "  max: "<< vmax << endl;
    unsigned int lastSwapPos =0;
    bool lastSwapState = false;
    // log statelength
    // 1 as 0101 ,     0 as 0011
    int accumlong=0;
    int accumshort=0;
    bool hasFirstZero=false;
    unsigned int prevStateLength=0,stateLength=0;
    uint64_t accumBusyStateLength=0;

    for(size_t i=0; i<soundVector.size() ; i++)
    {
        signed short s = soundVector[i];
        if(s>-512 && s<512) s=0;

/*        signed short mind = s- vmin;
        signed short maxd = s- vmax;
        if(mind<0) mind=-mind;
        if(maxd<0) maxd=-maxd;
        */
//        bool nearmax= (maxd<mind);
        bool nearmax = (s>0);
        if(nearmax != lastSwapState)
        {

            if(i-lastSwapPos<3 && busymode != sm_SearchBusy  ) // could be an error peak...
            {
                continue;
            }
            prevStateLength = stateLength;
            stateLength = i-lastSwapPos;


            unsigned int curfreq = prevStateLength+stateLength;

            if(curfreq>=freqstep2-2 && curfreq<=freqstep2+2)
            {
                accumBusyStateLength++;
                if(accumBusyStateLength>1200)
                {
                    if(busymode != sm_SearchFirstZero)
                    {
                        busymode = sm_SearchFirstZero;
                        cout << " busy found at: " << timeFromSample(i,freq) << endl;
                    }
                }
            } else
            {
                // can be first zero
//                if(busymode != sm_SearchBusy )
//                {
//                    cout << " busy mode lost at: " << timeFromSample(i,freq) << endl;
//                }
//                busymode = sm_SearchBusy;
                accumBusyStateLength=0;
            }

         //   cout << "sl:"<< stateLength << endl;
             if(busymode == sm_SearchFirstZero )
             {
                if(stateLength<freqstep2+3 && stateLength>freqstep-3) // >: first found or empty data
                {
                    if(stateLength>midLength)
                    {
                        if(!hasFirstZero)
                        {
                            cout << " found 0 at: " << timeFromSample(i,freq) << endl;

                            // next routines wants its busy one lines before first zero...
                            // let's set that magically
                            for(int j=0;j<11;j++)  m_streambits.push_back(1);

                        }
                        hasFirstZero = true;
                        accumlong = 0;
                        accumshort = 0;
                        busymode = sm_Data;
                    }
                }
             }
             if(busymode == sm_Data )
             {
                 if(stateLength<freqstep2+6 && stateLength>freqstep-3) // >: first found or empty data
                 {
                    if(stateLength<midLength)
                    {
                        accumlong=0;
                        accumshort++;

                        if(accumshort==4)
                        {
                            m_streambits.push_back(1); // because long list of 1 at start are just void.
                            accumshort=0;
                        }
                    } else{
                        accumlong++;
                        accumshort=0;

                        if(accumlong==2)
                        {
                            m_streambits.push_back(0); // because long list of 1 at start are just void.
                            accumlong=0;
                        }

                    }

                } else
                {
                    if(busymode != sm_SearchBusy)
                    {
                        cout << " lost data because of length:"<< stateLength<< " at: " << timeFromSample(i,freq) << endl;
                        busymode=sm_SearchBusy;
                        hasFirstZero = false;
                    }
                }
             }


            lastSwapPos = i;
            lastSwapState = nearmax;

            // for 44100Hz:  stateLength (8,8,9,10) or (17,18,19,20)
        }
    }

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
    int conseqones=0;
    for(size_t j=0;j<m_streambits.size(); j++)
    {
        char b = m_streambits[j]; // 0,1
        if(b!=0) conseqones++;
        else conseqones=0;
        if(conseqones>11 && !busymode)
        {
            // busy mode detected
            busymode = true;
            bc=0;
            bytevalue=0;
            bimask=1;
        }
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
            } else
            {
                // could be fault in start of busy mode, bc still 0
             //   continue;
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
//void SC3KSoundAndBytes::bytesToSound(std::vector<signed short> &soundVector)
//{

//}
