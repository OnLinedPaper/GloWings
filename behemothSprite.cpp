#include "behemothSprite.h"
#include "gamedata.h"
#include "renderContext.h"
#include "rotateSprite.h"
#include "shieldSprite.h"

behemothSprite::behemothSprite(const std::string& name) :
    MultiSprite(name),
    children(),
    shields(),
    panicRadius(Gamedata::getInstance().
            getXmlInt("behemothAttrs/observerTraits/panicRadius")),
    maxSpeed(Gamedata::getInstance().
            getXmlInt("behemothAttrs/movementTraits/maxSpeed")),
    acceleration(Gamedata::getInstance().
            getXmlInt("behemothAttrs/movementTraits/acceleration")),
    maxShieldCount(Gamedata::getInstance().
        getXmlInt("behemothAttrs/childTraits/shieldCount")),
    shieldCount(0),
    shieldSpawnDelay(Gamedata::getInstance().
        getXmlInt("behemothAttrs/childTraits/shieldSpawnDelay")),
    timeToNextShield(0),
    seekerCount(Gamedata::getInstance().
        getXmlInt("behemothAttrs/childTraits/seekerCount")),
    seekersSpawned(0),
    miteCount(Gamedata::getInstance().
        getXmlInt("behemothAttrs/childTraits/miteCount")),
    mitesSpawned(0)
{
    //4 random possible spawn points
    setX(Gamedata::getInstance().getXmlInt("world/width")
        * ((std::rand() % 2 == 0) ? 1 : 3) / 4);
    setY(Gamedata::getInstance().getXmlInt("world/height")
        * ((std::rand() % 2 == 0) ? 1 : 3) / 4);
}

void behemothSprite::draw() const {
    MultiSprite::draw();
}

void behemothSprite::detachShield(Drawable *o) {
    std::list<Drawable*>::iterator ptr = shields.begin();
    while(ptr != shields.end()) {
        if(*ptr == o) {
            ptr = shields.erase(ptr);
            shieldCount--;
            return;
        }
        ++ptr;
    }
}

void behemothSprite::destroyShields() {
    for(Drawable *s : shields) {
        dynamic_cast<shieldSprite *>(s)->setProtectThisIsAlive(false);
    }
}

const Vector2f behemothSprite::moveAboutPlayer() const {
    //run away from the player

    Vector2f heading(0, 0);
    //distance from the player
    Vector2f dist = getPrecisePlayerPos() - getPrecisePos();

    if(getPlayerIsAlive() &&
        fabs(dist.magnitude()) < panicRadius) {
            //player is too close for comfort

            //adjust heading to flee
            heading -= dist;
            //normalize heading to flee
            heading = heading.normalizeSafe() * acceleration;
    }

    if(getHealth() < ((getMaxHealth() * 3) / 4)) {
        //behemoth enters battle mode at 3/4 health and doubles its speed
        heading *= -2;
    }

    return(heading);
}

void behemothSprite::steer(const Vector2f heading) {
    Vector2f speed = getVelocity();

    setVelocity(speed + heading);
    speed = getVelocity();

    if(speed.magnitude() > maxSpeed) {
        setVelocity(speed.normalizeSafe() * maxSpeed);
    }
}

void behemothSprite::update(Uint32 ticks) {
    steer(moveAboutPlayer());

    MultiSprite::update(ticks);

    std::list<Drawable*>::iterator ptr = shields.begin();
    while(ptr != shields.end()) {
        (dynamic_cast<shieldSprite *>(*ptr))->setProtectThis(Vector2f(
            getX() + (getScaledWidth() /2),
            getY() + (getScaledHeight() /2)
        ));
        ++ptr;
    }
}
