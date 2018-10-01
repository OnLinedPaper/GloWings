#include "pickupSprite.h"
#include "gamedata.h"
#include "renderContext.h"

pickupSprite::pickupSprite( const std::string &name, const Vector2f &pos) :
    MultiSprite(name),
    timeToLive(65535),
    selfDestructed(false)
{
    //spawn it at spawn point
    setPosition(pos);

    //it can't move
    setVelocity(Vector2f(0,0));
}

void pickupSprite::draw() const {
    MultiSprite::draw();
}

void pickupSprite::update(Uint32 ticks) {
    MultiSprite::update(ticks);

    timeToLive--;
    if(timeToLive <= 0){
        //pickup expired.
        selfDestructed = true;
        killHealth();
    }
}
