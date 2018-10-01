#ifndef ROTATESPRITE__H
#define ROTATESPRITE__H

#include "multisprite.h"

class rotateSprite : public MultiSprite {
public:
    rotateSprite(const std::string&);
    rotateSprite(const rotateSprite&) = delete;
    rotateSprite& operator=(const rotateSprite&) = delete;

    virtual void draw() const;
    virtual void update(Uint32 ticks);

protected:
};

#endif
