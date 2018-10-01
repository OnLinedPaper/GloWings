#ifndef PICKUPSPRITE__H
#define PICKUPSPRITE__H

#include "multisprite.h"

class pickupSprite : public MultiSprite {
public:
    pickupSprite(const std::string&, const Vector2f&);
    pickupSprite(const pickupSprite&) = delete;
    pickupSprite& operator=(const pickupSprite&) = delete;

    virtual void draw() const;
    virtual void update(Uint32 ticks);

    bool getSelfDestructed() const { return selfDestructed; }

protected:

    int timeToLive;
    bool selfDestructed;
};

#endif
