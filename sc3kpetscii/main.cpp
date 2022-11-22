/*  Written by Vic Ferry aka Krabob/Mkd - 2022
 This relase is under APACHE LICENSE , Version 2.0 https://www.apache.org/licenses/LICENSE-2.0
  _________                       __________________ ________ _______  _______  _______
 /   _____/ ____   _________     /   _____/\_   ___ \\_____  \\   _  \ \   _  \ \   _  \
 \_____  \_/ __ \ / ___\__  \    \_____  \ /    \  \/  _(__  </  /_\  \/  /_\  \/  /_\  \
 /        \  ___// /_/  > __ \_  /        \\     \____/       \  \_/   \  \_/   \  \_/   \
/_______  /\___  >___  (____  / /_______  / \______  /______  /\_____  /\_____  /\_____  /
        \/     \/_____/     \/          \/         \/       \/       \/       \/       \/

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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "log.h"
#include <jsoncpp/json/json.h>

#include "SegaAsciiJpToUnicode.h"
#include "PetsciiToUnicode.h"

 using namespace std;


struct PetsciiFrame
{
    int _width=0,_height=0;
    int _bgcolor=0,_bordercolor=0;
    bool _charsetUpper=true;
    std::string _name;
    std::vector<char> _chars;

    void readFromJson(Json::Value &jsf)
    {
        if(jsf.isMember("width")) _width = jsf["width"].asInt();
        else throw runtime_error("can't find data in json");
        if(jsf.isMember("height")) _height = jsf["height"].asInt();
        if(jsf.isMember("backgroundColor")) _bgcolor = jsf["backgroundColor"].asInt();
        if(jsf.isMember("borderColor")) _bordercolor = jsf["borderColor"].asInt();
        if(jsf.isMember("name")) _name = jsf["name"].asString();
        if(jsf.isMember("charset"))
        {
            _charsetUpper = (jsf["charset"].asString() == "upper");
        }
        Json::Value jsa = jsf["screencodes"];
        if(jsa.isArray() )
        {
            int s = jsa.size();
            _chars.resize(s);
            for(int i=0;i<s;i++)
            {
                _chars[i]=(char)jsa[i].asInt();
            }
        }
    }
    void readJsonFile(std::string inputfile)
    {
        using namespace std;

        Json::Reader jsonrd;
        Json::Value jsroot;

        ifstream inputstream(inputfile.c_str(),ios::binary);
        if(!inputstream.good()) throw runtime_error("can't open file");

        ostringstream sstr;
        sstr << inputfile;
        string jsonDocStr = sstr.str();

        bool res = jsonrd.parse(jsonDocStr, jsroot/*, bool collectComments = true*/);
        if(!res) {
            throw runtime_error("can't get json from file");
        }
        if(jsroot.isObject())
        {
            Json::Value frbufs = jsroot["framebufs"];
            if(frbufs.isArray() && frbufs.size()>0 )
            {
                Json::Value  frbuf = frbufs[0];
                readFromJson(frbuf);
            }
        }

    }
//    "width":40,"height":25,"backgroundColor":0,"borderColor":14,
//   "charset":"upper","name":"screen_001",
}; // end class Frame

class SegasciiExporter
{
public:
    void exportFrameToBasic(const PetsciiFrame &f, std::ostream &ofs)
    {
        ofs << " screen 2,2:cls:cursor0,0\n";
        // translate chars to print strings

        // translate colors to vpoke & data
/*
VRAM 16k
&H0000 mode 2 pattern generator table (6144b) -> le bitmap 256x192
&H1800 CASE mode 1 (text mode) pattern generator table (2048b)
       CASE mode 2: sprite generator table

&H2000 mode 2 color table (6144) (4bit color on, 4 bit color off), per 8 pixels.
&H3800 mode 2 "pattern name table" (768b, 32x24)
&H3B00 Sprite attribute table 128b.
&H3C00 text mode pattern name table (960b) -> text mode chars ! (6x8 pixels/char, 40*24 charx, 256x192)
$28 ->40 colonnes

 le charset petscii = 8*128 = 1kb


*/


    }
};



int main(int argc, char *argv[])
{
    Log::global().addLogListener(std::cout,{eError,eWarning,eInfo});
    Log::global().setWarningErrorAutoColor(true);
    if(argc<2)
    {

       LOGI()<< ANSICOL_YELLOW <<" ____   ___  ____  __ _  ____  ____  ____  ____   ___  __  __\n";
       LOGI()<< ANSICOL_YELLOW <<"/ ___) / __)( __ \\(  / )(  _ \\(  __)(_  _)/ ___) / __)(  )(  )\n";
       LOGI()<< ANSICOL_YELLOW <<"\\___ \\( (__  (__ ( )  (  ) __/ ) _)   )(  \\___ \\( (__  )(  )(\n";
       LOGI()<< ANSICOL_YELLOW <<"(____/ \\___)(____/(__\\_)(__)  (____) (__) (____/ \\___)(__)(__)\n";
        LOGI()<< ANSICOL_DEF <<"\n";
        LOGI() << " Sega SC-3000 / SK1100 petmate json export to Basic converter v0.9 2022\n";
        LOGI() << " hopefully translate petscii screen to SegaScii screen \n";
        LOGI() << " - experimental and in dev- \n";
        LOGI() << endl;
        return 0;
    }
    string inputfile(argv[1]);

    try {
        PetsciiFrame f;
        f.readJsonFile(inputfile);


        string outBasicFilePath = inputfile + ".sc.bas";
        ofstream ofs(outBasicFilePath,ios::binary|ios::out);
        if(!ofs.good())
        {
            stringstream ss;
             ss << "can't write file: " << outBasicFilePath;
            throw runtime_error(ss.str());
        }

        SegasciiExporter exporter;
        exporter.exportFrameToBasic(f,ofs);

    } catch(const std::exception &e)
    {
        string err = e.what();
        LOGE() << "error: " <<err<< endl;
        return 1;
    }
    return 0;

}
