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
    double frest = 1.0/(double)freq;
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
    // 2400Hz for one
    // give perfect numbers for 44100 and 22050
    const uint64_t tapefreq = 2400*2; // *2 because need half wavelength length
    const uint64_t tapefreqS16 = tapefreq<<16;
    uint64_t samplePerWaveS16 = tapefreqS16 / freq;
    uint64_t freqrunS16=0;
    uint32_t idealLength1 = (uint32_t) (samplePerWaveS16>>16);
    uint32_t idealLength2 = (uint32_t) (samplePerWaveS16>>15);

    cout << "samplePerWaveS16:" << samplePerWaveS16 << endl;

    uint64_t silenceVolumeLimit = idealLength1*512;

    freqrunS16 = samplePerWaveS16>>2; //test


    // average value that should fit frequency.
    double freqdiv = (double)freq * (1.0/44100.0);


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

    int64_t svolumeUpAcc=0;
    int64_t svolumeDownAcc=0;

    char st[4];
    int istab=0;

    enum sMode {
        sm_SearchBusy=0,
        sm_SearchFirstZero,
        sm_Data
    };

    sMode busymode=sm_SearchBusy; // means search for new ones chain to synchrnoize, no data.

    int64_t accumLooksLikeBusyMode01=0;
    int64_t accumLooksLikeBusyMode10=0;

 size_t lasti=0; // test
    for(size_t i=0; i<soundVector.size() ; i++)
    {
        signed short s = soundVector[i];
        //v2: tapes have low noise around zero
        if(s>-512 && s<512) s=0;

        // uint64_t freqrun=freqrunS16>>16;


        // would manage imperfect scale signal
        //signed short mind = s- vmin;
        //signed short maxd = s- vmax;
        //if(mind<0) mind=-mind;
        //if(maxd<0) maxd=-maxd;
        //bool nearmax= (s>=0);
        if(s>=0) {
            svolumeUpAcc += (int64_t)s;
        } else
        {
            svolumeDownAcc -= (int64_t)s;
        }

        if(freqrunS16>tapefreqS16)
        {
           size_t idif = i-lasti;
           cout << "dif:" << idif << endl;
            lasti = i;
            freqrunS16 -= tapefreqS16;
            // goes here once per signal
            if(svolumeUpAcc<silenceVolumeLimit && svolumeDownAcc<silenceVolumeLimit)
            {
                if(i>44100*9 && i< 44100*10)
                {
                    cout << "FLAT"<< endl;
                }
                st[istab] = 2; // flat
                // should set in "busy mode"
                if(busymode != sm_SearchBusy)
                {
                    busymode = sm_SearchBusy;
                    cout << "data flat, restart search at: " << timeFromSample(i,freq) << endl;
                }
            } else
            {
                if(i>44100*9 && i< 44100*10)
                {
                    cout << "up: " <<  svolumeUpAcc << "   down: "<< svolumeDownAcc<< endl;

                }
                st[istab] =  (svolumeUpAcc>svolumeDownAcc)?1:0;
            }
            svolumeUpAcc=0;
            svolumeDownAcc=0;

            istab++;
            if((istab == 2 && busymode != sm_Data)
               || (istab == 4 && busymode == sm_Data) )
            {
                istab=0;
                // 1 as 0101 ,     0 as 0011
                if(busymode == sm_SearchBusy)
                {   // search if busy mode

                    if(i>44100*9 && i< 44100*10)
                    {
                        string s;
                        if(st[0]==0) s="0";
                        if(st[0]==1) s="1";
                        if(st[0]==2) s="2";

                        cout << "st: " << s;
                        if(st[1]==0) s="0";
                        if(st[1]==1) s="1";
                        if(st[1]==2) s="2";
                        cout << s << endl;

                    }
                    if(st[0]==0 && st[1]==1 )
                    {
                        accumLooksLikeBusyMode01++;
                    } else accumLooksLikeBusyMode01=0;

                    if(st[0]==1 && st[1]==0 )
                    {
                        accumLooksLikeBusyMode10++;
                    } else accumLooksLikeBusyMode10=0;

                    if(accumLooksLikeBusyMode01>(11*3*2))
                    {
                        cout << "found busy mode at: " << timeFromSample(i,freq) << endl;
                        busymode = sm_SearchFirstZero;
                    }

                    if(accumLooksLikeBusyMode10>(11*3*2))
                    {
                        //still busymode = sm_SearchBusy;
                        cout << "found step to busy mode at: " << timeFromSample(i,freq) << endl;
                        //busymode = sm_SearchSynchro;
                        freqrunS16 += tapefreqS16; // hoppefully activate 0101

                        accumLooksLikeBusyMode01 = accumLooksLikeBusyMode10-8;
                        accumLooksLikeBusyMode10 = 0;
                    }

                } else if (busymode == sm_SearchFirstZero)
                {
                    // should try to affinate  freqrunS16 to sign switch
                    // ...

                    // first ones could be
                    if(st[0]==0 && st[1]==1 )
                    {
                        accumLooksLikeBusyMode01++;
                    } else
                    {
                        if(st[0]==0 && st[1]==0 )
                        {
                            // found busy sould: 8.5 then 12.64
                            // should 11.4175 then  15.663
                            cout << "first data found at: " << timeFromSample(i,freq) << endl;
                            busymode = sm_Data;
                            istab = 2;
                        } else
                        {
                            cout << "lost busy mode at: " << timeFromSample(i,freq) << endl;
                            accumLooksLikeBusyMode01=0;
                            busymode = sm_SearchBusy;
                        }
                    }
                } else
                if(busymode == sm_Data)
                {
                    if(st[0]==0 && st[1]==0 && st[2]==1 && st[3]==1 )
                    {
                        m_streambits.push_back(0);
                    } else if(st[0]==0 && st[1]==1 && st[2]==0 && st[3]==1 )
                    {
                        m_streambits.push_back(1);
                    } else
                    {
                        bool foundflat = (st[0]==2) || (st[1]==2) || (st[2]==2) || (st[3]==2);
                        cout << "lost data at: " << timeFromSample(i,freq) << endl;
                        if(foundflat) cout << "(found flat)" << endl;
                    }

                }

            } // end if 4x 1200 hz sample read


        }


/*
        if(nearmax != lastSwapState)
        {
            unsigned int stateLength = i-lastSwapPos;

            cout << "stateLength:"<< stateLength << "ideal:" << idealLength1 << endl;

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
        */
        freqrunS16 += samplePerWaveS16;
    }

    // test thing
//    int nbcl=0;
//    int nbch=0;
//    bool hasfirstone=false;
//    size_t firstOnePos=0;

//    unsigned int nbcSinceFirstOne = 0;
//    for(size_t j=3600;j</*m_streambits.size()*/3600+12*40; j++)
//    {
//       // if(nbcSinceFirstOne>400) break;
//         if(m_streambits[j]>0)
//         {
//            hasfirstone = true;
//            firstOnePos = j;
//         }
//        //if(hasfirstone)
//        {
//            cout << ((m_streambits[j]==0)?"0":"1") ;

//            nbcl++;
//            nbch++;
//            nbcSinceFirstOne++;
//            //if(nbch==11) {
//            //    nbch=0;
//            //    cout << " ";
//            //}
//            if(nbcl==1) {
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
