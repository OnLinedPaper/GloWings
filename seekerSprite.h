#ifndef SEEKERSPRITE__H
#define SEEKERSPRITE__H

#include "dartSprite.h"

class seekerSprite : public dartSprite {
    //a derivative of the dartsprite - a seeker can track the player from
    //anywhere, and moves in a sinusoidal fashion.

public:
    seekerSprite(const std::string&, const Vector2f&);
    seekerSprite(const seekerSprite&) = delete;
    seekerSprite& operator=(const seekerSprite&) = delete;

    virtual void draw() const;
    virtual void update(Uint32 ticks);

    void incrementSineTicks(Uint32 ticks);

protected:

    //no seekRadius, since it can see the player everywhere

    const Vector2f seekPlayer() const; //use the dartsprite's steer
    const Vector2f waveMotion(const Vector2f&) const; //sinusoidal motion
    void steer(const Vector2f heading);

    Vector2f trueVelocity;

    int maxSpeed;
    int acceleration;
    int oscillationPeriod;
    float oscillationIntensity;
    int sineTicks;
};

#endif
