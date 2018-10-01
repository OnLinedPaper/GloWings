#include <random>
#include <math.h>
#include "shieldSprite.h"
#include "gamedata.h"

//initialize the shields
std::vector<shieldSprite *> shieldSprite::shields =
    std::vector<shieldSprite *>();

shieldSprite::shieldSprite(const std::string &name, const Vector2f pos) :
    rotateSprite(name),
    orbitRadius(Gamedata::getInstance().
        getXmlInt(name+"Attrs/movementTraits/orbitRadius")),
    panicRadius(Gamedata::getInstance().
        getXmlInt(name+"Attrs/movementTraits/panicRadius")),
    maxSpeed(Gamedata::getInstance().
        getXmlInt(name+"Attrs/movementTraits/maxSpeed")),
    randomInterval(std::rand() % 360),
    protectVal(Gamedata::getInstance().
        getXmlFloat(name+"Attrs/movementTraits/protection")),
    avoidVal(Gamedata::getInstance().
        getXmlFloat(name+"Attrs/movementTraits/avoidance")),
    jitterVal(Gamedata::getInstance().
        getXmlFloat(name+"Attrs/movementTraits/jitter")),
    spacingVal(Gamedata::getInstance().
        getXmlFloat(name+"Attrs/movementTraits/spacing")),
    intercessionVal(Gamedata::getInstance().
        getXmlFloat(name+"Attrs/movementTraits/intercession")),
    protectThisIsAlive(true),
    protectThis(pos)
{
    setForcesOverheat(true);

    //position it right on top of the thing it's defending
    setPosition(protectThis);

    Vector2f initVel(
        std::rand() / 2 - std::rand(),
        std::rand() / 2 - std::rand()
    );

    initVel = initVel.normalizeSafe();

    initVel *= maxSpeed;

    //give it a random initial direction
    setVelocity(initVel);

    shields.push_back(this);
}

Vector2f shieldSprite::protect() const {
    Vector2f heading(0,0);

    Vector2f dist = getPrecisePos() - protectThis;

    heading -= dist;

    heading = heading.normalizeSafe();

    return(heading * protectVal);
}

Vector2f shieldSprite::intercede() const {
    Vector2f heading(0,0);

    Vector2f distToPlayer = getPrecisePos() - getPrecisePlayerPos();

    if(!(getPlayerIsAlive()) ||
        distToPlayer.magnitude() > orbitRadius * 2) {return Vector2f(0,0);}

    Vector2f distToBehemoth = getPrecisePos() - protectThis;
    heading -= distToPlayer;
    if(distToBehemoth.magnitude() > orbitRadius) {
        return Vector2f(0,0);
    }
    else {
        heading = heading.normalizeSafe();
        return(heading * intercessionVal);
    }
}

Vector2f shieldSprite::space() const {
    Vector2f heading(0,0);

    Vector2f dist = getPrecisePos() - protectThis;

    if(dist.magnitude() < panicRadius) {
        heading += dist;
        heading = heading.normalizeSafe();
    }

    return(heading * spacingVal);
}

Vector2f shieldSprite::jitter() {
    randomInterval = fmod((randomInterval + .1), 360);
    return (Vector2f(
        sin(randomInterval),
        cos(randomInterval)
    ).normalizeSafe() * jitterVal);
}

Vector2f shieldSprite::avoid() const {
    Vector2f heading(0,0);
    int count = 0;

    for(shieldSprite const *s : shields) {
        Vector2f dist = getPosition() - s->getPosition();

        if(fabs(dist.magnitude()) < panicRadius) {
            heading += dist;
            count++;
        }
    }

    if(count > 0) {
        heading /= count;
    }
    heading = heading.normalizeSafe();

    return(heading * avoidVal);
}

void shieldSprite::steer(const Vector2f h) {
    Vector2f speed = getVelocity();
    Vector2f heading = h;

    heading = h.normalizeSafe() * maxSpeed;

    setVelocity(speed + heading);
    speed = getVelocity();

    if(speed.magnitude() > maxSpeed) {
        //going to fast, set it to max speed
        speed = speed.normalizeSafe();
        speed *= maxSpeed;
        setVelocity(speed);
    }
}

void shieldSprite::draw() const {
    Vector2f diff = getPrecisePos() - protectThis;
  	images[currentFrame]->drawRotate(getX(), getY(), getScale(),
        getScaledWidth(), getScaledHeight(), diff[0], diff[1]);
}

void shieldSprite::update(Uint32 ticks) {
    //if behemoth is dead, kill the shield
    if(!protectThisIsAlive) { killHealth(); }
    steer(protect() + space() + intercede() + jitter() + avoid());
    rotateSprite::update(ticks);
}
