#ifndef SHIELDSPRITE__H
#define SHIELDSPRITE__H

#include "rotateSprite.h"

//protects its target from any distance
//tries not to get too close to target

class shieldSprite : public rotateSprite {
public:
    shieldSprite(const std::string &, const Vector2f);

    ~shieldSprite() {
        //iterate through the shields and delete the removed shield
        std::vector<shieldSprite *>::iterator it = shields.begin();
        while(it != shields.end()) {
            if(*it == this) {
                //found the shield, erase it
                shields.erase(it);
                return;
            }
            else {
                //keep looking
                ++it;
            }
        }
    }

    virtual void draw() const;
    virtual void update(Uint32 ticks);

    void setProtectThis(Vector2f p) { protectThis = p; }
    void setProtectThisIsAlive(bool b) { protectThisIsAlive = b; }


protected:

    int orbitRadius, panicRadius, maxSpeed;
    float randomInterval,
        protectVal, avoidVal, jitterVal, spacingVal, intercessionVal;
    bool protectThisIsAlive;

    static std::vector<shieldSprite *> shields;

    Vector2f protect() const;
    Vector2f space() const;
    Vector2f intercede() const;
    Vector2f jitter();
    Vector2f avoid() const;
    void steer(const Vector2f heading);

    Vector2f protectThis;

private:
    shieldSprite(const shieldSprite&);
    shieldSprite& operator=(const shieldSprite&);
};



#endif
