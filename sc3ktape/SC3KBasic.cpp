/*            ________  __     __
  ______ ____ \_____  \|  | __/  |______  ______   ____
 /  ____/ ___\  _(__  <|  |/ \   __\__  \ \____ \_/ __ \
 \___ \\  \___ /       |    < |  |  / __ \|  |_> \  ___/
/____  >\___  /______  |__|_ \|__| (____  |   __/ \___  >
     \/     \/       \/     \/          \/|__|        \/
 .oO Sega SC3000/SK1100 Tape-2-Basic-2-Tape Converter Oo.
      Written by Vic Ferry aka Krabob/Mkd - (c)2022

 This relase is under APACHE LICENSE , Version 2.0 https://www.apache.org/licenses/LICENSE-2.0 */

#include "SC3KBasic.h"
#include "SoundReader.h"
#include "SC3KSoundAndBytes.h"

#include "SegaAsciiJpToUnicode.h"
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <map>
#include <sstream>
#include <iomanip>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <cctype>
#include <string>
#include <fstream>
#include <algorithm>

#include "log.h"

using namespace std;

#ifndef uint32_t
typedef unsigned int uint32_t; 
#endif
// displays 12kb free at boot... probably consider if opcodes wouldnt compress that.
#define BASIC_LEVEL_III_A_MAX_BINARY_SIZE (8192+2048-1024)
// displays 26kb free at boot... which is crazy
#define BASIC_LEVEL_III_B_MAX_BINARY_SIZE (8192+16384-1024)

#define KEYCODE_BASICHEADER 0x16
#define KEYCODE_BASICPROGRAM 0x17

#define KEYCODE_ASMHEADER 0x26
#define KEYCODE_ASMPROGRAM 0x27

struct upair { unsigned short _opcode; std::string _word; };
upair BasicKeywords[]=
//map<unsigned short,std::string> BasicKeywords=
{
    // note: 0x80 means: id on 2 bytes.
    {0x82,"list"},
    {0x83,"llist"}, // on printer
    {0x84,"auto"},
    {0x85,"delete"},
    {0x86,"run"},
    {0x87,"cont"},
    {0x88,"load"},
    {0x89,"save"},
    {0x8a,"verify"},
    {0x8b,"new"},
    {0x8c,"renum"},

    {0x90,"rem"},
        {0x91,"print"},
            {0x91,"?"},
        {0x92,"lprint"},
            {0x92,"l?"},
    {0x93,"data"},
    {0x94,"def"},
    {0x95,"input"},
    {0x96,"read"},
    {0x97,"stop"},
    {0x98,"end"},

    {0x99,"let"},
    {0x9a,"dim"},
    {0x9b,"for"},
    {0x9c,"next"},
    {0x9d,"goto"},
    {0x9e,"gosub"},
    {0x9f,"go"},

    {0xa0,"on"},
    {0xa1,"return"},
    {0xa2,"erase"},
    {0xa3,"cursor"},
    {0xa4,"if"},
    {0xa5,"restore"},
    {0xa6,"screen"},
    {0xa7,"color"},
    {0xa8,"line"},
    {0xa9,"sound"},
    {0xaa,"beep"},
    {0xab,"console"},
    {0xac,"cls"},
    {0xad,"out"}, // test output device.
    {0xae,"call"},
    {0xaf,"poke"},

    {0xb0,"pset"},
    {0xb1,"preset"},
    {0xb2,"paint"},
    {0xb3,"bline"},
    {0xb4,"position"},
    {0xb5,"hcopy"},
    {0xb6,"sprite"},
    {0xb7,"pattern"}, // call written as patternc are "pattern character"
    {0xb8,"circle"},
    {0xb9,"bcircle"},
    {0xba,"mag"},
    {0xbb,"vpoke"},
    {0xbc,"motor"},

    {0xe0,"fn"}, // function
    {0xe1,"to"},
    {0xe2,"step"},
    {0xe3,"then"},
    {0xe4,"tab"},
    {0xe5,"spc"},
            {0xc1,"*"},
            {0xc2,"/"},
            {0xc2,"mod"},
            {0xc4,"+"},
            {0xc5,"-"},
                {0xc6,"<>"},{0xc6,"><"},
                {0xc7,">="},{0xc7,"=>"},
                {0xc8,"<="},{0xc8,"=<"},
            {0xc9,">"},
            {0xca,"<"},
            {0xcb,"="},
    {0xcc,"not"},
    {0xcd,"and"},
    {0xce,"or"},
    {0xcf,"xor"},

    {0x8080,"abs"},
    {0x8081,"rnd"},
    {0x8082,"sin"},
    {0x8083,"cos"},
    {0x8084,"tan"},
    {0x8085,"asn"},
    {0x8086,"acs"},  // acos !
    {0x8087,"atn"},
    {0x8088,"log"},
    {0x8089,"lgt"},
    {0x808a,"ltw"},
    {0x808b,"exp"},
    {0x808c,"rad"},
    {0x808d,"deg"},
    {0x808e,"pi"},
    {0x808f,"sqr"},
    {0x8090,"int"},
    {0x8091,"sgn"},
    {0x8092,"asc"},

    {0x8093,"len"},
    {0x8094,"val"},
    {0x8095,"peek"},
    {0x8096,"inp"},
    {0x8097,"fre"},
    {0x8098,"vpeek"},
    {0x8099,"stick"},
    {0x809a,"strig"},
    {0x80a0,"chr$"},
    {0x80a1,"hex$"},
    {0x80a2,"inkey$"},
    {0x80a3,"left$"},
    {0x80a4,"right$"},
    {0x80a5,"mid$"},
    {0x80a6,"str$"},
    {0x80a7,"time$"},

};

// {0x9d,"goto"},
// {0x9e,"gosub"},
#define GOTO_ID  0x9d
#define GOSUB_ID 0x9e
#define RESTORE_ID 0xa5
#define REM_ID      0x90

typedef enum {
    epm_Code=0,
    epm_Quote,
    epm_Rem
} eParseMode;

