#ifndef SC3KSoundAndBytes_H_
#define SC3KSoundAndBytes_H_

#include <vector>

/** \class SC3KSoundAndBytes
*
*/
class SC3KSoundAndBytes
{
public:
    SC3KSoundAndBytes();

    /** convert external sound vector to bytes */
    void soundToBytes(const std::vector<signed short> &soundVector, unsigned int freq);

    /** convert bytes to external sound vector */
    // void bytesToSound(std::vector<signed short> &soundVector);

    /** get bytes */
    inline const std::vector< std::vector<unsigned char> > &vbytes() { return m_vbytes; }

protected:
    /** raw 0,1 bits suite with the extra 3 start/stop bits per byte, all chunks. */
    std::vector<char> m_streambits;
    /** decoded bytes chunks, may contain "op-coded" basic or something else. */
    std::vector< std::vector<unsigned char> > m_vbytes;

    void soundToBits(const std::vector<signed short> &soundVector, unsigned int freq);
    void bitsToBytes();

};

#endif // SC3KSoundAndBytes_H_
