#ifndef PLAYER__H
#define PLAYER__H

#include "rotateSprite.h"
#include "bullet.h"
#include <memory>

#define RAD_TO_DEG 180/3.141569
#define DEG_TO_RAD 3.141569/180

struct bulletStruct {
    bulletStruct(std::string b, std::list<Bullet *> bs,
        std::list<Bullet *> fbs, float bsp, float in, float tslf, int hrs) :
        bulletName(b),
        bullets(bs),
        freeBullets(fbs),
        bulletSpeed(bsp),
        interval(in),
        trueInterval(interval),
        timeSinceLastFrame(tslf),
        heatRateShoot(hrs)
    { };

    bulletStruct(const bulletStruct* b) :
        bulletName(b->bulletName),
        bullets(b->bullets),
        freeBullets(b->freeBullets),
        bulletSpeed(b->bulletSpeed),
        interval(b->interval),
        trueInterval(b->trueInterval),
        timeSinceLastFrame(b->timeSinceLastFrame),
        heatRateShoot(b->heatRateShoot)
    { };

    ~bulletStruct() {
        for(Bullet *b : bullets) {
            delete b;
        }
        for(Bullet *b : freeBullets) {
            delete b;
        }
    };

    std::string bulletName;
    std::list<Bullet *> bullets;
    std::list<Bullet *> freeBullets;
    float bulletSpeed;
    float interval;
    float trueInterval;
    float timeSinceLastFrame;
    int heatRateShoot;
};

class Player : public rotateSprite {
public:
    Player(const std::string&);
    Player(const Player&) = delete;
    ~Player();

    virtual void draw() const;

    void attach(Drawable *o) {observers.push_back(o);}
    void detach(Drawable *o);
    virtual void update(Uint32 ticks);

    Player& operator=(const Player&) = delete;

    void right();
    void left();
    void up();
    void down();
    void damp();

    void setCursorPos(const Vector2f& c) { cursorPos = c; }
    std::list<Bullet*> *getPrimaryBullets(){return &(primary->bullets);}
    std::list<Bullet*> *getSecondaryBullets(){return &(secondary->bullets);}

    int getPrimaryBulletSize() const {return primary->bullets.size();}
    int getPrimaryFreeSize() const {return primary->freeBullets.size();}
    int getSecondaryBulletSize() const {return secondary->bullets.size();}
    int getSecondaryFreeSize() const {return secondary->freeBullets.size();}

    bool shootPrimary() {return shoot(primary);}
    bool shootSecondary() {return shoot(secondary);}

    void respawn() {
        resetHealth();
        heat = 0;
        isDead = false;
        respawnTimer = 0;
        invincibilityTimer = 0;
        setVelocity(Vector2f(0,0));
        coolTimer = 0;
    };

    float getHeat() const { return heat; }
    float getMaxHeat() const { return maxHeat; }
    bool isOverheated() const { return overheated; }
    void forceOverheat() { overheated = true; }
    void heatUpShoot(int);
    void coolDown();

    void setIsDead(const bool b) { isDead = b; }
    bool getIsDead() const { return isDead; }

    bool primedToExplode() {
        if(explodeMe) {
            explodeMe = false;
            return true;
        }
        return false;
    }

    bool isCollidable() const { return(!isDead && !isInvincible);}

    void makeInvincible() { isInvincible = true; invincibilityTimer = 0; }
    void keepInvincible() {
        isInvincible = true;
        if (invincibilityTimer > (invincibilityFrames / 2)) {
            invincibilityTimer = 0;
        }
    }
    void instantCool() { heat = 0; }
    void addCoolTicks(int ticks) { coolTimer += ticks; }
    bool hasCoolTicks() const { return coolTimer > 0; }
    float getCoolTicks() const { return coolTimer; }

    void boostFireRate() {
        //double fire rate
        primary->interval = primary->trueInterval / 2;
        secondary->interval = secondary->trueInterval / 2;
    }

    void dropFireRate() {
        //restore fire rate
        primary->interval = primary->trueInterval;
        secondary->interval = secondary->trueInterval;
    }
    void dropInvincible() {
        //drop invincibility
        isInvincible = false;
        invincibilityTimer = 0;
    }

protected:
    bool shoot(bulletStruct *);
    float getAngle();

private:

    bulletStruct *primary;
    std::string primaryBulletName;

    bulletStruct *secondary;
    std::string secondaryBulletName;

    std::list<Drawable*> observers;

    float maxSpeed;
    float acceleration;
    float damping;
    float movementPenalty;

    float spread;

    int heat;
    int maxHeat;
    int coolRate;
    int coolRatePenalty;
    int coolTimer;

    bool overheated;

    int respawnTime;
    int respawnTimer;
    bool isInvincible;
    int invincibilityFrames;
    int invincibilityTimer;

    bool isDead;
    bool explodeMe;

    Vector2f accelVec;
    Vector2f cursorPos;
};

#endif
