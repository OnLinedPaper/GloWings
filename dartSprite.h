#ifndef DARTSPRITE__H
#define DARTSPRITE__H

#include "rotateSprite.h"

class dartSprite : public rotateSprite {
public:
    dartSprite(const std::string&);
    dartSprite(const dartSprite&) = delete;
    dartSprite& operator=(const dartSprite&) = delete;

    virtual void draw() const;
    virtual void update(Uint32 ticks);

protected:

    const Vector2f seekPlayer() const;
    void steer(const Vector2f heading);

    int seekRadius;

    int maxSpeed;
    int acceleration;
};

#endif
