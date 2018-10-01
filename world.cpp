#include <iostream>
#include "world.h"
#include "renderContext.h"
#include "gamedata.h"

World::World(const std::string& name, int fact) :
    image( RenderContext::getInstance()->getImage(name) ),
    factor(fact),
    worldWidth( Gamedata::getInstance().getXmlInt("world/width") ),
    worldHeight( Gamedata::getInstance().getXmlInt("world/height") ),
    windowWidth( Gamedata::getInstance().getXmlInt("view/width") ),
    windowHeight( Gamedata::getInstance().getXmlInt("view/height") ),
    imageWidth( image->getWidth() ),
    imageHeight( image->getHeight() ),
    viewX(0.0), viewY(0.0),
    view(Viewport::getInstance())
{
    if(windowWidth > worldWidth || windowHeight > worldHeight) {
        throw std::string
        ("window dimensions larger than world dimensions.");
    }
}

void World::update() {
    viewX = static_cast<int>(view.getX() / factor) % imageWidth;
    viewY = static_cast<int>(view.getY() / factor) % imageHeight;
}

void World::draw() const {
    /*
    example function call:
        image->draw(0,0,-viewX,-viewY);

    0,0 is the top left corner of the view to draw to, i.e. the top left
    corner of whatever window you have the game open in. if, for example,
    you use 20,20, then the image will be drawn 20 pixels from the top and
    20 pixels from the left.

    -viewX,-viewY are so when your sprite moves forward, the view moves
    backward. if these are positive, the view will move with your sprite,
    shifting the background in the wrong direction and making it look like
    you're going backwards.
    */

    /*
    draw until the start x is greater than the world width
    repeat until the start y is greater than the world height
    if start x plus image width is less than window width, don't draw it
    since it's off screen to the left
    if start y plus image height is less than window height, don't draw it
    since it's off screen above
    */

    float xStart, yStart, xEnd, yEnd = 0;

    xStart = viewX - fmod(viewX, imageWidth);
    yStart = viewY - fmod(viewY, imageHeight);

    xEnd = windowWidth + imageWidth;
    yEnd = windowHeight + imageHeight;

    for(int i=0; imageWidth*i+xStart < xStart + xEnd; i++) {
        for(int j=0; imageHeight*j+yStart < yStart + yEnd; j++) {

            /*
            optimization: don't draw the image unless at least part of it
            is on-screen.
            */

            image->draw(0,0,(imageWidth*i)-viewX,(imageHeight*j)-viewY);
        }
    }
}
