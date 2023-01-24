
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
using namespace std;

typedef

#define STRUCT_SMS_TYPE_SIZE 256
#define MACHINE_MAPPER_REGS_COUNT_FOR_SG 8

int main(int argc, char **argv)
{
    if(argc<3) {
     cout << "this patches a meka emulator basic state to enter basic bin into." << endl;
        cout << "mekastateloader blv3state.SXX mybasicbin.bdbin " << endl;
        return 1;
    }
    //
     vector<uint8_t> vState;
    {
        ifstream stateifs(argv[1],ios::binary|ios::in);
        if(!stateifs.good())
        {
            cout << "can't read meka state" << endl;
            return 1;
        }
        stateifs.seekg(0,istream::end);
        size_t statefsize = (size_t)stateifs.tellg();
        stateifs.seekg(0,istream::beg);
        vState.resize(statefsize);
        stateifs.read((char*)vState.data(),statefsize);
        stateifs.close;
    }
    if (strcmp((char*)vState.data(), "MEKA") != 0)
    {
        cout << "file not meka" << endl;
        return 1;
    }
    uint8_t version = vState[5];
    cout << "meka version:"<< (int)version<< endl;
    uint8_t driverId = vState[6];
    cout << "machine id:"<< (int)driverId<< endl;

    // from Load_Game_MSV
    size_t i=7;
    i+=sizeof(uint32_t); // crc
    i+= STRUCT_SMS_TYPE_SIZE; // just jump, very shitty format ! and size differ against compile options

    if(version >= 0x0E)
    {

        // Always save at least 4 so that legacy software can readily bump up version and read data for most games (apart from those using mappers with >4 registers)
        const int mappers_regs_to_save =
            // (g_machine.mapper_regs_count <= 4) ? 4 : g_machine.mapper_regs_count;
        //fread(&g_machine.mapper_regs[0], sizeof(u8), mappers_regs_to_save, f);
        i+= MACHINE_MAPPER_REGS_COUNT_FOR_SG;
    }
    if (version >= 0x0D)
    {
        //u16 w;
        //fread (&w, sizeof (u16), 1, f);
        //tsms.VDP_Line = w;
        i+= 2;
    }
    size_t mapramstart = i;
    size_t mapramSize = 0x01000; // 4kb ? Why ? It's either 16 or 32 with basics, and varies from carts.
    i+= mapramSize;

     // then VRAM 16k ...
     i += 0x04000;
     // then palette
     i += 32; // Verify

     // then PSG data
     i += 4 + 4*(2+1+4); // 4 or 8 for long int ??
     // Fm data load
     i +=64;
     // ports
     i++;
     // backed memory
    //     fread (SRAM, sms.SRAM_Pages * 0x2000, 1, f);
    int nbRamPages = 1;
    size_t bramstart = i;
    size_t bramSize = 0x02000 * nbRamPages; // acording to sms.SRAM_Pages

    // - - -
    ifstream bdbinifs(argv[2],ios::binary|ios::in);
    if(!bdbinifs.good())
    {
        cout << "can't read basic bin" << endl;
    }
    bdbinifs.seekg(0,istream::end);
    size_t bdbinfsize = (size_t)bdbinifs.tellg();
    bdbinifs.seekg(0,istream::beg);
    vector<uint8_t> vBasicBin(bdbinfsize,0);
    bdbinifs.read((char*)vBasicBin.data(),bdbinfsize);

    size_t ofs_basicPointersInRam= 0x08160-0x08000;
    size_t ofs_basicInRam= 0x09800-0x08000;
    uint16_t *pPointers = (uint16_t *)(vState.data() +(bramstart + ofs_basicPointersInRam));
    // basic start
    if(pPointers[0] != 0x09800)
    {
        cout << "pointer not in place, index not good for basic state, exiting" << endl;
        return 1;
    }
    pPointers[0]= 0x09800 ;
    // basic end
    pPointers[1]= 0x09800 + (uint16_t) bdbinifs.size();
    // vars start
    pPointers[2]= pPointers[1]+2 ;
    // var end (no vars)
    pPointers[3]= pPointers[3];
    // copy prog (basic+attached asm in bdbin)
    for(uint16_t j=0;j<(uint16_t)bdbinifs.size(); j++)
    {
        vState[bramstart+ofs_basicInRam+j]=bdbinifs[j];
    }
    // save back
    ofstream stateofs(argv[1],ios::binary|ios::out);
    if(!stateofs.good())
    {
        cout << "can't write meka state back" << endl;
        return 1;
    }
    stateofs.write((const char *)vState.data(),vState.size());

    return 0;
}
