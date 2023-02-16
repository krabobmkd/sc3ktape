/*  Written by Vic Ferry aka Krabob/Mkd - 2023
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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <SDL.h>
#include <SDL_image.h>

#include "TMS9918State.h"
#include "TMS9918_SC2Loader.h"
#include "TMS9918_SC3KExport.h"

#include "log.h"
//#include <jsoncpp/json/json.h>

//#include "SegaAsciiJpToUnicode.h"
//#include "PetsciiToUnicode.h"

 using namespace std;


string trimfileName(string pathn)
{
    string s = pathn;
    size_t i = s.find_last_of("/\\");
    if(i != string::npos)
    {
        s = s.substr(i+1);
    }
    return s;
}

int main(int argc, char *argv[])
{
    Log::global().addLogListener(std::cout,{eError,eWarning,eInfo});
    Log::global().setWarningErrorAutoColor(true);
    if(argc<2)
    {
        LOGI() << "pixer need .sc2 or .gif file as arg1 \n";
        LOGI() << endl;
        return 0;
    }
    string filepath = argv[1];

    bool isSc2 = (filepath.rfind(".sc2") == filepath.length()-4);
    if(filepath.rfind(".SC2") == filepath.length()-4) isSc2 = true;
    bool isGif = (filepath.rfind(".gif") == filepath.length()-4);
    if(filepath.rfind(".GIF") == filepath.length()-4) isGif = true;

    vchip::TMS9918State tms;

    TMS_SC2Loader tmsLoader(tms);

    if(isSc2)
    {

        try {
            ifstream ifs(filepath, ios::binary|ios::in);
            if(!ifs.good())
            {
                stringstream ss;
                ss << "can't read .sc2 image file :" << filepath << endl;
                throw runtime_error(ss.str());
            }

            tmsLoader.load(ifs);

            TMS_Compressor exporter(tms);
            string exportname = trimfileName(filepath) + ".asm";

            ofstream exportOfs(exportname, ios::binary|ios::out);
            exporter.compressGraphics2();
            exporter.exportAsm(exportOfs,"gfx");

        } catch(const std::exception &e)
        {
            cout << "exc: " << e.what() << endl;
        }
    }
    if(isGif)
    {

        try {
            /*ifstream ifs(filepath, ios::binary|ios::in);
            if(!ifs.good())
            {
                stringstream ss;
                ss << "can't read .sc2 image file :" << filepath << endl;
                throw runtime_error(ss.str());
            }
            tmsLoader.load(ifs);
            */

            SDL_Surface * pImageToConv = IMG_Load(filepath.c_str());
            cout << "pixel depth:" << (int) pImageToConv->format->BitsPerPixel << endl;


            TMS_Compressor exporter(tms);
            string exportname = trimfileName(filepath) + ".asm";

            ofstream exportOfs(exportname, ios::binary|ios::out);
            exporter.compressGraphics2();
            exporter.exportAsm(exportOfs,"gfx");

        } catch(const std::exception &e)
        {
            cout << "exc: " << e.what() << endl;
        }
    }


    tms.updateRender(); // would alloc rgb buffers


 //   SDL_Surface * imageToConv = IMG_Load("image.gif");

    SDL_Window * window=nullptr;
    SDL_Renderer * renderer = nullptr;
  //  SDL_Surface *tmsImage = nullptr;
    SDL_Texture * screen_texture = nullptr;


    try {
        bool quit = false;
        SDL_Event event;

        SDL_Init(SDL_INIT_VIDEO);

        window = SDL_CreateWindow("SDL2 Displaying Image",
         SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  800, 600,
                                  SDL_WINDOW_RESIZABLE);

        if(!window) throw runtime_error("no window");
        renderer = SDL_CreateRenderer(window, -1,
                                      SDL_RENDERER_PRESENTVSYNC//0
                                      );
        if(!renderer) throw runtime_error("no window");

        SDL_SetWindowMinimumSize(window, tms.pixelWidth(), tms.pixelHeight());
        SDL_RenderSetLogicalSize(renderer, tms.pixelWidth(), tms.pixelHeight());
//        SDL_RenderSetIntegerScale(renderer, 1);

        screen_texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888, // On little endian
            SDL_TEXTUREACCESS_STREAMING,
            tms.pixelWidth(), tms.pixelHeight());

        // SDL_PIXELFORMAT_RGBA8888
                //SDL_LoadBMP("image.bmp");
       // texture = SDL_CreateTextureFromSurface(renderer, tmsImage);

        while (!quit)
        {
           // menu.draw(screen.getSurface());
          //  SDL_Flip(screen.getSurface());
           // SDL_Delay(10);

            SDL_WaitEvent(/*&event*/nullptr);
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT) quit = true;
                if (event.type == SDL_KEYUP)
                {
                    if (event.key.keysym.sym == SDLK_q)
                    {
                        quit = true;
                    }
                }
            }

           // tms.updateRender();

            //
            SDL_RenderClear(renderer);
            SDL_UpdateTexture(screen_texture, NULL, tms.pixelRGBA(), tms.pixelWidth() * 4);
            SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
            SDL_RenderPresent(renderer);

        } // frame loop


    } catch(const std::exception &e)
    {
        string err = e.what();
        LOGE() << "error: " <<err<< endl;
        return 1;
    }

    if(screen_texture) SDL_DestroyTexture(screen_texture);
 //   if(tmsImage) SDL_FreeSurface(tmsImage); //need tms to be delete after
    if(renderer) SDL_DestroyRenderer(renderer);
    if(window) SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;

}

