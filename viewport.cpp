#include <sstream>
#include "viewport.h"
#include "ioMod.h"

Viewport& Viewport::getInstance() {
    static Viewport viewport;           //meyers singleton, will only be
                                        //declared once

    return viewport;                    //return the instance
}

Viewport::Viewport() :
    //create viewport

    //instantiate an instance
    gdata(Gamedata::getInstance()),

    //set position to 0,0
    position(0, 0),

    //width and height data are used to prevent the view from stretching
    //outside the world boundaries

    //set world width and height from xml data
    worldWidth(gdata.getXmlInt("world/width")),
    worldHeight(gdata.getXmlInt("world/height")),

    //set the view width and height from xml data
    viewWidth(gdata.getXmlInt("view/width")),
    viewHeight(gdata.getXmlInt("view/height")),

    //width and height of the currently-tracked object
    objWidth(0), objHeight(0),

    //not tracking anything now
    objectToTrack(NULL)
{}

void Viewport::setObjectToTrack(const Drawable *obj) {
    //set the current tracked object

    //set the object
    objectToTrack = obj;

    //set the object width and height
    objWidth = objectToTrack->getScaledWidth();
    objHeight = objectToTrack->getScaledHeight();
}

void Viewport::draw() const {
    //draw something into the viewport

    int x, y = 0;
    SDL_GetMouseState(&x, &y);

    IOmod::getInstance().
        writeText("Nick Reed", 30, (viewHeight - 60),
            SDL_Color({160, 160, 255, 0}));
}

void Viewport::draw(const std::string& msg, int x, int y) const {
    //broaden ability by allowing things to appear anywhere
    IOmod::getInstance().
        writeText(msg.c_str(), x, y);
}

void Viewport::draw(const std::string& msg, int x, int y, SDL_Color c) const {
    //draw text in any color
    IOmod::getInstance().
        writeText(msg.c_str(), x, y, c);
}

void Viewport::update() {
    //update the view for the next time it's drawn

    //get the x and y position of the object to track
    const float x = objectToTrack->getX();
    const float y = objectToTrack->getY();

    //set the x and y position based on the position of the tracked object
    position[0] = (x + objWidth/2) - viewWidth/2;
    position[1] = (y + objHeight/2) - viewHeight/2;

    //if the object hits the end of the screen, stop before overflowing out
    //of the world
    if (position[0] < 0) position[0] = 0;
    if (position[1] < 0) position[1] = 0;
    if (position[0] > (worldWidth - viewWidth)) {
        position[0] = worldWidth-viewWidth;
    }
    if (position[1] > (worldHeight - viewHeight)) {
        position[1] = worldHeight-viewHeight;
    }
}
