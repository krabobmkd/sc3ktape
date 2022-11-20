/*            ________  __     __
  ______ ____ \_____  \|  | __/  |______  ______   ____  🖭
 /  ____/ ___\  _(__  <|  |/ \   __\__  \ \____ \_/ __ \
 \___ \\  \___ /       |    < |  |  / __ \|  |_> \  ___/
/____  >\___  /______  |__|_ \|__| (____  |   __/ \___  >
     \/     \/       \/     \/          \/|__|        \/
 .oO Sega SC3000/SK1100 Tape-2-Basic-2-Tape Converter Oo.
      Written by Vic Ferry aka Krabob/Mkd - (c)2022

 This relase is under APACHE LICENSE , Version 2.0 https://www.apache.org/licenses/LICENSE-2.0 */

#include <iostream>
#include <fstream>
#include "SoundReader.h"
#include <stdexcept>
#include <map>
#include <sstream>
#include  <iomanip>

using namespace std;

#define CHUNKID(a,b,c,d) ( ((unsigned int)d)<<24 | ((unsigned int)c)<<16 | ((unsigned int)b)<<8 | (unsigned int)a)

struct TypeHeader {
    unsigned short _AudioFormat;
    unsigned short _NumChannels;
    unsigned int _SampleRate;
     unsigned int _byteRate;
        unsigned short _blockAlign;
        unsigned short _BitsPerSample;
};

bool SoundReader_Wave::testStreamFormat(std::istream &filestream)
{
    filestream.seekg(0,ios::beg);

    char th[4]={0,0,0,0};
    filestream.read(th,4);
    filestream.seekg(0,ios::beg);

    return (th[0]=='R' && th[1]=='I' && th[2]=='F' && th[3]=='F' ) ;
}
SoundReader_Wave::SoundReader_Wave() : ASoundReader()
{
}

template<typename T>
void SoundReader_Wave::waveToSignedShortT(std::vector<signed short> &soundVector,std::istream &ifs,size_t chunksize)
{
    unsigned int vssize = chunksize / sizeof(T);
    if(vssize==0) return;
    vector<T> vs(vssize);
    ifs.read((char *)&vs[0],chunksize);

    soundVector.resize(vssize);
    for( size_t i=0 ; i<vssize ; i++ )
    {
        soundVector[i] = (signed short)vs[i];
    }

}


