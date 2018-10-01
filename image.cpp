#include "drawable.h"
#include "image.h"
#include "ioMod.h"
#include "viewport.h"
#include "renderContext.h"
#include <math.h>

Image::Image( SDL_Surface* surf, SDL_Texture* tex ) :
    renderer(RenderContext::getInstance()->getRenderer()),
    surface( surf ),
    texture( tex ),
    view{0,0,surf->w,surf->h}
{ }

Image::Image( const Image& image ) :
    renderer(image.renderer),
    surface(image.surface),
    texture(image.texture),
    view(image.view)
{ }


Image& Image::operator=(const Image& rhs) {
    renderer = rhs.renderer;
    texture = rhs.texture;
    view = rhs.view;
    return *this;
}

void Image::draw(int x, int y) const {
    //re-call draw, but with a scale of "1"
    draw(x,y,1.0f);
}

void Image::drawRotate(int x, int y, float scale, int width, int height,
    int xVel, int yVel) const {
    //draw the current frame... ROTATED

    double xv = xVel;
    double yv = yVel;

    //get the x and y positions of the current view
    x -= Viewport::getInstance().getX();
    y -= Viewport::getInstance().getY();

    //scale the height to the current setting
    int tempHeight = scale*view.h;
    int tempWidth =  scale*view.w;

    //a new rectangle based on the size of what's being displayed
    SDL_Rect dest =  {x, y, tempWidth, tempHeight};

    //weird angle stuff, has to do with how SDL interprets angles
    double angle = 0;
    if(xv != 0) {
        //if it's moving on the x axis
        angle = atan(yv/xv) * 180 / 3.141569 + 90;
        if(xv < 0) angle += 180;
    }
    else {
        if(yv <= 0) {
            //if it's moving perfectly straight up, or stopped
            angle = 0;
        }
        else {
            //if it's moving perfectly straight down
            angle = 180;
        }
    }

    SDL_Point center({width/2,height/2});
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    //copy this to what's going to be rendered
    SDL_RenderCopyEx(renderer, texture, &view, &dest, angle, &center, flip);
}

void Image::draw(int x, int y, float scale) const {
    //draw the current frame

    //get the x and y positions of the current view
    x -= Viewport::getInstance().getX();
    y -= Viewport::getInstance().getY();

    //scale the height to the current setting
    int tempHeight = scale*view.h;
    int tempWidth =  scale*view.w;

    //a new rectangle based on the size of what's being displayed
    SDL_Rect dest =  {x, y, tempWidth, tempHeight};

    //copy this to what's going to be rendered
    SDL_RenderCopy(renderer, texture, &view, &dest);
}

void Image::draw(int sx, int sy, int dx, int dy) const {
    //the view: top left x, top left y, width, height
    SDL_Rect src = { sx, sy, view.w, view.h };

    //the texture's destination within the view: top left x, top left y,
    //width, height
    SDL_Rect dst = { dx, dy, getWidth(), getHeight() };

    SDL_RenderCopy(renderer, texture, &src, &dst);
}

Image* Image::crop(SDL_Rect sub)const{
    if(sub.x+sub.w > view.w
    || sub.y+sub.h > view.h){
        std::cerr << "Attempted to crop image with invalid geometry."
            << std::endl
            << "(0,0 + "<<view.w << "x"<<view.h<<")"
            << " --> "
            << "("<<sub.x<<","<<sub.y<<" + "<<sub.w << "x"<<sub.h<<")"
            << std::endl;
        return nullptr;
  }

    Image* cloned = new Image(*this);
    cloned->view = sub;

    return cloned;
}
