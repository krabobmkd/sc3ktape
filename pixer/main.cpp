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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <SDL.h>

#include "TMS9918State.h"

#include "log.h"
//#include <jsoncpp/json/json.h>

//#include "SegaAsciiJpToUnicode.h"
//#include "PetsciiToUnicode.h"

 using namespace std;



int main(int argc, char *argv[])
{
    Log::global().addLogListener(std::cout,{eError,eWarning,eInfo});
    Log::global().setWarningErrorAutoColor(true);
//    if(argc<2)
//    {
//        LOGI() << " - \n";
//        LOGI() << endl;
//        return 0;
//    }

    TMS9918State tms;
    tms.setMode_Graphics2Default();

    tms.vpoke(0,129);
    tms.vpoke(1,24);
    tms.vpoke(7,36);
    tms.vpoke(8*3,129);
    tms.vpoke(8*33+7,255-129);

    tms.vpoke(0+0x2000,0x47);

    tms.updateRender(); // would alloc buffers


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