/** read sound vector from fileStream */
void SoundReader_Wave::read(SoundVector &soundVector,std::istream &filestream)
{
    // default invalid values
    soundVector._soundVector.clear();
    soundVector._samplePerSeconds = 0;

   if(!filestream.good()) throw runtime_error("File not readable");

    struct MiniHeader {
        unsigned int _ChunkId;
        unsigned int _ChunkSize;
    };

    struct Header {
        unsigned int _ChunkId;
        unsigned int _ChunkSize;
        unsigned int _Format;

    };

    filestream.seekg(0,ifstream::end);
    streampos pend = filestream.tellg();
    filestream.seekg(0,ifstream::beg);

    Header header;
    filestream.read((char*)&header, sizeof(header));

   // cout << "header._ChunkId:"<<header._ChunkId << endl;

    if(header._ChunkId != CHUNKID('R','I','F','F') )
    {
        throw runtime_error("NO RIFF HEADER");
    }
    if(header._Format != CHUNKID('W','A','V','E') )
    {
        throw runtime_error("NO WAVE HEADER");
    }

    TypeHeader m_type;

    MiniHeader mhd;
    bool hasdata=false;

    streampos chunkpos = filestream.tellg();

    while(filestream.good() && filestream.tellg()<pend){


        filestream.read((char*)&mhd, sizeof(mhd));
        if(mhd._ChunkSize<=0) {
            throw runtime_error("wave file messing size");
        }
        cout << "chunkid: " << ( (char)(mhd._ChunkId)) << (char)(mhd._ChunkId>>8) << (char)(mhd._ChunkId>>16) << (char)(mhd._ChunkId>>24) << endl;
        cout << "chunksize: "<< mhd._ChunkSize << endl;
        if(mhd._ChunkId == CHUNKID('f','m','t',' '))
        {
            filestream.read((char*)&m_type, sizeof(TypeHeader));

           cout << "_AudioFormat:" << m_type._AudioFormat << endl;
           cout << "_NumChannels:" << m_type._NumChannels << endl;
           cout << "_SampleRate:" << m_type._SampleRate << endl;
           cout << "_byteRate:" << m_type._byteRate << endl;
           cout << "_BitsPerSample:" << m_type._BitsPerSample << endl;


        }
        if(mhd._ChunkId == CHUNKID('d','a','t','a'))
        {
            if(m_type._AudioFormat == 1 &&
               (m_type._BitsPerSample ==16 ||
                m_type._BitsPerSample ==8
                )
                )
            {
               // waveToStreamBits(filestream,mhd._ChunkSize,m_type._SampleRate,m_type._BitsPerSample);

                if(m_type._BitsPerSample==16) waveToSignedShortT<signed short>(soundVector._soundVector,filestream,(size_t)mhd._ChunkSize);
                else if(m_type._BitsPerSample==8) waveToSignedShortT<unsigned char>(soundVector._soundVector,filestream,(size_t)mhd._ChunkSize);

                soundVector._samplePerSeconds = m_type._SampleRate;

            } else
            {
                throw runtime_error("wave ok but only support for 16 and 8 bits mono waves");
            }
            hasdata = true;
        }

        chunkpos += (long long) (mhd._ChunkSize + 8);
        filestream.seekg(chunkpos,ifstream::beg);
    }
    if(!hasdata) throw runtime_error("no data chunk found in wave - binary mess. ");


}
/** write signed short vector */
void SoundWriter_Wave::write(std::ostream &ofs, std::vector<unsigned char> &v,int freq)
{
    struct MiniHeader {
        unsigned int _ChunkId;
        unsigned int _ChunkSize;
    };

    struct Header {
        unsigned int _ChunkId;
        unsigned int _ChunkSize;
        unsigned int _Format;

    };
    Header header;
    header._ChunkId =  CHUNKID('R','I','F','F');
    header._ChunkSize = sizeof(header) +

                        sizeof(MiniHeader)+
                        sizeof(TypeHeader)+

                        sizeof(MiniHeader)+
                        v.size() -8
                            ;
    header._Format =  CHUNKID('W','A','V','E');
    ofs.write((const char *)&header,sizeof(header));

    MiniHeader mhd1;
    TypeHeader typeH;

    mhd1._ChunkId = CHUNKID('f','m','t',' ');
    mhd1._ChunkSize = sizeof(TypeHeader);
    typeH._AudioFormat = 1;
    typeH._NumChannels = 1;
    typeH._SampleRate = freq;
    typeH._byteRate = freq; //?
    typeH._blockAlign = 1; //?
    typeH._BitsPerSample = 8;

    ofs.write((const char *)&mhd1,sizeof(mhd1));
    ofs.write((const char *)&typeH,sizeof(typeH));

    MiniHeader mhd2;
    mhd2._ChunkId = CHUNKID('d','a','t','a');
    mhd2._ChunkSize = (int)v.size();

    ofs.write((const char *)&mhd2,sizeof(mhd2));
    ofs.write((const char *)&v[0],v.size());

}

SoundReaderBroker::SoundReaderBroker()
{
}


/** return true if first bytes tells the format is any we managed */
bool SoundReaderBroker::testStreamFormat(std::istream &filestream)
{
    if(SoundReader_Wave::testStreamFormat(filestream))
    {
        return true;
    }
    // add other format heres
    return false;
}

void SoundReaderBroker::read(SoundVector &soundVector,std::istream &filestream)
{
    if(SoundReader_Wave::testStreamFormat(filestream))
    {
        SoundReader_Wave sr;
        sr.read(soundVector,filestream);
        return;
    }
    // add other format here

    throw runtime_error("Unsupported sound file format");

}
