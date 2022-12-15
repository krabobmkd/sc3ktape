<<<<<<< HEAD
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
    tms.updateRender();


    SDL_Window * window=nullptr;
    SDL_Renderer * renderer = nullptr;
    SDL_Surface *tmsImage = nullptr;
    SDL_Texture * texture = nullptr;


    try {
        bool quit = false;
        SDL_Event event;

        SDL_Init(SDL_INIT_VIDEO);

        window = SDL_CreateWindow("SDL2 Displaying Image",
         SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, 0);
        if(!window) throw runtime_error("no window");
        renderer = SDL_CreateRenderer(window, -1, 0);
        if(!renderer) throw runtime_error("no window");

        SDL_Surface* SDL_CreateRGBSurfaceWithFormat
            (Uint32 flags, int width, int height, int depth, Uint32 format);

        tmsImage = SDL_CreateRGBSurfaceWithFormat
                ( 0,//Uint32 flags,
                 tms.pixelWidth(),//int width,
                 tms.pixelHeight(),//int height,
                 32, // int depth,
                 SDL_PIXELFORMAT_ABGR8888
                  //Uint32 format
                  );

        // SDL_PIXELFORMAT_RGBA8888
                //SDL_LoadBMP("image.bmp");
        texture = SDL_CreateTextureFromSurface(renderer, tmsImage);

        while (!quit)
        {
            SDL_WaitEvent(&event);

            switch (event.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            }
        }


    } catch(const std::exception &e)
    {
        string err = e.what();
        LOGE() << "error: " <<err<< endl;
        return 1;
    }

    if(texture) SDL_DestroyTexture(texture);
    if(image) SDL_FreeSurface(image);
    if(renderer) SDL_DestroyRenderer(renderer);
    if(window) SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;

}
=======
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
    tms.setMode2Default();
    tms.updateRender(); // would alloc buffers


    SDL_Window * window=nullptr;
    SDL_Renderer * renderer = nullptr;
    SDL_Surface *tmsImage = nullptr;
    SDL_Texture * texture = nullptr;


    try {
        bool quit = false;
        SDL_Event event;

        SDL_Init(SDL_INIT_VIDEO);

        window = SDL_CreateWindow("SDL2 Displaying Image",
         SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, 0);
        if(!window) throw runtime_error("no window");
        renderer = SDL_CreateRenderer(window, -1, 0);
        if(!renderer) throw runtime_error("no window");

//        inline const uint8_t *pixelRGBA()
          tmsImage = SDL_CreateRGBSurfaceFrom(
                    (void *)tms.pixelRGBA(), //void *pixels,
                    tms.pixelWidth(), // int width,
                    tms.pixelHeight(), //int height,
                    32, //int depth,
                    tms.pixelWidth()*4, //int pitch,
                    0xff000000,// Uint32 Rmask,
                    0x00ff0000,
                    0x0000ff00,//Uint32 Bmask,
                    0x000000ff//Uint32 Amask
                      );
        /*
        SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels,
                                              int width,
                                              int height,
                                              int depth,
                                              int pitch,
                                              Uint32 Rmask,
                                              Uint32 Gmask,
                                              Uint32 Bmask,
                                              Uint32 Amask);
    */
//        tmsImage = SDL_CreateRGBSurfaceWithFormat
//                ( 0,//Uint32 flags,
//                 tms.pixelWidth(),//int width,
//                 tms.pixelHeight(),//int height,
//                 32, // int depth,
//                 SDL_PIXELFORMAT_RGBA8888
//                  //Uint32 format
//                  );

        // SDL_PIXELFORMAT_RGBA8888
                //SDL_LoadBMP("image.bmp");
        texture = SDL_CreateTextureFromSurface(renderer, tmsImage);

        while (!quit)
        {
           // menu.draw(screen.getSurface());
            SDL_Flip(screen.getSurface());
            SDL_Delay(10);

            SDL_WaitEvent(&event);


            switch (event.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            }
            /*
       while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                shouldQuit = true;
            }
            else if (event.type == SDL_KEYUP)
            {
                if (event.key.keysym.sym == SDLK_q)
                {
                    shouldQuit = true;
                }
            }
        }
*/
        }


    } catch(const std::exception &e)
    {
        string err = e.what();
        LOGE() << "error: " <<err<< endl;
        return 1;
    }

    if(texture) SDL_DestroyTexture(texture);
    if(tmsImage) SDL_FreeSurface(tmsImage); //need tms to be delete after
    if(renderer) SDL_DestroyRenderer(renderer);
    if(window) SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;

}
>>>>>>> 0f1ea1d8600c70070bbeb4f5f150476c2015c7dd
