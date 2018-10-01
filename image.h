#ifndef IMAGE__H
#define IMAGE__H

#include <iostream>
#include <string>
#include <SDL.h>

class Image {
public:
  Image( SDL_Surface*, SDL_Texture* );  //convert
  Image(const Image&);                  //copy
  Image& operator=(const Image&);       //assign

  void draw(int x, int y) const;
  void draw(int x, int y, float scale) const;
  void draw(int sx, int sy, int dx, int dy) const;

  void drawRotate(int x, int y, float scale, int width, int height, int xVel,
      int yVel) const;

  inline int getWidth()  const { return surface->w; }   //made inline for speed
  inline int getHeight() const { return surface->h; }   //made inline for speed
  SDL_Surface* getSurface() const { return surface; }   //return the surface
  Image* crop(SDL_Rect)const;   //reduce image size and return it
private:
  SDL_Renderer * renderer;
  SDL_Surface * surface;
  SDL_Texture * texture;
  SDL_Rect view;                //a rectangle to describe what part of the
                                //screen is being viewed

  Image();                      //no default constructor allowed
};

#endif
