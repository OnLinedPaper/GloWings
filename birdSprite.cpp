#include <random>
#include "birdSprite.h"
#include "rotateSprite.h"
#include "gamedata.h"
#include "renderContext.h"

//initialize the flock
std::vector<birdSprite *> birdSprite::flock = std::vector<birdSprite *>();

//give the birds their common traits
int birdSprite::neighbor =
    Gamedata::getInstance().getXmlInt("birdAttrs/neighborRadius");
int birdSprite::panic =
    Gamedata::getInstance().getXmlInt("birdAttrs/panicRadius");

int birdSprite::maxSpeed =
    Gamedata::getInstance().getXmlInt("birdAttrs/maxSpeed");
float birdSprite::damping =
    Gamedata::getInstance().getXmlFloat("birdAttrs/damping");

float birdSprite::coherence =
    Gamedata::getInstance().getXmlFloat("birdAttrs/coherence");
float birdSprite::alignment =
    Gamedata::getInstance().getXmlFloat("birdAttrs/alignment");
float birdSprite::separation =
    Gamedata::getInstance().getXmlFloat("birdAttrs/separation");
float birdSprite::avoidance =
    Gamedata::getInstance().getXmlFloat("birdAttrs/avoidance");

int birdSprite::maxX = Gamedata::getInstance().getXmlInt("world/width");
int birdSprite::maxY = Gamedata::getInstance().getXmlInt("world/height");

//=============================================================================

birdSprite::birdSprite(const std::string &name) :
    rotateSprite(name)
{
    //4 bird spawn points, in the "middle" corners of the world
    setX(Gamedata::getInstance().getXmlInt("world/width")
        * ((std::rand() % 2 == 0) ? 1 : 3) / 4);
    setY(Gamedata::getInstance().getXmlInt("world/height")
        * ((std::rand() % 2 == 0) ? 1 : 3) / 4);


    //give bird random velocity
    int xSpeed = Gamedata::getInstance().getXmlInt("bird/speedX");
    int ySpeed = Gamedata::getInstance().getXmlInt("bird/speedY");
    setVelocityX(std::rand() % (xSpeed*2) - xSpeed);
    setVelocityY(std::rand() % (ySpeed*2) - ySpeed);

    flock.push_back(this);
}

void birdSprite::draw() const {
  	images[currentFrame]->drawRotate(getX(), getY(), getScale(),
        getScaledWidth(), getScaledHeight(), getVelocityX(), getVelocityY());
}

void birdSprite::update(Uint32 ticks) {
    steer(cohere() + separate() + align() + avoid());
	rotateSprite::update(ticks);
}

//=============================================================================

Vector2f birdSprite::cohere() const {
    //fly towards nearby birds

    Vector2f heading(0,0);
    int count = 0;

    for(birdSprite const *b : flock) {
        //check each bird's position
        Vector2f dist = getPosition() - b->getPosition();

        if(fabs(dist.magnitude()) < neighbor) {
            //a friend, flock up!

            heading -= dist;
            count++;
        }
    }

    if(count > 0) {
        //we found at least one friend, adjust heading to get average
        //and then weight it versus the coherence factor
        heading /= count;

        heading /= coherence;
    }

    return(heading);
}

Vector2f birdSprite::align() const {
    //fly the same direction as nearby birds

    Vector2f heading(0,0);
    int count = 0;

    for(birdSprite const *b : flock) {
        //check each bird's position
        Vector2f dist = getPosition() - b->getPosition();

        if(fabs(dist.magnitude()) < neighbor) {
            //a friend, fly with them!

            Vector2f adjust = b->getVelocity();
            heading += adjust;
            count++;
        }
    }

    if(count > 0) {
        //at least one bird was close enough. adjust heading for alignment.
        heading = heading.normalizeSafe();
        heading /= alignment;
    }

    return(heading);
}

Vector2f birdSprite::separate() const {
    //fly away from nearby birds if they're too close

    Vector2f heading(0,0);
    int count = 0;

    for(birdSprite const *b : flock) {
        //check each bird's position
        Vector2f dist = getPosition() - b->getPosition();

        if(fabs(dist.magnitude()) < panic) {
            //this bird is too close!
            if(fabs(dist.magnitude()) < .01) {
                //WAY too close, the bird may not properly avoid at this
                //distance.
                //add some randomness to get them properly separated.
                heading += Vector2f(
                    std::rand() % maxSpeed + 1,
                    std::rand() % maxSpeed + 1
                ).normalizeSafe();
            }

            heading += dist;
            count++;
        }
    }

    if(count > 0) {
        //at least one bird was close enough. adjust heading for separation.
        heading /= count;

        heading /= separation;
    }

    return(heading);
}

Vector2f birdSprite::avoid() const {
    //avoid walls

    int xPos = getX();
    int yPos = getY();

    bool altered = false;

    Vector2f heading(0,0);

    if(xPos < panic) {
        //too close to the left wall
        Vector2f alter((maxSpeed-xPos), 0);
        heading += alter;
        altered = true;
    }
    if(yPos < panic) {
        //too close to the top wall
        Vector2f alter(0, maxSpeed-yPos);
        heading += alter;
        altered = true;
    }
    if(maxX - (xPos + getScaledWidth()) < panic) {
        //too close to the right wall
        Vector2f alter(-maxSpeed+(maxX - xPos), 0);
        heading += alter;
        altered = true;
    }
    if(maxY - (yPos + getScaledHeight()) < panic) {
        //too close to the too close to the bottom wall
        Vector2f alter(0, -maxSpeed+(maxY - yPos));
        heading += alter;
        altered = true;
    }

    if(altered) {
        heading = heading.normalizeSafe();
        heading *= maxSpeed;
        heading /= avoidance;
    }

    return(heading);
}

void birdSprite::steer(const Vector2f heading) {
    Vector2f speed = getVelocity();

    setVelocity(speed + heading);
    speed = getVelocity();

    if(speed.magnitude() > maxSpeed) {
        //bird's going too fast, damp its speed
        setVelocity(speed / damping);
    }
}
