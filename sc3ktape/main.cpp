/* 🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭  🖭
 Written by Vic Ferry aka Krabob/Mkd - 2022
 This relase is under APACHE LICENSE , Version 2.0 https://www.apache.org/licenses/LICENSE-2.0
  _________                       __________________ ________ _______  _______  _______
 /   _____/ ____   _________     /   _____/\_   ___ \\_____  \\   _  \ \   _  \ \   _  \
 \_____  \_/ __ \ / ___\__  \    \_____  \ /    \  \/  _(__  </  /_\  \/  /_\  \/  /_\  \
 /        \  ___// /_/  > __ \_  /        \\     \____/       \  \_/   \  \_/   \  \_/   \
/_______  /\___  >___  (____  / /_______  / \______  /______  /\_____  /\_____  /\_____  /
        \/     \/_____/     \/          \/         \/       \/       \/       \/       \/
___________                         ________      __________               .__
\__    ___/____  ______   ____      \_____  \     \______   \_____    _____|__| ____
  |    |  \__  \ \____ \_/ __ \      /  ____/      |    |  _/\__  \  /  ___/  |/ ___\
  |    |   / __ \|  |_> >  ___/     /       \      |    |   \ / __ \_\___ \|  \  \___
  |____|  (____  /   __/ \___  >    \_______ \     |______  /(____  /____  >__|\___  >
               \/|__|        \/             \/            \/      \/     \/        \/

    “Somewhere, something incredible is waiting to be known.”
    ― Carl Sagan
*/

#pragma message("")
#pragma message("   ... now compiling...")
#pragma message("              ________  __     __")
#pragma message("  ______ ____ \\_____  \\|  | __/  |______  ______   ____ ")
#pragma message(" /  ____/ ___\\  _(__  <|  |/ \\   __\\__  \\ \\____ \\_/ __ \\")
#pragma message(" \\___ \\\\  \\___ /       |    < |  |  / __ \\|  |_> \\  ___/")
#pragma message("/____  >\\___  /______  |__|_ \\|__| (____  |   __/ \\___  >")
#pragma message("     \\/     \\/       \\/     \\/          \\/|__|        \\/")
#pragma message("")

#include "SC3KBasic.h"
#include "SoundReader.h"
#include <iostream>
#include <fstream>
#include "log.h"
 using namespace std;

// C:\Users\victo\Documents\WaveProgManager\Waves\ProgW_AAAABBB.wav -tolabel -o test.bas

