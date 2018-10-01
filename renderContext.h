#ifndef RENDERCONTEXT__H
#define RENDERCONTEXT__H

#include <iostream>
#include <SDL.h>
#include "imageFactory.h"

/*const int WIDTH = 854;
const int HEIGHT = 480;
const int DELAY = 1000;*/

class RenderContext{
public:
    static RenderContext* getInstance();    //meyers singleton
    ~RenderContext();   //destructor

    SDL_Window* getWindow() const { return window; }    //current window

    SDL_Renderer* getRenderer() const { return renderer; }  //return the
                                                            //current renderer

    Image* getImage(const std::string& n) {
        return factory->getImage(n);
    }
    std::vector<Image*> getImages(const std::string& n) {
        return factory->getImages(n);
    }
private:
    static RenderContext* instance;
    SDL_Window* window;
    SDL_Renderer* renderer;

    const int WIDTH;
    const int HEIGHT;
    //const int DELAY;

    ImageFactory* factory;

    SDL_Window* initWindow();
    SDL_Renderer* initRenderer();
    RenderContext();
    RenderContext(const RenderContext&) = delete;
    RenderContext& operator=(const RenderContext&) = delete;
};

#endif
