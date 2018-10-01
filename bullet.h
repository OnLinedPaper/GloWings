#include <iostream>
#include "rotateSprite.h"
#include "gamedata.h"

class Bullet : public rotateSprite {
public:
    explicit Bullet(const string& name) :
        rotateSprite(name),
        distance(0),
        maxDistance(Gamedata::getInstance().getXmlInt(name+"Attrs/distance")),
        accel(Gamedata::getInstance().
            getXmlFloat(name+"Attrs/movementTraits/acceleration")),
        elapsedTicks(0),
        maxTicks(Gamedata::getInstance().getXmlInt(name+"Attrs/maxTicks")),
        tooLong(false)
        { }

    virtual void update(Uint32 ticks);

    bool goneTooLong() const { return tooLong; }

    void reset() {
        elapsedTicks=0;
        distance = 0;
        tooLong = false;
        resetHealth();
    }

    Bullet(const Bullet& b) = delete;
    Bullet &operator=(const Bullet&) = delete;

private:
    float distance;
    float maxDistance;
    float accel;
    int elapsedTicks;
    int maxTicks;
    bool tooLong;
};