int main(int argc, char *argv[])
{
    Log::global().addLogListener(std::cout,{eError,eWarning,eInfo});
    Log::global().setWarningErrorAutoColor(true);
    if(argc<2)
    {

        LOGI()<< ANSICOL_YELLOW <<"               ________  __     __\n";
        LOGI()<< ANSICOL_YELLOW <<"   ______ ____ \\_____  \\|  | __/  |______  ______   ____ \n";
        LOGI()<< ANSICOL_YELLOW <<"  /  ____/ ___\\  _(__  <|  |/ \\   __\\__  \\ \\____ \\_/ __ \\\n";
        LOGI()<< ANSICOL_YELLOW <<"  \\___ \\\\  \\___ /       |    < |  |  / __ \\|  |_> \\  ___/\n";
        LOGI()<< ANSICOL_ORA <<" /____  >\\___  /______  |__|_ \\|__| (____  |   __/ \\___  >\n";
        LOGI()<< ANSICOL_ORA <<"      \\/     \\/       \\/     \\/          \\/|__|        \\/\n";
        LOGI()<< ANSICOL_DEF <<"\n";

        LOGI() << " Sega SC-3000 / SK1100 Tape-to-basic-to-tape converter v0.9 2022\n";
        LOGI() << "   >sc3ktape file.wave -o basicfile.sc.bas  [options] convert wave to basic\n";
        LOGI() << "   >sc3ktape file.txt/.bas -o basicfile.wave [options] convert basic to wave\n";
        LOGI() << "   >sc3ktape file.bin -o file.wave [options] convert asm binary to wave\n";

        LOGI() << " tape to basic options:\n";
        LOGI() << "   -jp:      for japan basic, use utf8 katakana, else european dieresis.\n";
        LOGI() << "   -tolabel: replace lines number by tabs and generate labels (usable with b2t).\n";
        LOGI() << " basic to tape options:\n";
        LOGI() << "   -nNAME: give header prog file name for tape wave header.\n";
        LOGI() << " Input waves can use any frequencies (22050Hz,44100Hz), must be mono.\n";
        LOGI() << " Ouput waves are always 8bit/ 22050Hz, which is enough for the hardware.\n";
        LOGI() << endl;
        return 0;
    }

    string inputfile(argv[1]);
    ifstream inputstream(inputfile.c_str(),ios::binary);
    size_t rif = inputfile.rfind(".bin");
    bool endsWithbin = (rif!=string::npos && rif == inputfile.length()-4);
    string basepath = ".";
    size_t rifb = inputfile.find_last_of("\\/");
    if(rifb != string::npos) basepath = inputfile.substr(0,rifb);

    if(!inputstream.good())
    {
        LOGE() << "file "<<inputfile <<" not found "<< endl;
        return 1;
    }

    bool isSoundFormat = SoundReaderBroker::testStreamFormat(inputstream);

    //  - - - collect options - - - - -
    bool jpCharset=false;
    bool lineIndexToLabels=false;

    string programName=endsWithbin?"BINARY":"BASIC"; // default.

    //   bool useUtf8Encoding=true;
    string outputfile;
    for(int i=2;i<argc;i++)
    {
        string strarg(argv[i]);
        if(strarg=="-o")
        {
            i++;
            if(i<argc)
            {
                outputfile = argv[i];
            }
        }
        if(strarg.find("-n") == 0)
        {
            programName = strarg.substr(2);
        }
        if(strarg.find("-tolabel") == 0)
        {
            lineIndexToLabels = true;
        }
        if(strarg == "-jp") jpCharset=true;

    }

    try {
    if(isSoundFormat)
    {       // - - - - input is wave
            SC3KBasic wr;
            wr.setIsEuroAscii(!jpCharset);
            wr.readWave(inputstream);
            if(outputfile.length()==0)
            {
                outputfile = inputfile + ".sc.bas";
            }
            if(lineIndexToLabels)  wr.lineIndexToLabels();

            ofstream ofs(outputfile.c_str(), ios::binary);
            if( !ofs.good())
            {
                throw runtime_error(string("can't write to: ") + outputfile);
            }
            wr.writeBasic(ofs);
            LOGI() << "Tape Wave exported to Basic source: " << outputfile << endl;
            if(wr.hasPostBinary())
            {
                string postbinfilename = outputfile + ".post.bin";
                ofstream ofspb(postbinfilename.c_str(), ios::binary);
                if( !ofspb.good())
                {
                    throw runtime_error(string("can't write .post.bin file to: ") + postbinfilename);
                }
                wr.writePostBinary(ofspb);
                LOGI() << "Also exported extra post-binary of length: "<< wr.postBinaryLength()
                       <<" to binary file: " << postbinfilename << endl;
            }
    } else
    {
        SC3KBasic wr;
        wr.setSourceBasePath(basepath);
        if(endsWithbin)
        {   // assembler to tape wave case
            wr.readAsmBin(inputstream);

            std::string waveVersion = inputfile + ".wave";

            ofstream outputStream(waveVersion.c_str(), ios::binary);
            if(!outputStream.good()) {
                throw runtime_error(string("can't save to : ")+waveVersion);
            }
            wr.writeWave(outputStream,programName);
            LOGI() << "Binary Exported To Tape Wave: " << waveVersion << endl;

        } else
        { // basic to tape wave

            // - - - - input is basic.
            wr.setIsEuroAscii(!jpCharset);

            wr.readBasic(inputstream);
            std::string waveVersion = inputfile + ".wave";

            ofstream outputStream(waveVersion.c_str(), ios::binary);
            if(!outputStream.good()) {
                throw runtime_error(string("can't save to : ")+waveVersion);
            }
            wr.writeWave(outputStream,programName);
            LOGI() << "Basic Exported To Tape Wave: " << waveVersion << endl;
        }
    }
    } catch(const std::exception &e)
    {
        string err = e.what();
        LOGE() << "error: " <<err<< endl;
        return 1;
    }
    return 0;

}
