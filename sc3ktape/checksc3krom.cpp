#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

int main(int argc, char **argv)
{
    if(argc<2) {
        cout << "param is 8/16/32kb rom file" << endl;
        return 1;
    }
    vector<uint8_t> rom ; // (statefsize,0);

    // with this paragraph, ifstream close accordingly.
    {
        ifstream romifs(argv[1],ios::binary| ios::in);
        if(!romifs.good()) {
            cout << "can't read file :" << argv[1] << endl;
            return 1;
        }

        romifs.seekg(0,istream::end);
        size_t statefsize = (size_t)romifs.tellg();
        romifs.seekg(0,istream::beg);

        if(statefsize<2) {
            cout << "file too short :" << argv[1] << endl;
            return 1;
        }
        cout << "file size: " <<statefsize << endl;
        rom.resize(statefsize);
        romifs.read((char *)rom.data(),statefsize);
    }

    uint8_t lastbyte = rom[rom.size()-1];

    uint8_t binsum=0;
    for(size_t i=0 ;i<rom.size()-1 ; i++)
    {
        binsum += rom[i];
    }
    uint8_t parity = (uint8_t)(-binsum);

    cout << "lastbyte : " << (uint32_t) lastbyte << endl;
    cout << "parity : " << (uint32_t) parity << endl;

    if(parity == lastbyte)
    {
        cout << "parity already ok , do nothing" << endl;
        return 0;
    }
    // - - - -
    cout << " ... update file with correct parity ..." << endl;
    rom[rom.size()-1] = parity;

    // - -  - rewrite !
    ofstream romofs(argv[1],ios::binary| ios::out);
    if(!romofs.good()) {
        cout << "can't write file :" << argv[1] << endl;
        return 1;
    }
    romofs.write((const char *)rom.data(),rom.size() );

    return 0;
}
