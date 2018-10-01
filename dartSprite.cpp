#include "dartSprite.h"
#include "gamedata.h"

dartSprite::dartSprite(const std::string& name) :
    rotateSprite(name),

    seekRadius(Gamedata::getInstance().
            getXmlInt("dartAttrs/observerTraits/seekRadius")),
    maxSpeed(Gamedata::getInstance().
            getXmlInt("dartAttrs/movementTraits/maxSpeed")),
    acceleration(Gamedata::getInstance().
            getXmlInt("dartAttrs/movementTraits/acceleration"))
{
    //4 random possible spawn points
    setX(Gamedata::getInstance().getXmlInt("world/width")
        * ((std::rand() % 2 == 0) ? 1 : 3) / 4);
    setY(Gamedata::getInstance().getXmlInt("world/height")
        * ((std::rand() % 2 == 0) ? 1 : 3) / 4);

    //random velocity
    setVelocity(Vector2f(
        std::rand()%(maxSpeed)-maxSpeed/3,
        std::rand()%(maxSpeed)-maxSpeed/3
    ));
}

void dartSprite::draw() const {
    rotateSprite::draw();
}

const Vector2f dartSprite::seekPlayer() const {
    //divebomb the player!

    Vector2f heading(0, 0); //distance from the player

    Vector2f dist = getPrecisePlayerPos() - getPrecisePos();

    if(getPlayerIsAlive() &&
        fabs(dist.magnitude()) < seekRadius) {
            //player is within seek radius

            //adjust heading to intercept
            heading += dist;
            //normalize heading to intercept
            heading = heading.normalizeSafe() * acceleration;
    }

    return(heading);
}

void dartSprite::steer(const Vector2f heading) {
    Vector2f speed = getVelocity();

    setVelocity(speed + heading);
    speed = getVelocity();

    if(speed.magnitude() > maxSpeed) {
        setVelocity(speed.normalizeSafe() * maxSpeed);
    }
}

void dartSprite::update(Uint32 ticks) {
    steer(seekPlayer());
    rotateSprite::update(ticks);
}
