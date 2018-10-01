#include <cmath>

#include "seekerSprite.h"
#include "gamedata.h"

seekerSprite::seekerSprite(
    const std::string& name, const Vector2f& spawnPoint) :
        dartSprite(name),
        trueVelocity(0,0),
        maxSpeed(Gamedata::getInstance().
            getXmlInt(name+"Attrs/movementTraits/maxSpeed")),
        acceleration(Gamedata::getInstance().
            getXmlInt(name+"Attrs/movementTraits/acceleration")),
        oscillationPeriod(Gamedata::getInstance().
            getXmlInt(name+"Attrs/movementTraits/oscillationPeriod")),
        oscillationIntensity(Gamedata::getInstance().
            getXmlInt(name+"Attrs/movementTraits/oscillationIntensity")),
        sineTicks((rand() % 90) * 4)
{
    setPosition(spawnPoint); //spawn it beneath the behemoth

    //random velocity
    setVelocity(Vector2f(
        std::rand()%(maxSpeed)-maxSpeed/3,
        std::rand()%(maxSpeed)-maxSpeed/3
    ));

    //true velocity holds the "straight line" velocity
    trueVelocity = getVelocity();
}

const Vector2f seekerSprite::seekPlayer() const {
    //divebomb the player!

    Vector2f heading(0,0);
    Vector2f dist = getPrecisePlayerPos() - getPrecisePos();

    if(getPlayerIsAlive()) {
        //player is alive, seek

        //adjust heading to intercept
        heading += dist;
        //normalize heading to intercept
        heading = heading.normalizeSafe() * acceleration;
    }

    if(getPlayerIsOverheated()) {
        //player is vulnerable! DIVEBOMB

        heading *= 2;
    }

    return(heading);
}

void seekerSprite::incrementSineTicks(Uint32 ticks) {
    sineTicks += ticks;
    sineTicks = sineTicks % 360;
}

const Vector2f seekerSprite::waveMotion(const Vector2f& currVel) const {
    //wave motion movement

    Vector2f heading = currVel;

    float DEG_TO_RAD = (3.141569/180);

    float sinVal = sin(sineTicks*DEG_TO_RAD);
    Vector2f normalizedVel = currVel.normalizeSafe();

    float xVal = currVel[0] + (sinVal * oscillationIntensity * normalizedVel[1]);
    float yVal = currVel[1] + (-sinVal * oscillationIntensity * normalizedVel[0]);

    heading[0] = xVal;
    heading[1] = yVal;

    return(heading);
}

void seekerSprite::steer(const Vector2f heading) {
    //track the "true" straight line velocity, but also
    //add some undulation with the waveMotion function

    Vector2f speed = trueVelocity;

    trueVelocity = speed + heading;
    speed = trueVelocity;

    if(speed.magnitude() > maxSpeed) {
        trueVelocity = speed.normalizeSafe() * maxSpeed;
    }

    if(!getPlayerIsOverheated() && getPlayerIsAlive()) {
        Vector2f newVel = waveMotion(trueVelocity);
        setVelocity(newVel);
    }
    else{
        setVelocity(trueVelocity);
    }

}

void seekerSprite::draw() const {
    dartSprite::draw();
}

void seekerSprite::update(Uint32 ticks) {
    incrementSineTicks(oscillationPeriod);
    steer(seekPlayer());
    rotateSprite::update(ticks); //don't use dartsprite's update
}
