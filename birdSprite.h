#ifndef BIRD__H
#define BIRD__H

#include <vector>
#include "rotateSprite.h"
#include "gamedata.h"

class birdSprite : public rotateSprite {
public:
    birdSprite(const std::string&);
    birdSprite(const birdSprite&) = delete;
    birdSprite& operator=(const birdSprite&) = delete;

    virtual void draw() const;
    virtual void update(Uint32 ticks);

    ~birdSprite() {
        //iterate through the flock and delete the removed bird
        std::vector<birdSprite *>::iterator it = flock.begin();
        while(it != flock.end()) {
            if(*it == this) {
                //found the bird, erase it
                flock.erase(it);
                return;
            }
            else {
                //keep looking
                ++it;
            }
        }
    }

protected:

    //each bird is in here; there is only one flock shared among the birds
    static std::vector<birdSprite *> flock;

    //traits all birds share
    static int neighbor, panic, maxSpeed, maxX, maxY;
    static float damping, coherence, alignment, separation, avoidance;

    Vector2f cohere() const;
    Vector2f align() const;
    Vector2f separate() const;
    Vector2f avoid() const;
    void steer(const Vector2f heading);
};

#endif
