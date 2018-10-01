#ifndef VIEWPORT__H
#define VIEWPORT__H

#include "drawable.h"
#include "gamedata.h"

//a view of the current object being tracked
//also an overlaid texture for printing information to the screen

class Viewport {
public:
    static Viewport& getInstance();     //meyers singleton
    void draw() const;                  //say what's being tracked by adding a
                                        //texture on top
    void draw(const std::string&, int, int) const;
    void draw
        (const std::string& msg, int x, int y, SDL_Color c) const;
    void update();

    Vector2f getPosition() const { return position; }
    float getX() const  { return position[0]; }
    void  setX(float x) { position[0] = x; }
    float getY() const  { return position[1]; }
    void  setY(float y) { position[1] = y; }

    void setObjectToTrack(const Drawable *obj);
    const Drawable* getObjectToTrack() const { return objectToTrack; }

private:
    const Gamedata& gdata;
    Vector2f position;
    int worldWidth;
    int worldHeight;
    int viewWidth;
    int viewHeight;
    int objWidth;
    int objHeight;

    const Drawable *objectToTrack;

    Viewport();
    Viewport(const Viewport&);
    Viewport& operator=(const Viewport&);
};
#endif
