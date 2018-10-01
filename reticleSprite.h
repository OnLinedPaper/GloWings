#ifndef RETICLESPRITE__H
#define RETICLESPRITE__H

#include <list>
#include "multisprite.h"
#include "player.h"

class ReticleSprite : public MultiSprite {
public:
    ReticleSprite(const std::string&);
    ReticleSprite(const ReticleSprite&) = delete;
    ReticleSprite& operator=(const ReticleSprite&) = delete;

    virtual void draw() const;
    virtual void update(Uint32 ticks);

    void attach( Player *p ) { observers.push_back(p); }
    void detach( Player *p );

protected:

    std::list<Player*> observers;

    void notify();
};

#endif
