#ifndef BEHEMOTHSPRITE__H
#define BEHEMOTHSPRITE__H

#include "multisprite.h"

class behemothSprite : public MultiSprite {
public:
    behemothSprite(const std::string&);
    behemothSprite(const behemothSprite&) = delete;
    behemothSprite& operator=(const behemothSprite&) = delete;

    ~behemothSprite() {
        destroyShields();
    }

    virtual void draw() const;
    virtual void update(Uint32 ticks);

    int getShieldCount() const { return shieldCount; }
    int getMaxShieldCount() const { return maxShieldCount; }
    bool checkShieldSpawn() {
        if(--timeToNextShield <= 0) {
            timeToNextShield = shieldSpawnDelay;
            return true;
        }
        else {
            return false;
        }
    }

    int getSeekersSpawned() const { return seekersSpawned; }
    void incrementSeekersSpawned() { seekersSpawned++; }
    int getSeekerCount() const { return seekerCount; }
    void augmentSeekerCount(int wave) { seekerCount *= wave; }

    int getMitesSpawned() const { return mitesSpawned; }
    void incrementMitesSpawned() { mitesSpawned++; }
    int getMiteCount() const { return miteCount; }
    void augmentMiteCount(int wave) { miteCount *= wave; }

    void attachShield(Drawable *o) {shields.push_back(o); shieldCount++;}
    void detachShield(Drawable *o);
    void destroyShields();


protected:

    std::list<Drawable*> children;
    std::list<Drawable*> shields;

    const Vector2f moveAboutPlayer() const;
    void steer(const Vector2f heading);

    int panicRadius;

    int maxSpeed;
    int acceleration;
    int maxShieldCount;
    int shieldCount;
    int shieldSpawnDelay;
    int timeToNextShield;
    int seekerCount;
    int seekersSpawned;
    int miteCount;
    int mitesSpawned;
};

#endif
