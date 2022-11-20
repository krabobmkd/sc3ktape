/*            ________  __     __
  ______ ____ \_____  \|  | __/  |______  ______   ____  🖭
 /  ____/ ___\  _(__  <|  |/ \   __\__  \ \____ \_/ __ \
 \___ \\  \___ /       |    < |  |  / __ \|  |_> \  ___/
/____  >\___  /______  |__|_ \|__| (____  |   __/ \___  >
     \/     \/       \/     \/          \/|__|        \/
 .oO Sega SC3000/SK1100 Tape-2-Basic-2-Tape Converter Oo.
      Written by Vic Ferry aka Krabob/Mkd - (c)2022

 This relase is under APACHE LICENSE , Version 2.0 https://www.apache.org/licenses/LICENSE-2.0 */

#ifndef SOUNDREADER_H
#define SOUNDREADER_H

#include <vector>
#include <iostream>

struct SoundVector {
    std::vector<signed short> _soundVector;
    unsigned int    _samplePerSeconds;
};

/** \class ASoundReader
*  Abstract class to check a sound file format and read it to signed short vector.
*  Allow to add extra file format easily.
*/
class ASoundReader
{
public:

    /** read sound vector from fileStream */
    virtual void read(SoundVector &soundVector,std::istream &filestream) =0;

protected:

};

/** \class ASoundReader
*  Abstract class to write a sound file format from signed short vector.
*/
class ASoundWriter
{
public:
    /** read signed short vector */
    virtual void write(std::ostream &ofs, std::vector<unsigned char> &v,int freq) =0;
};


/** \class SoundReader_Wave
* Manage reading Microsoft wave format.
*/
class SoundReader_Wave : public ASoundReader
{
public:
    SoundReader_Wave();

    /** return true if first bytes tells the format is the one managed */
    static bool testStreamFormat(std::istream &filestream) ;

    /** read sound vector from fileStream */
    void read(SoundVector &soundVector,std::istream &filestream) override;

protected:
    std::vector<signed char> m_normalized;

    template<typename T>
    void waveToSignedShortT(std::vector<signed short>  &soundVector,std::istream &ifs,size_t chunksize);

};

/** \class SoundWriter_Wave
* Manage Microsoft wave format.
*/
class SoundWriter_Wave : public ASoundWriter
{
public:
    /** write signed short vector */
    void write(std::ostream &ofs, std::vector<unsigned char> &v,int freq) override;
};


/** \class SoundReaderBroker
*  Will find out which format the file is, and return a valid ASoundReader for this format.
*/
class SoundReaderBroker : public ASoundReader
{
public:
    SoundReaderBroker();

    /** return true if first bytes tells the format is any we managed */
    static bool testStreamFormat(std::istream &filestream) ;

    /** read sound vector from fileStream */
    void read(SoundVector &soundVector,std::istream &filestream) override;

};

#endif // SOUNDREADER_H
