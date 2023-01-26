
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <string.h>
//#include
using namespace std;

#define STRUCT_SMS_TYPE_SIZE 96
#define MACHINE_MAPPER_REGS_COUNT_FOR_SG 8

namespace meka {
typedef union
{
    #ifdef LSB_FIRST
      struct { uint8_t l,h; } B;
    #else
      struct { uint8_t h,l; } B;
    #endif
      uint16_t W;
    } pair;

    typedef struct
    {
      pair AF, BC, DE, HL, IX, IY, PC, SP;  /* Mœain registers      */
      pair AF1, BC1, DE1, HL1;              /* Shadow registers    */
      uint8_t IFF, I;                          /* Interrupt registers */
      uint8_t R, R7;                           /* Refresh register    */ /* Copy of 7th bit of R assigned by user */

      int  IPeriod, ICount; /* Set IPeriod to number of CPU cycles */
                            /* between calls to LoopZ80()          */
      int  IBackup;         /* Private, don't touch                */
      uint16_t IRequest;        /* Set to address of pending IRQ       */

      /* Miscellaneous */
      uint8_t IAutoReset;      /* Set to 1 to autom. reset IRequest   */
      uint8_t TrapBadOps;      /* Set to 1 to warn of illegal opcodes */
      uint16_t Trap;            /* Set Trap to address to trace from   */
      uint8_t Trace;           /* Set Trace=1 to start tracing        */
      void *User;           /* Arbitrary user data (ID,RAM*,etc.)  */
    } Z80;


    // Variables needed by one emulated SMS
    // FIXME: reconceptualize those stuff, this is pure, old crap
    struct SMS_TYPE
    {
        // CPU State
        Z80   R;                              // CPU Registers (Marat Faizullin)

        // Other State
        uint8_t      VDP [16];                      // VDP Registers
        uint8_t      __UNUSED__PRAM_Address;        // Current palette address
        // NOTE: variable below (VDP_Status) is modified from videoasm.asm, do NOT move it
        uint8_t      VDP_Status;                    // Current VDP status
        uint16_t     VDP_Address;                   // Current VDP address
        uint8_t      VDP_Access_Mode;               // 0: Address Low - 1: Address High
        uint8_t      VDP_Access_First;              // Address Low Latch
        uint8_t      VDP_ReadLatch;                 // Read Latch
        uint8_t      VDP_Pal;                       // Currently Reading Palette ?
        uint8_t      Country;                       // 0: English - 1: Japanese
        int     Lines_Left;                    // Lines Left before H-Blank
        uint8_t      Pending_HBlank;                // Pending HBL interrupt
        uint8_t      Pending_NMI;                   // Pending NMI interrupt (for Coleco emulation)
        uint8_t      Glasses_Register;              // 3-D Glasses Register
        uint8_t      SRAM_Pages;                    // SRAM pages used
        uint8_t      SRAM_Mapping_Register;         // SRAM status + mapping offset
        uint8_t      FM_Magic;                      // FM Latch (for detection)
        uint8_t      FM_Register;                   // FM Register
        uint8_t      Input_Mode;   // Port 0xDE     // 0->6: Keyboard - 7: Joypad
    };


};

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
        stateifs.close();
    }
    if (strncmp((char*)vState.data(), "MEKA",4) != 0)
    {
        cout << "file not meka" << endl;
        return 1;
    }
    uint8_t version = vState[5];
    cout << "meka version:"<< (int)version<< endl;
    uint8_t driverId = vState[6];
    cout << "machine id:"<< (int)driverId<< endl;
  // 3: DRV_SC3000 2: SG1000

enum t_machine_driver
{
  DRV_SMS   = 0,
  DRV_GG      = 1,
  DRV_SG1000  = 2,
  DRV_SC3000  = 3,
  DRV_COLECO  = 4,
  DRV_MSX___  = 5,
  DRV_NES___  = 6,
  DRV_SF7000  = 7,
  DRV_MAX   = 8,
};


    // from Load_Game_MSV
    size_t i=7;
    i+=sizeof(uint32_t); // crc


    meka::SMS_TYPE sms;
    memcpy((char *)&sms,vState.data()+i,sizeof(sms));
    i+= sizeof(sms);

    cout << "sms ram 8kb pages: " << (int)sms.SRAM_Pages << endl;

    //i+= STRUCT_SMS_TYPE_SIZE; // just jump, very shitty format ! and size differ against compile options

    if(version >= 0x0E)
    {

        // Always save at least 4 so that legacy software can readily bump up version and read data for most games (apart from those using mappers with >4 registers)
        //  const int mappers_regs_to_save = (g_machine.mapper_regs_count <= 4) ? 4 : g_machine.mapper_regs_count;

        int mappers_regs_to_save = 4;
        if(driverId == DRV_SC3000) mappers_regs_to_save=4;



                // (g_machine.mapper_regs_count <= 4) ? 4 : g_machine.mapper_regs_count;
        //fread(&g_machine.mapper_regs[0], sizeof(u8), mappers_regs_to_save, f);
        i+= mappers_regs_to_save;
    }
    if (version >= 0x0D)
    {
        //u16 w;
        //fread (&w, sizeof (u16), 1, f);
        //tsms.VDP_Line = w;
        i+= 2;
    }
    /*
    switch (g_machine.mapper)
    {
    case MAPPER_32kRAM:
    case MAPPER_SC3000_Survivors_Multicart:
        fread (RAM, 0x08000, 1, f);
        break;
    }*/
    // for SC3k Basic Level3, 32k

    size_t mapramstart = i;
    size_t mapramSize = 0x08000; // 4kb ? Why ? It's either 16 or 32 with basics, and varies from carts.
    i+= mapramSize;

    // that's what we searched

/*

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

    size_t bramstart = i;
    size_t bramSize = 0x02000 * sms.SRAM_Pages; // acording to
*/
    // - - -
    ifstream bdbinifs(argv[2],ios::binary|ios::in);
    if(!bdbinifs.good())
    {
        cout << "can't read basic bin" << endl;
        return 1;
    }
    bdbinifs.seekg(0,istream::end);
    size_t bdbinfsize = (size_t)bdbinifs.tellg();
    bdbinifs.seekg(0,istream::beg);
    vector<uint8_t> vBasicBin(bdbinfsize,0);
    bdbinifs.read((char*)vBasicBin.data(),bdbinfsize);

    size_t ofs_basicPointersInRam= 0x08160-0x08000;
    size_t ofs_basicInRam= 0x09800-0x08000;
    uint16_t *pPointers = (uint16_t *)(vState.data() +(mapramstart + ofs_basicPointersInRam));
    // basic start
    if(pPointers[0] != 0x09800)
    {
        cout << "pointer not in place, index not good for basic state, exiting" << endl;
        return 1;
    }
    pPointers[0]= 0x09800 ;
    // basic end
    pPointers[1]= 0x09800 + (uint16_t) vBasicBin.size();
    // vars start
    pPointers[2]= pPointers[1]+2 ;
    // var end (no vars)
    pPointers[3]= pPointers[3]+1;
    // copy prog (basic+attached asm in bdbin)
    for(uint16_t j=0;j<(uint16_t)vBasicBin.size(); j++)
    {
        vState[mapramstart+ofs_basicInRam+j]=vBasicBin[j];
    }
    // save back
    ofstream stateofs(argv[1],ios::binary|ios::out);
    if(!stateofs.good())
    {
        cout << "can't write meka state back" << endl;
        return 1;
    }
    stateofs.write((const char *)vState.data(),vState.size());

    cout << "meka state patch ok." << endl;

    return 0;
}