size_t findNoCase(const std::string &str, const std::string &s, size_t start=0 )
{
    string stra=str;
    std::transform(stra.begin(), stra.end(), stra.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return stra.find(s,start);
}
int strReplaceNoCase(std::string &s,const std::string &bef,const std::string &aft)
{
    size_t i=0;
    size_t in = findNoCase(s,bef,i);
    if(in == string::npos) return 0;
    string ns;
    int nbdone=0;
    while(in != string::npos)
    {
        ns += s.substr(i,in-i); // same chain between
        ns +=aft;
        nbdone++;
        i = in + bef.length();
        in = findNoCase(s,bef,i);
    }
    ns += s.substr(i,in-i); // till end

    s = ns;
    return nbdone;
}
// stra strab must be same size.
void replaceInBin( vector<unsigned char> &bin,const std::string &stra,const std::string &strb)
{
    size_t i=0;
    while(i<bin.size())
    {
        if(stra[0]!=(char)bin[i])
        {
            i++;
            continue;
        }
        bool isFound=true;
        for(size_t j=0; j<stra.length() ; j++ )
        {
            if(stra[j] !=(char)bin[i+j])
            {
                isFound=false;
                break;
            }
        }
        if(!isFound)
        {
            i++;
            continue;
        }
        // -- found one !
        for(size_t j=0; j<stra.length() ; j++ )
        {
            bin[i+j] = strb[j];
        }
        i+= stra.length();
    }
}
void strTrim(std::string &s) {
    size_t i = s.find_last_not_of(" \n\r\t");
    if( i != string::npos ) s = s.substr(0,i+1);
    i = s.find_first_not_of(" \n\r\t");
    s = s.substr(i);
}
unsigned short findKeyCode(const std::string &v, size_t il, string &strfound)
{
    strfound="";
    // bruteforce because of some C99 port
   // vector<upair>::iterator it = BasicKeywords.begin();
   // while(it != BasicKeywords.end())
    for(int i=0; i< sizeof(BasicKeywords)/sizeof(upair) ; i++ )
    {

        const upair &p= BasicKeywords[i];
        if(v.length()-il< p._word.length()) continue;
        size_t j;
        for(j=0;j<p._word.length();j++)
        {
            char c = v[il+j];
            if(c>='A' && c<='Z') c+= ('a'-'A');
            if(c!=p._word[j]) {
                break;
            }
        }
        if(j==p._word.length())
        {
            strfound = p._word;
            return p._opcode;
        }

    }
    return 0;
}
std::string findCodeKey(unsigned short c)
{
    // bruteforce
//    vector<upair>::iterator it = BasicKeywords.begin();
//    while(it != BasicKeywords.end())
    for(size_t i=0; i< sizeof(BasicKeywords)/sizeof(upair) ; i++ )
    {
    
        const upair &p= BasicKeywords[i];
//        const upair &p=*it++;
        if(c==p._opcode) return p._word;
    }
    return "";
}

std::string Utf8ToSegascii(std::string ustr)
{
    std::string sc;

    static map<unsigned int, unsigned char> uniToScMap;
    if(uniToScMap.size() ==0)
    {
        for(int i=0;i<256;i++)
        {
            uniToScMap[tSegaAsciiJpToUnicode[i]]=(unsigned char)i;
        }
        // euro symbols, can use same table in that case:
        for(int i=0x0a0;i<0x0d0;i++) // symbols that differs from jp
        {
            uniToScMap[tSegaAsciiEuToUnicode[i]]=(unsigned char)i;
        }
    }

    // first loop check chars encoding
    size_t l = ustr.length();
    size_t i = 0;
    bool bIsUTF8 = true;
    while (i < l)
    {
        // that's how you read UTF-8 chars for real to extract unicodes:
        unsigned int cc = (unsigned char)ustr[i]; // on one char, 7 bits
        i++;
//        if ((cc & 0b11000000) == 0b10000000)
        if ((cc & 0xc0) == 0x80)
        {	// means not unicode, already ok ?
            sc += (char) cc;
            bIsUTF8 = false;
            continue;
        }
        if (cc > 127 && bIsUTF8)
        {
            if (i >= l) break;
            unsigned int c2 = (unsigned char)ustr[i];
//            if ((cc & 0b11100000) == 0b11000000)
            if ((cc & 0xe0) == 0xc0)

            {
                // 2 chars
//                cc = ((cc & 0b00011111) << 6 | (c2 & 0b00111111));
                cc = ((cc & 0x1f) << 6 | (c2 & 0x3f));
                
            }
//            else if ((cc & 0b11110000) == 0b11100000)
            else if ((cc & 0xf0) == 0xe0)

            {
                unsigned int c3 = (unsigned char)ustr[i+1];
                // 3 chars
//                cc = ((cc & 0b00001111) << 12 | (c2 & 0b00111111)<<6 | (c3 & 0b00111111));
                cc = ((cc & 0x0f) << 12 | (c2 & 0x3f)<<6 | (c3 & 0x3f));

            }
//            else if ((cc & 0b11111000) == 0b11110000)
            else if ((cc & 0xf8) == 0xf0)            
            {
                unsigned int c3 = (unsigned char)ustr[i + 1];
                unsigned int c4 = (unsigned char)ustr[i + 2];
                // 4 chars
//                cc = ((cc & 0b00000111) << 18 | (c2 & 0b00111111) << 12 | (c3 & 0b00111111)<<6 | (c4 & 0b00111111) );
                cc = ((cc & 0x07) << 18 | (c2 & 0x3f) << 12 | (c3 & 0x3f)<<6 | (c4 & 0x3f) );

            }
            // then...
            if(uniToScMap.find(cc) != uniToScMap.end())
            {
                unsigned char csc = uniToScMap[cc];
                sc +=csc;
            } else
            {   // if unicode is not supported by segascii, set a space.
                 sc+= ' ';
            }

        } else
        {

            sc += (char) cc;
        }
    }

    return sc;
}


SC3KBasic::SC3KBasic()
    : _keyCode(0)
    , _ProgramLength(0)
    , _ProgramStart(0)
    , m_isEuroAscii(false)
    , m_postBinaryLength(0)
{

}




// do not parse code, only inner string, comment or program name.
std::string SegasciiToUrl(std::string str)
{
    //TODO
    std::string surl;


    return surl;
}
// do not parse code, only inner string, comment or program name.
std::string SegasciiToUtf8(std::string strsg, bool isEuroAscii)
{
    std::string sutf8;

    size_t i=0;
    while(strsg[i] !=0)
    {
        unsigned char c = strsg[i];
        i++;
    //    unsigned int sgc = tSegaAsciiJpToUnicode[c];
        unsigned int sgc = (isEuroAscii)?
                    tSegaAsciiEuToUnicode[c]
                  :
                    tSegaAsciiJpToUnicode[c];
        if(sgc<128){
            sutf8 += (char)sgc;
            continue;
        }
        // to utf8, 2 bytes
        if(sgc< 0x0800){
            sutf8 += (char)(0xc0 |
                            ((sgc>>6)& 0x1f)
                            );
            sutf8 += (char)(0x80 |
                            (sgc& 0x3f)
                            );
            continue;
        }
        // to utf8, 3 bytes
        if(sgc<0x010000){
            sutf8 += (char)(0xe0 |
                            ((sgc>>12)& 0x0f)
                            );
            sutf8 += (char)(0x80 |
                            ((sgc>>6) & 0x3f)
                            );
            sutf8 += (char)(0x80 |
                            (sgc& 0x3f)
                            );
            continue;
        }
        // to utf8, 4 bytes
        if(sgc<0x0110000){
            sutf8 += (char)(0xf0 |
                            ((sgc>>18)& 0x07)
                            );
            sutf8 += (char)(0x80 |
                            ((sgc>>12) & 0x3f)
                            );
            sutf8 += (char)(0x80 |
                            ((sgc>>6) & 0x3f)
                            );
            sutf8 += (char)(0x80 |
                            (sgc& 0x3f)
                            );
            continue;
        }
    }

    return sutf8;
}


/*
class SC3KProg
{
public:

    unsigned char _keyCode; //0x16
   // char    _FileName[16];
    std::string _FileNamestr;
    unsigned short _ProgramLength;
};
*/

int SC3KBasic::readWave(std::istream &inputStream)
{
    stringstream strreport;

    SC3KSoundAndBytes soundToBytes;
    {
        SoundVector soundVector;
        SoundReaderBroker srb;
        srb.read(soundVector,inputStream);

        soundToBytes.soundToBytes(soundVector._soundVector,soundVector._samplePerSeconds);
    }

    const vector< vector<unsigned char> > &vb = soundToBytes.vbytes();

    if(vb.size()<2) {
        throw runtime_error( "less than 2 chunk found, no data" );
    }
    const vector<unsigned char> &programTapeHead = vb[0];

    if(programTapeHead.size()<22) {
        throw runtime_error( "header chunk too short" );
    }
    _keyCode = programTapeHead[0];
    if(programTapeHead[0] != 0x16) {
        throw runtime_error( "header code not 0x16" );
    }

    string sgfileName;
    _FileName = "";
    for(int i=0;i<16;i++)
    {
        char c = (char)programTapeHead[i+1];
       // no, it's space. if(c==0) break;
        if(programTapeHead[i+1]!=0) sgfileName+= c;
    }
    // remove end spaces
//    while(sgfileName.length()>0 && sgfileName.back()==' ') sgfileName.pop_back();
    while(sgfileName.length()>0 && sgfileName[sgfileName.length()-1]==' ')
    {
        string s = sgfileName.substr(0,sgfileName.length()-1);
        sgfileName = s;
    }

    _FileName = SegasciiToUtf8(sgfileName,m_isEuroAscii);
    _ProgramLength = ( (unsigned short)programTapeHead[17]<<8) |( (unsigned short)programTapeHead[18] );
    //verify parity ?
    unsigned char parity = 0;
    for(int i=1;i<19;i++)
    {
        parity += (unsigned char)programTapeHead[i];
      //  for(int j=0;j<8;j++) if( (((unsigned char)1<<j)&c) != 0) parity ++;
    }
    parity =(unsigned char)(-(int)parity);
//    if((unsigned char)programTapeHead[19] != (unsigned char)parity)
    {
    cout << "Parity Issue t: " << (int)programTapeHead[19] << " : " << (int)parity << endl;
       // throw runtime_error( "wrong header parity" );
    }
    if(programTapeHead[20] != 0 || programTapeHead[21] != 0)
    {
        throw runtime_error( "Dummy zero overriden at end of header chunk." );
    }

    cout << "ProgramName: " << _FileName << endl;
    cout << "ProgramByteSize: " << dec << _ProgramLength << endl;

    // - - - program chunk:
    const vector<unsigned char> &programBin = vb[1];
    int attemptedsize = (1+_ProgramLength+3);
    if(programBin.size()< (size_t) attemptedsize)
    {
        stringstream ss;
        ss <<"program chunk 2 should have at least: " << attemptedsize << "b , found: " << (int) programBin.size() ;
        throw runtime_error(ss.str());
    }
    if(programBin[0] != KEYCODE_BASICPROGRAM) {
        throw runtime_error( "program chunk header code not 0x17" );
    }

    // - - - decode lines...
//    map<unsigned short,string> codeToStr;
//    for(const pair<unsigned short,string> &p : BasicKeywords) codeToStr[]
//BasicKeywords
//    m_basicStream.str(); // flush

    unsigned char lineCodeLength=0; // =programBin[1]; // number indicating the current position within the program field

    int linestate=0; //0: first parity value ? 1: line number
    unsigned int programLine=0;
    //bool isInQuoteMode=false;
    eParseMode parseMode = epm_Code;

    for(int i=1;i<_ProgramLength+1 ;i++)
    {
        if(linestate==0)
        {
            lineCodeLength = (unsigned int)programBin[i] ;
            if(lineCodeLength==0)
            {
              //  cout << "line code length 0 means end" << endl;
                if(i<_ProgramLength)
                {
                    // put post binary in separate file
                    m_postBinaryStream.clear();
                    m_postBinaryStream.str(string());
                    // stringstream used as binary fifo:
                    int postbinlength = (_ProgramLength-i);
                    m_postBinaryStream.write((char*)&programBin[i+1],postbinlength);
                    m_postBinaryLength = postbinlength;
                }
                // means end of chunk !!!
                break;
            }
            //           strversion << ":" << dec << (int)lineCodeLength << " \n" ;
            linestate = 1;
            continue;
        }

        if(linestate==1)
        {
            programLine = (unsigned int)programBin[i] |
                        ((unsigned int)programBin[i+1]<<8 ) |
                        ((unsigned int)programBin[i+2]<<16 ) |
                        ((unsigned int)programBin[i+3]<<24 );
            i+=3;
           m_basicStream << dec << programLine << " " ;
            linestate = 2;
            continue;
        }

        unsigned char c =programBin[i];
        if(c==0) continue;
        if(c<0x80)
        {
            if(c==13)
            {
                m_basicStream << '\n';
                linestate = 0;
                parseMode = epm_Code; // I think...
            }
            else if(c=='"')
            {
                m_basicStream << (char)c;
                if(parseMode == epm_Quote)
                {
                    parseMode = epm_Code;
                } else
                {
                    if(parseMode == epm_Code) parseMode = epm_Quote;
                }
            } else if(c=='%')
            {
                //if(parseMode != epm_Code) m_basicStream << "%%"; // to safely use %hex translation on PC side.
                //else m_basicStream << (char)c;
                 m_basicStream << (char)c;
            }
            else //if(c==32)
            {
                m_basicStream << (char)c;
            }
        } else
        {
            if(parseMode != epm_Code)
            {   // quotes and remarks: only litterals, no opcodes...
                // in quotes: upper ascii is more likely "SegaSCII" or japan chars...
             //   m_basicStream << "%" << setfill('0') << setw(2) << right << hex << (int)c;
             //   m_basicStream << dec ;
                // utf8 way:
                string strc;
                strc += ((char)c);
                string strutf8 = SegasciiToUtf8(strc,m_isEuroAscii);
                m_basicStream << strutf8;

            } else
            {
                if(c==0x80 && i<_ProgramLength+1-1)
                {
                    //code on 2 bytes
                    i++;
                    unsigned short cc =((unsigned short)c)<<8  | (unsigned short)programBin[i];
                    string strf = findCodeKey(cc);
                    if(strf.length()>0)
                    {
                        m_basicStream << strf;
                    }
                } else
                {
                    string strf = findCodeKey(c);
                    if(strf.length()>0)
                    {
                        m_basicStream << strf;
                        if(c==REM_ID) parseMode= epm_Rem;
                    }
                }
            }
        }

    } // loop by i
    cout << "code:\n\n" << m_basicStream.str() << endl;




    // firts chunk:
    //  key code 1byte = 0x16
    //  16b filename filled with 0 at end
    // 2b Program length = n (in chunk 2)
    // 1b Parity: 2complement sum of data (filename+programlength)
    // 2b Dummy data (0)?

    // one sec not recorded (silence)
    // busy 1

    // chunk 2:
    // 1byte keycode 17H
    //(program of n bytes)


    //

    return 0;
}

void replaceLabelsByLine(std::string &s, const map<string,uint32_t> &labelToLine)
{
    string sLower=s;
    string sresult;

    std::transform(sLower.begin(), sLower.end(), sLower.begin(),
        [](unsigned char c){ return std::tolower(c); });

    const string opcodes[]={"gosub","goto","restore"};
    size_t il=0;
    size_t prevIl=0;
    while(il<s.length())
    {
        size_t iFirstFound=1024;
        string opcodeFound;
//        for(const string &opcode : opcodes )
        for(int i=0 ; i<3 ; i++)
        {
            const string &opcode = opcodes[i];
            size_t is=sLower.find(opcode,il);
            if(is != string::npos && is<iFirstFound)
            {
                opcodeFound = opcode;
                iFirstFound = is;
            }
        }
        if(opcodeFound.length()>0)
        {
            // treat this one:
            sresult += s.substr(il,iFirstFound+opcodeFound.length()-il);
            il = iFirstFound+opcodeFound.length();
            while(il<s.length())
            {
                // there may be labels here...
                while(il<s.length() && (s[il]==' ' || s[il]=='\t') ) {
                    sresult +=' ';
                    il++;
                }
                // search next " \n:,
                size_t iln = s.find_first_of(" \n:,",il);
                string slabel = s.substr(il, iln-il);
                if(labelToLine.find(slabel) != labelToLine.end())
                {
                    stringstream sn;
                    sn << (*labelToLine.find(slabel)).second;
                    sresult += sn.str();
                    il = iln;
                }
            }

        } else
        {
           // prevIl = il;
            sresult += s.substr(il,s.length()-il);
            il = s.length(); // aka exit
        }
    }
    s=  sresult;
}

int SC3KBasic::readBasic(std::istream &ifs)
{
    m_basicStream.clear();
    m_basicStream.str(string());

    ifs.seekg(0,ios::beg);
    // first 3 bytes may be UTF-8 BOM
    {
        char bom[3]={0,0,0};
        ifs.read(bom,3);
        if(bom[0]!=0xEF || bom[1]!=0xBB || bom[2]!=0xBF)
        {  // if not bom restart, if bom continue at 3.
            ifs.seekg(0,ios::beg);
        }
        // if UTF16, do not manage at the moment
        if((bom[0]==0xFF && bom[1]==0xFE) ||
                (bom[0]==0xFE && bom[1]==0xFF)
                )
        {
            throw runtime_error("basic file is UTF16 encoded, please use UTF8.");
        }
    }

    // get lines, if source in label mode, renum.
    uint32_t renumStep = 10;
    uint32_t iline=renumStep;
    bool renumMode = false;
    map<string,uint32_t> labelToLine;

    string line;
    getline(ifs,line);
    while(true)
    {
        if(line.length()==0 || line[0]=='#' )
        {
            getline(ifs,line);
            continue;
        }
        if((line[0]< '0' || line[0]> '9') && !renumMode )
        {
            cout << "no number found for line, use renum mode." << endl;
            renumMode = true;
        }
        line.erase(line.find_last_not_of(" \n\r")+1);
        if(renumMode)
        {
            if(line[0] == '\t' || line[0] == ' ')
            {   // code line
                string codeline = line; //.substr(1);
                strTrim(codeline);
                if(codeline.length()>0)
                {
                    // replace label later !
                    m_basicStream << iline << " " << codeline  << "\n";
                    iline += renumStep;
                }
            } else
            {
                // label
                size_t itf = line.find(":");
                string slabel = line.substr(0,itf);
                labelToLine[slabel] = iline;
            }


        } else
        {
            // source already using line numbers
            m_basicStream << line << "\n";
        }
        if(ifs.peek() == EOF) break;
        getline(ifs,line);
    }
    // need another pass to replace labels if needed.
    if(labelToLine.size()>0 && renumMode)
    {
        stringstream sn;
        getline(m_basicStream,line);
        while(line.length()>0)
        {
            replaceLabelsByLine(line,labelToLine);
            sn << line << "\n";
            getline(m_basicStream,line);
        }
        //m_basicStream.swap(sn);
        m_basicStream.clear();
        m_basicStream.str(string());
        m_basicStream << sn.str();
    }

    basicStreamToBytes(m_bytes);

    return 0;
}

//! will work in asm mode with writeWave()
int SC3KBasic::readAsmBin(std::istream &inputStream)
{

    /*
    YET TO BE VALIDATED ON REAL HARDWARE
*/

    m_basicStream.clear();
    m_basicStream.str(string());
    int parity=0;

    // start with chunk2 to know size
    // in asm bin case, just do a bare copy
    inputStream.seekg(0,istream::end);
    size_t binsize = (size_t)inputStream.tellg();
    inputStream.seekg(0,istream::beg);
    if(binsize>= BASIC_LEVEL_III_A_MAX_BINARY_SIZE)
    {
        // warn because wouldn't work on very common Basic IIIA
        LOGW() << "Warning: Binary file obviously too big for a 12kb sega BASIC IIIA SC/SG wave\n";
    }

    if(binsize>= BASIC_LEVEL_III_B_MAX_BINARY_SIZE)
    {
        // error because wouldnt work nowhere
        _ProgramLength=0;
        throw runtime_error("Binary file obviously too big even for a 24kb sega BASIC IIIB SC/SG wave");
    }
    _ProgramLength = (unsigned short)binsize;
    _ProgramStart = 0xA000; // would be happy to know a correct value. (discussion...)
    // with 0xA000, on BasicIIIA ,end is 0xBFFF max length would be 8k
/*

 1800:pattern generator ?

    $0000-$7FFF : ROM
    $8000-$BFFF : DRAM (first 16K) -> in the cartridge. 4x 4ko

     page 14 of doc:
iAfterZero
    $8000->$97FF basic reserved ram
       $8160 (2b) pointer to start of basic program
        ... which should be : $9800

        $9808 -> start adress into a rem first line

    $C000-$FFFF : Work RAM (mirrored repeatedly every 2K) -> the 2K ram from motherboard !
    so rather say:
    $C000-$C800
    - or -
    $C000-$FFFF : DRAM (latter 16K if work RAM disabled and missing DRAM chips added)

    + VMEM is other 16k 0->7fff !


    http://steveproxna.blogspot.com/2013/08/sega-assembly-programming.html
    start at F000
    10 CALL &HF000
    -----
    $9808 -> start adress into a rem first line
*/
    vector<unsigned char> programBin(1+binsize+1+2,0);
    programBin[0] = KEYCODE_ASMPROGRAM;
    inputStream.read((char *)&programBin[1],binsize);
    // parity
    parity=0;
    for(size_t i=1; i<binsize+1 ; i++) // 1 because jump keycode
    {
        parity += (unsigned char) programBin[i];
    }
    programBin[1+binsize]=(unsigned char)(-parity);
    //+2 ending zero already set at alloc.

    // = = = = = = = == = = = = = = = = = = = == = ==
    // - - now we can create the first chunk program header:
    vector<unsigned char> programHeader(1,KEYCODE_ASMHEADER ); // ={0x26};
    {
        string sgname = Utf8ToSegascii(_FileName);
        int i;
        for(i=0;i<16;i++)
        {
            if(sgname[i]==0) break;
            programHeader.push_back((unsigned char)sgname[i]);
        }
        // padded with spaces...
        for(;i<16;i++) programHeader.push_back((unsigned char)' ');
    }
    // _ProgramLength
    programHeader.push_back((unsigned char)(_ProgramLength>>8));
    programHeader.push_back((unsigned char)_ProgramLength);
    // _ProgramStart start of program in memory
    programHeader.push_back((unsigned char)(_ProgramStart>>8));
    programHeader.push_back((unsigned char)_ProgramStart);

    parity=0;
    for(size_t i=1; i<programHeader.size() ; i++) // 1 because jump keycode
    {
        parity += (unsigned char) programHeader[i];
    }
    programHeader.push_back((unsigned char)(-parity));
    programHeader.push_back(0); // want 2 zeros at the end.
    programHeader.push_back(0);

    m_bytes.push_back(programHeader);
    m_bytes.push_back(programBin);


}

int SC3KBasic::writeWave(
    std::ostream &outputStream,
    std::string progName,
    int bitsPerSample,
    int freq)
{
	if(progName.length()==0) {
    	progName="BASIC";
    } else
	if(progName.length()>16) {
    	progName = progName.substr(0,16);
    }

    // upper case for prog name
	{ 	string s;
        for(size_t i=0;i<progName.length();i++)
        {
			char c = progName[i];
            s+= (c>='a' && c<='z')? c+('A'-'a'):c ;
        }
        progName  =s;
    }

    tapeWaveFromBytes(outputStream,bitsPerSample,freq);
    return 0;
}

int SC3KBasic::writeBasic(std::ostream &outputStream)
{
    outputStream << m_basicStream.str();
    return 0;
}
//! will do if hasPostBinary()
int SC3KBasic::writePostBinary(std::ostream &outputStream)
{
    if(m_postBinaryLength==0) throw runtime_error("no post binary to save.");
    outputStream << m_postBinaryStream.rdbuf() ;
    return 0;
}

//function<string(map<uint32_t,std::string> &,int &,uint32_t , unsigned short )>

static std::string addOrGetLabelForLine( map<uint32_t,std::string> &labelsMap,
                           int &labelidGrower,
                           uint32_t iline, unsigned short codecause )
{
    if(labelsMap.find(iline) != labelsMap.end()) return labelsMap[iline];
    string labelnamebase;
    if(codecause == GOSUB_ID) labelnamebase = "func";
    else if(codecause == GOTO_ID) labelnamebase = "label";
    else if(codecause == RESTORE_ID)  labelnamebase = "ldata";

    stringstream labelname;
    labelname<< labelnamebase << dec << labelidGrower ;
    string slabel  =labelname.str();
    labelsMap[iline] = slabel;
    labelidGrower++;
    return slabel;
}


// before writeBasic
void SC3KBasic::lineIndexToLabels()
{
    // std::stringstream m_basicStream;
    map<uint32_t,std::string> lineMap; // string is normal code line.
    map<uint32_t,std::string> labelsMap; // string is label for this line.
    int labelidGrower=1;


    // save state, restart
    string startstate = m_basicStream.str();
    m_basicStream.clear();
    m_basicStream.str(string());
    m_basicStream << startstate;

    // seek for GOTO, GOSUB , RESTORE
    // there is:
    // ON VAR GOTO 100,200,300
    // ON (var) GOSUB 100,200,300

    string sline;
    getline(m_basicStream,sline);
    while(sline.length()>0)
    {
        int nline=0;
        size_t il= 0;

        if(sline[0]<'0' || sline[0]>'9')
        {   // alreay done, exit with start state...

            m_basicStream.clear();
            m_basicStream.str(string());
            m_basicStream << startstate;
            return; //
        }
        nline = (int) (sline[il]-'0');
        il++;
        while( il<sline.length() && sline[il]>='0' && sline[il]<='9' )
        {
            nline *=10;
            nline += (int)(sline[il]-'0');
            il++;
        }
        while( il<sline.length() && sline[il]==' ') il++;
       // string rline = sline.substr(il);
       // lineMap[nline]= rline;
        stringstream snewline;
        // parse the same way as wave decoding/writing

        eParseMode parseMode = epm_Code;

        while(il<sline.length())
        {
   //
            if(parseMode == epm_Code)
            {
                // because we keep words as lower case on this side..
                string strfound;
                unsigned short code = findKeyCode(sline,il,strfound);
                if(code!=0)
                {
                    il+= strfound.length();
                    snewline << strfound;
                    // cases where line index is used:
                    switch(code)
                    {
                        case GOTO_ID:
                        case GOSUB_ID:
                        case RESTORE_ID:
                        {
                            uint32_t iline=0;
                            while(il<sline.length())
                            {
                                if(sline[il]==':')
                                {
                                    snewline << ':';
                                    il++;
                                    break;
                                }
                                if(sline[il]==' ') {
                                    snewline << ' ';
                                    il++;
                                    continue;
                                }
                                if(sline[il]==',') {
                                    string slabel = addOrGetLabelForLine(labelsMap,labelidGrower,iline,code);
                                    snewline << slabel << ',';
                                    // restart iline
                                    iline = 0;
                                    il++;
                                    continue;
                                }
                                if(sline[il]>='0' && sline[il]<='9' )
                                {
                                    iline *=10;
                                    iline += (int)(sline[il]-'0');
                                    il++;
                                    continue;
                                }
                                il++;


                            } // end while parse x, x ,x
                            if(iline>0)
                            {
                                /*
                                addOrGetLabelForLine=[](map<uint32_t,std::string> &labelsMap,
                                                        int &labelidGrower,
                                                    uint32_t iline, unsigned short codecause )
                                */
                                string slabel = addOrGetLabelForLine(labelsMap,labelidGrower,iline,code);
                                snewline << slabel;
                            }

                        }
                        break;
                    case REM_ID:
                        parseMode = epm_Rem;

                        break;
                    default:
                        break;
                    }

                } else
                {
                    // in code, but regular ascii:
                    char c =sline[il]; il++;
                    snewline << c;
                    if(c=='"') parseMode = epm_Quote;
                }
            } else{
                //
                char c =sline[il]; il++;
                snewline << c;
                if(c=='"' && parseMode == epm_Quote)  parseMode=epm_Code;
            }
        }
        lineMap[nline] = snewline.str();

        // next line
        getline(m_basicStream,sline);
    }

    // recreate text:
//    map<uint32_t,std::string> lineMap; // string is normal code line.
//    map<uint32_t,std::string> labelsMap; // string is label for this line.

    stringstream ns;
//    string r = m_basicStream.str();
//    m_basicStream.clear();

   map<uint32_t,string>::iterator lit = lineMap.begin();
   while(lit != lineMap.end())
   {
        pair<const uint32_t,std::string> &p = *lit++;
        // if line has label or not:
        map<uint32_t,string>::iterator fit = labelsMap.find(p.first);
        if(fit != labelsMap.end())
        {
            // insert label
            ns << fit->second << ":\n";
        }
        //  just tab the line without line number
        ns << '\t' << p.second << '\n';
   }
   //was:
   //m_basicStream.swap(ns);
    m_basicStream.clear(); // clear bits
    m_basicStream.str(string());
    m_basicStream << ns.str();

}

void SC3KBasic::tapeWaveFromBytes(std::ostream &ofs,int bitsPerSamples,int wavefreq)
{
    vector<unsigned char> waves;

    for(size_t i=0 ; i<m_bytes.size() ;i++)
    {
     	const vector<unsigned char> &chunk = m_bytes[i];

        vector<unsigned char> wave;
        tapeWaveFromBytes(wave,chunk,wavefreq);
        // append
        waves.insert(waves.end(),wave.begin(),wave.end());
    }

 //   ofstream ofs("path");
    SoundWriter_Wave sw;
    sw.write(ofs,waves,wavefreq);

}

void SC3KBasic::tapeWaveFromBytes(
        std::vector<unsigned char> &wave,
        const std::vector<unsigned char> &bytes, unsigned int wavefreq)
{

    class BitFeed
    {
    public:
        // 3 sec. of ones...
        BitFeed(const std::vector<unsigned char> &bytes, int floodOnesAtStart=1200*3) :
            _floodOnesAtStart(floodOnesAtStart)
            ,_bytes(bytes)
            ,_currentbyteIndex(0)
            ,_fifo(8+3) // bytes are written as 0 XXXXXXXX 11
            ,_currentfifobitRead(0)
            ,_currentfifobitWrite(0)
        {

        }
        // - - - -
        // 0 or 1, 2 means no more bits.
        //bool hasBit();
        char get()
        {
            if(_floodOnesAtStart>0) {
                _floodOnesAtStart--;
                return 1;
            }
            if(_currentfifobitRead<_currentfifobitWrite)
            {
                char c = _fifo[_currentfifobitRead];
                _currentfifobitRead++;
                return c;
            }
            // has to feed...
            if(_currentbyteIndex<_bytes.size())
            {
                // feed:
                unsigned char c = _bytes[_currentbyteIndex];
                _currentbyteIndex++;

                _currentfifobitRead=0;
                _currentfifobitWrite=8+3;
                _fifo[0]=0;
                for(int j=0;j<8;j++)
                {
                    _fifo[1+j] = (c>>j)&1;
                }
                _fifo[9]=1;
                _fifo[10]=1;

                return get();
            }
            return 2; // mean end.
        }
        unsigned int nbSamples(unsigned int freq)
        {
            // in 1200hz
            unsigned long long nbBits = _floodOnesAtStart + (unsigned int)_bytes.size()*11;
            unsigned long long samplesForFreq= ((nbBits*freq) / 1200);
            return (unsigned int)samplesForFreq;
        }

        // - - - (note: then one sec silence).

    protected:
        int _floodOnesAtStart;
        const std::vector<unsigned char> &_bytes;
        size_t  _currentbyteIndex;
        // - -  -
        vector<char> _fifo;
        size_t     _currentfifobitRead;
        size_t     _currentfifobitWrite;
    };
    BitFeed bitFeed(bytes);
    // wavefreq 22050hz or 44100hz

    // 1: 2400hz  _-_-
    // 0: 1200hz  __--

    // magstep=
    // looks to easily fit ll cases:
    int magfreq = (wavefreq<<8);
    int magstep =  magfreq / (2400*2); // (1176 for 22050) // frq/4

    int waveFreqAccum=0;

    int freqLowPart=0; // goes 0123 for __-- _-_- schemes.
    int currentBitValue=0;

    unsigned int nbSamples = bitFeed.nbSamples(wavefreq);
    wave.resize(nbSamples+3*wavefreq,128); // +3 sec of silence.
    unsigned char lowval=128-10;
    unsigned char highval=128+10;
    unsigned char cursignalvalue=128;
    for(unsigned int i=0;i<nbSamples ; i++)
    {
        waveFreqAccum += 256;
        if(waveFreqAccum>=magstep)
        {
           waveFreqAccum-=magstep;

            if(freqLowPart==0)
            {
                currentBitValue = bitFeed.get();
                if(currentBitValue==2) break; // end
            }
            // currentBitValue 0:
            if(currentBitValue==0)
            {
                if(freqLowPart==0) cursignalvalue =lowval;
                if(freqLowPart==1) cursignalvalue =lowval;
                if(freqLowPart==2) cursignalvalue =highval;
                if(freqLowPart==3) cursignalvalue =highval;
            } else // 1
            {
                if(freqLowPart==0) cursignalvalue =lowval;
                if(freqLowPart==1) cursignalvalue =highval;
                if(freqLowPart==2) cursignalvalue =lowval;
                if(freqLowPart==3) cursignalvalue =highval;
            }

           freqLowPart++;
           if(freqLowPart==4) freqLowPart=0;
           // switch

        } // end if 2400*2 hz
        wave[i] = cursignalvalue;
    } // loop per audio sample.


}

void SC3KBasic::basicStreamToBytes(std::vector< std::vector<unsigned char> > &bytes)
{
    bytes.clear();
    vector<unsigned char> programBin(1,KEYCODE_BASICPROGRAM); //= {0x17}; // id for chunk2
    int textline=1;
    size_t iIncbinInLine = string::npos;
    int nbIncBinRefs=0;
    string sline;
    string postbinfilename;
    while(m_basicStream.good())
    {
        sline.clear();
        getline(m_basicStream,sline);
        if(sline.size()==0) continue;
        iIncbinInLine = findNoCase(sline,"incbin");
        if(iIncbinInLine != string::npos)
        {
            postbinfilename = sline.substr(iIncbinInLine+6);
            // then means end of program, it's a post binary
            // incbin treated after loop
            break;
        }
        // replace incbin refs by well sized &HXXXX, for later replacement.
        nbIncBinRefs += strReplaceNoCase(sline,"%binref","&HXXXX");

        size_t il= 0;
        int nline=0;
        if(sline[0]<'0' || sline[0]>'9')
        {
            throw runtime_error("line not starting with number");
        }
        nline = (int) (sline[il]-'0');
        il++;
        while( il<sline.length() && sline[il]>='0' && sline[il]<='9' )
        {
            nline *=10;
            nline += (int)(sline[il]-'0');
            il++;
        }

        while( il<sline.length() && sline[il]==' ') il++;
        // real start of line
        vector<unsigned char> linebytes(5);
        // linebytes[0] set at end...
        // 1 bytes nb bytes in line
        linebytes[1]=(nline & 255);
        linebytes[2]=((nline>>8) & 255);
        linebytes[3]=((nline>>16) & 255);
        linebytes[4]=((nline>>24) & 255);

        eParseMode parseMode = epm_Code;

        while(il<sline.length())
        {
   //
            if(parseMode == epm_Code)
            {
                // because we keep words as lower case on this side..
                string strfound;
                unsigned short code = findKeyCode(sline,il,strfound);
                if(code!=0)
                {   
                    if(code<256)
                    {
                        linebytes.push_back((unsigned char)code);
                    } else{
                        linebytes.push_back((unsigned char)(code>>8));
                        linebytes.push_back((unsigned char)(code));
                    }

                    il+= strfound.length();
                    if(code == REM_ID) parseMode = epm_Rem;
                } else
                {
                    // in code, but regular ascii:
                    char c =sline[il]; il++;

                    linebytes.push_back((unsigned char)c);
                    if(c=='"') parseMode = epm_Quote;
                  //never reached  if(c=='\n') parseMode = epm_Code;
                }
            } else{
                //
                char c =sline[il];
                il++;
                if(c=='%') // our enconding on our side.
                {
                    if(il<sline.length()-1 && sline[il+1]=='%')
                    {
                        linebytes.push_back((unsigned char)c);
                        il++;
                    }else if(il<sline.length()-2)
                    {
                        //sscanf ?
                        unsigned int v=20;
#ifdef WIN32
                        sscanf_s(sline.c_str()+il,"%02x",&v);
#else
                        sscanf(sline.c_str()+il,"%02x",&v);
#endif
                        linebytes.push_back((unsigned char)v);
                        il+=2;
                    }
                } else
                {
                    if(c=='"' && parseMode == epm_Quote)  parseMode=epm_Code;
                    linebytes.push_back((unsigned char)c);
                }
            }

        }
        // finaly:
        int bsize = (int)linebytes.size() - 5;
        linebytes[0] = (unsigned char)bsize;
        linebytes.push_back(0x0d); // CR, not LF.

//        cout << "lb:" << endl;
//        for(unsigned char c : linebytes)
//        {
//            cout << "l: "<< hex << (int)c << endl;
//        }
        programBin.insert(programBin.end(),linebytes.begin(),linebytes.end());
        textline++;
    } // end loop per line read.
    //
    programBin.push_back(0); // want a zero to say no more lines.

    if(iIncbinInLine != string::npos )
    {
        // if basic ends with incbin directive, append extra binary after basic
        // get file name
        strTrim(postbinfilename);
        cout << "include post binary file: " << postbinfilename << endl;
        string abspath = m_sourceBasePath + "/" + postbinfilename;
        ifstream pbinifs(abspath.c_str(),ios::binary|ios::in);
        if(!pbinifs.good())
        {
            stringstream ss;
            ss << "Can't read binary file:" << postbinfilename ;
            throw runtime_error(ss.str());
        }
        pbinifs.seekg(0,istream::end);
        size_t binsize = (size_t)pbinifs.tellg();
        pbinifs.seekg(0,istream::beg);
        if(binsize>= BASIC_LEVEL_III_A_MAX_BINARY_SIZE)
        {
            // warn because wouldn't work on very common Basic IIIA
            LOGW() << "Warning: incbin binary file obviously too big for a 12kb sega BASIC IIIA SC/SG wave\n";
        }
        if(binsize >= BASIC_LEVEL_III_B_MAX_BINARY_SIZE )
        {
            runtime_error("incbin binary file obviously too big even for a 26kb sega BASIC IIIB SC/SG wave");
        }
        size_t iAfterZero = programBin.size();
        programBin.resize(iAfterZero+binsize);
        pbinifs.read((char*)&programBin[iAfterZero],binsize);

        // resolve %binref incbin reference for call command to binary:

        // OK value for Basic LevelIIIA/B and SK III
#define BASIC_START 0x9800
        int binaryStartAdress = BASIC_START + (int)iAfterZero -1;

        if(nbIncBinRefs>0)
        {
            stringstream strrefs;
            strrefs << "&H" << setfill('0') << setw(4) << right << hex << binaryStartAdress ;
            string strref = strrefs.str();
            std::transform(strref.begin(), strref.end(), strref.begin(),
                [](unsigned char c){ return std::toupper(c); });
            // value must be known
            LOGI() << " %BINREF in code is replaced by " << strref << " which is the post binary start adress\n";
            LOGI() << " binary should be relocated to this value.\n";
            replaceInBin(programBin,"&HXXXX",strref);
        }
        LOGI() << "appended post binary file of length: " << binsize << " after basic code." << endl;

    }

    _ProgramLength = (int)programBin.size() -1;

    int parity=0;
    for(size_t i=1; i<programBin.size() ; i++)
    {
        parity += (unsigned char) programBin[i];
    }
 // cout << "parity: "<< hex << (int)(unsigned char)(-parity) << endl;
    programBin.push_back((unsigned char)(-parity));
    programBin.push_back(0); // want 2 zeros at the end.
    programBin.push_back(0);
/*
17
-----------
0b   11

05
00
00
00

91 print
20
22 "
41
41
41
41
42
42
42
22 "
0d
-------------
01

0a
00
00
00

aa beep
0d


---------
04   length (apres la ligne et sans le \n

14
00
00
00 line number =20

9d goto
20 ' '
31 '1'
30 '0'
0d \n
-----------

00 ?

1f parity
00 extra zero
00

*/



    // = = = = = = = == = = = = = = = = = = = == = ==
    // - - now we can create the first chunk program header:
    vector<unsigned char> programHeader(1,KEYCODE_BASICHEADER); // ={0x16};
    {
        string sgname = Utf8ToSegascii(_FileName);
        int i;
        for(i=0;i<16;i++)
        {
            if(sgname[i]==0) break;
            programHeader.push_back((unsigned char)sgname[i]);
        }
        // padded with spaces...
        for(;i<16;i++) programHeader.push_back((unsigned char)' ');
    }
    // _ProgramLength
    programHeader.push_back((unsigned char)(_ProgramLength>>8));
    programHeader.push_back((unsigned char)_ProgramLength);

    parity=0;
    for(size_t i=1; i<programHeader.size() ; i++)
    {
        parity += (unsigned char) programHeader[i];
    }
    programHeader.push_back((unsigned char)(-parity));
    programHeader.push_back(0); // want 2 zeros at the end.
    programHeader.push_back(0);

    bytes.push_back(programHeader);
    bytes.push_back(programBin);

    //TODO: manage 4 wave formats... (8b, be16b le16b, lefloat)
    // TODO: manage katakana file names !!!


}


