#ifndef IOMOD__H
#define IOMOD__H

#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

class IOmod {
public:
    static IOmod& getInstance();  //meyers singleton
    ~IOmod();
    SDL_Texture* readTexture(const std::string& filename);    //get a texture
    SDL_Surface* readSurface(const std::string& filename);    //get a surface

    //write some text
    void writeText(const std::string&, int, int, int = 1920) const;
    void writeText(const std::string&, int, int, const SDL_Color&);
    SDL_Renderer* getRenderer() const { return renderer; }    //get renderer
private:
    int init;                             //whether or not the font loaded
    SDL_Renderer* renderer;               //the renderer to be used
    TTF_Font* font;                       //the font to be used
    SDL_Color textColor;                  //the text color to be used
    IOmod();                              //no default constructor
    IOmod(const IOmod&);                  //no copy constructor
    IOmod& operator=(const IOmod&);       //no assignment operator
};

#endif
