#include <SDL_image.h>
#include "ioMod.h"
#include "gamedata.h"
#include "renderContext.h"

IOmod& IOmod::getInstance() {
    static IOmod instance;    //create a single instance
    return instance;          //return the instance
}

IOmod::~IOmod() {
    TTF_CloseFont(font);      //close the font file
    TTF_Quit();               //quit the font
}

IOmod::IOmod() :
    //initialize the font
    init(TTF_Init()),
    //initialize the renderer
    renderer( RenderContext::getInstance()->getRenderer() ),
    //get the font file and font size from xml data
    font(TTF_OpenFont(Gamedata::getInstance().getXmlStr("font/file").c_str(),
                        Gamedata::getInstance().getXmlInt("font/size"))),
    textColor({255, 255, 255, 0})
{
    if ( init == -1 ) {
        //font wouldn't initialize
        throw std::string("error: Couldn't init font");
    }
    if (font == NULL) {
        //font initialized, but the file wasn't found
        throw std::string("error: font not found");
    }
    //get font color
    textColor.r = Gamedata::getInstance().getXmlInt("font/red");
    textColor.g = Gamedata::getInstance().getXmlInt("font/green");
    textColor.b = Gamedata::getInstance().getXmlInt("font/blue");
    textColor.a = Gamedata::getInstance().getXmlInt("font/alpha");
}

SDL_Texture* IOmod::readTexture(const std::string& filename) {
    //load the texture file, using the string in the xml file
    SDL_Texture *texture = IMG_LoadTexture(renderer, filename.c_str());
    if ( texture == NULL ) {
        //couldn't load
        throw std::string("Couldn't load ") + filename;
    }

    //return loaded texture
    return texture;
}

SDL_Surface* IOmod::readSurface(const std::string& filename) {
    //load a surface from the xml file
    SDL_Surface *surface = IMG_Load(filename.c_str());
    if ( !surface ) {
        //couldn't load the surface
        throw std::string("Couldn't load ") + filename;
    }

    //return the surface
    return surface;
}

void IOmod::writeText(const std::string& msg, int x, int y, int width) const {
    //write some text to the screen

    //make a new surface, which will become a texture
    SDL_Surface* surface =
        //render a message in the selected color
        TTF_RenderText_Blended_Wrapped(font, msg.c_str(), textColor, width);

    //make a texture from the surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    //get the width and height of the text
    int textWidth = surface->w;
    int textHeight = surface->h;

    //remove the surface - we're done with it
    SDL_FreeSurface(surface);
    //make a rectangle with the size of the text width and height
    SDL_Rect dst = {x, y, textWidth, textHeight};

    //use the dst rectangle to get the size of the text texture
    //and copy it to the screen.
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    //rotated sprite
    /*SDL_Point pt = {x, y};
    SDL_RenderCopyEx(renderer, texture, NULL, &dst, 180, &pt,
    SDL_RendererFlip(SDL_FLIP_NONE)); //bkmrk */


    //remove the texture - we're done with it
    SDL_DestroyTexture(texture);

}

void IOmod::writeText(const std::string& msg, int x, int y,
    const SDL_Color& color) {
    //write some colorful text to the screen
    SDL_Color tempColor = textColor;
    textColor = color;
    writeText(msg, x, y);
    textColor = tempColor;

}
