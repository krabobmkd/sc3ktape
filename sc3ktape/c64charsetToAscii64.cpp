#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

int main(int argc, char **argv)
{
    if(argc<3) {
        cout << "this load a .c64 charset, reorder to ascii char [64,95]" << endl;
        cout << "c64charsettoascii asciicharset charset.c64 [fallback c64 charset]" << endl;
        return 1;
    }
    //
    vector<char> vC64,vC64f; // and fallback
    {
        ifstream stateifs(argv[2],ios::binary|ios::in);
        if(!stateifs.good())
        {
            cout << "can't read charset" << endl;
            return 1;
        }
        stateifs.seekg(0,istream::end);
        size_t statefsize = (size_t)stateifs.tellg();
        stateifs.seekg(0,istream::beg);
        vC64.resize(statefsize);
        stateifs.read((char*)vC64.data(),statefsize);
        stateifs.close();
    }
    if(argc>3)
    {
        ifstream stateifs(argv[3],ios::binary|ios::in);
        if(!stateifs.good())
        {
            cout << "can't read charset2" << endl;
            return 1;
        }
        stateifs.seekg(0,istream::end);
        size_t statefsize = (size_t)stateifs.tellg();
        stateifs.seekg(0,istream::beg);
        vC64f.resize(statefsize);
        stateifs.read((char*)vC64f.data(),statefsize);
        stateifs.close();
    }

    string c64order=
        "@ABCDEFGHIJKLMNO"
        "PQRSTUVWXYZ[&]~~" // too arrow at the end. ~ is not managed in (32,96)
       " !\"#$%&'()*+,-./"
        "0123456789:;<=>?"
            // then petscii things
    ;


    vector<char> asciics(64*8,0);
    for(size_t i=0;i<64;i++)
    {
        char asciishift=32+(char)i;
        size_t charinpet=c64order.find(asciishift);
        if(charinpet==string::npos) charinpet=32; // also space , if not found.        
        const char *pInPet = vC64.data()+(2+charinpet*8); // asciics[i]
        bool isAllSpaces = true;
        for(size_t j=0;j<8;j++) {
            if(pInPet[j]!=0) isAllSpaces=false;
        }
        if(asciishift != ' ' && isAllSpaces && vC64f.size()>2+charinpet*8)
        {
            // try fallback font
            pInPet = vC64f.data()+(2+charinpet*8);
        }

        for(size_t j=0;j<8;j++)
        {
            asciics[i*8+j] = *pInPet++;
        }
    }
    ofstream ofs(argv[1], ios::binary| ios::out);
    if(!ofs.good())
    {
        cout << "can't write file" << endl;
        return 1;
    }
    ofs.write(asciics.data(),asciics.size());
    return 0;
}
