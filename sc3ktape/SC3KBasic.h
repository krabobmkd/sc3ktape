/*            ________  __     __
  ______ ____ \_____  \|  | __/  |______  ______   ____  🖭
 /  ____/ ___\  _(__  <|  |/ \   __\__  \ \____ \_/ __ \
 \___ \\  \___ /       |    < |  |  / __ \|  |_> \  ___/
/____  >\___  /______  |__|_ \|__| (____  |   __/ \___  >
     \/     \/       \/     \/          \/|__|        \/
 .oO Sega SC3000/SK1100 Tape-2-Basic-2-Tape Converter Oo.
      Written by Vic Ferry aka Krabob/Mkd - (c)2022

 This relase is under APACHE LICENSE , Version 2.0 https://www.apache.org/licenses/LICENSE-2.0 */

#ifndef SC3KBASIC_H
#define SC3KBASIC_H

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

/** \class SC3KBasic
    Manage basic code, convertion to bytes,
    and some utilities with basic like a special mode with labels instead of line index.
*/

class SC3KBasic
{
public:
    SC3KBasic();
    // return >0 if ok
    int readWave( std::istream &inputStream);
    //! will work in basic mode with writeWave()
    int readBasic(std::istream &inputStream);
    //! -untested beta- will work in asm mode with writeWave()
    int readAsmBin(std::istream &inputStream);

    int writeBasic(std::ostream &outputStream);
    int writeWave(std::ostream &outputStream,std::string progName,int bitsPerSample=8, int freq=22050);

    //! optional after writeBasic
    inline bool hasPostBinary() const { return m_postBinaryLength>0; }
    inline int postBinaryLength() const {return m_postBinaryLength; }
    //! will do if hasPostBinary()
    int writePostBinary(std::ostream &outputStream);

    //! because of incbin feature
    inline void setSourceBasePath(std::string basepath) {
        m_sourceBasePath = basepath;
    }

    /** optional use before writeBasic */
    void lineIndexToLabels();

    /** use japan basic ascii version or european ascii version */
    inline void setIsEuroAscii(bool isEuroAscii) {
        m_isEuroAscii = isEuroAscii;
    }
protected:
    // from first chunk header:
    unsigned char _keyCode; //0x16=basic 0x26=assembler.
    std::string    _FileName; // 16 max
    unsigned short _ProgramLength; // on tape for basic, on memory for ASM.
    unsigned short _ProgramStart; // ASM only, where program is copy-loaded and started.

    std::stringstream m_basicStream;
    std::stringstream m_postBinaryStream; // optional extra binary after basic

    std::vector< std::vector< unsigned char > > m_bytes;

    bool        m_isEuroAscii;
    int         m_postBinaryLength;
    std::string m_sourceBasePath;

    void basicStreamToBytes( std::vector< std::vector<unsigned char> > &bytes);
    void tapeWaveFromBytes(std::ostream &ofs,int bitsPerSamples=8,int wavefreq=22050);
    void tapeWaveFromBytes(
            std::vector<unsigned char> &wave,
            const std::vector<unsigned char> &bytes, unsigned int wavefreq);

};

#endif // SC3KBASIC_H
