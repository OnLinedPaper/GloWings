#ifndef DRAWABLE__H
#define DRAWABLE__H

#include <SDL.h>
#include <iostream>
#include <string>
#include <list>
#include "vector2f.h"
#include "image.h"
#include "gamedata.h"

// Drawable is an Abstract Base Class (ABC) that specifies the methods
// that derived classes may or must have.
class Drawable {
public:
    Drawable() = delete;
    Drawable(const std::string& n, const Vector2f& pos, const Vector2f& vel):
    name(n), position(pos), velocity(vel), playerPos(0,0),
    precisePlayerPos(0,0), playerIsAlive(true), playerIsOverheated(false),
    scale(1.0),
	health(Gamedata::getInstance().
        getXmlInt(name+"Attrs/healthTraits/health")),
	maxHealth(Gamedata::getInstance().
        getXmlInt(name+"Attrs/healthTraits/health")),
	damage(Gamedata::getInstance().
        getXmlInt(name+"Attrs/healthTraits/damage")),
    score(Gamedata::getInstance().
        getXmlInt(name+"Attrs/score")),
    trueHealth(health),
    trueMaxHealth(maxHealth),
    trueDamage(damage),
    trueScore(score),
    forcesOverheat(false),
    isBehemothEntity(false),
    dropsHealth(false),
    dropsCooldown(false),
    isHealth(false),
    isCooldown(false),
    lastHitByBullet(false)
    { }

    Drawable(const Drawable& s) :
    name(s.name),
    position(s.position),
    velocity(s.velocity),
    playerPos(s.playerPos),
    precisePlayerPos(s.precisePlayerPos),
    playerIsAlive(s.playerIsAlive),
    playerIsOverheated(s.playerIsOverheated),
    scale(s.scale),
    health(s.health),
    maxHealth(s.maxHealth),
    damage(s.damage),
    score(s.score),
    trueHealth(s.trueHealth),
    trueMaxHealth(s.trueMaxHealth),
    trueDamage(s.trueDamage),
    trueScore(s.trueScore),
    forcesOverheat(s.forcesOverheat),
    isBehemothEntity(s.isBehemothEntity),
    dropsHealth(s.dropsHealth),
    dropsCooldown(s.dropsCooldown),
    isHealth(s.isHealth),
    isCooldown(s.isCooldown),
    lastHitByBullet(s.lastHitByBullet)
    { }

    virtual ~Drawable() {}

    virtual void draw() const = 0;
    virtual void update(Uint32 ticks) = 0;

    float getScale() const  { return scale; }
    void  setScale(float s) { scale = s; }
    virtual int getScaledWidth() const = 0;
    virtual int getScaledHeight() const = 0;
    virtual const SDL_Surface* getSurface() const = 0;

    const std::string& getName() const { return name; }
    void setName(const std::string& n) { name = n;    }

    virtual const Image* getImage() const = 0;

    float getX() const  { return position[0]; }
    void  setX(float x) { position[0] = x;    }

    float getY() const  { return position[1]; }
    void  setY(float y) { position[1] = y;    }

    const Vector2f& getVelocity() const    { return velocity; }
    void  setVelocity(const Vector2f& vel) { velocity = vel;  }
    const Vector2f& getPosition() const    { return position; }
    void  setPosition(const Vector2f& pos) { position = pos;  }

    float getVelocityX() const   { return velocity[0]; }
    void  setVelocityX(float vx) { velocity[0] = vx;   }
    float getVelocityY() const   { return velocity[1]; }
    void  setVelocityY(float vy) { velocity[1] = vy;   }

    void setPlayerPos(const Vector2f &p) {playerPos = p;}
    void setPrecisePlayerPos(const Vector2f &p) {precisePlayerPos = p;}
    const Vector2f getPlayerPos() const {return playerPos;}
    void setPlayerIsAlive(bool b) {playerIsAlive = b;}
    bool getPlayerIsAlive() const {return playerIsAlive;}
    void setPlayerIsOverheated(bool b) {playerIsOverheated = b;}
    bool getPlayerIsOverheated() const {return playerIsOverheated;}

    const Vector2f getPrecisePos() const {
        return Vector2f(
            (position[0] + (getScaledWidth()/2)),
            (position[1] + (getScaledHeight()/2))
        );
    }
    const Vector2f getPrecisePlayerPos() const { return precisePlayerPos; }

    void takeDamage(int const damage)
        { health -= damage; if (health < 0) {health = 0;}}
    float getHealth() const { return health; }
    float getHealthFraction() const { return float(float(health) / float(maxHealth));}
    void killHealth() { health = -1; }
    void resetHealth() { health = maxHealth; }
    void addHealth(int h) {
        health += h;
        if(health > maxHealth) {
            health = maxHealth;
        }
    }
    float getMaxHealth() const { return maxHealth; }
    float getDamage() const { return damage; }
    void scaleToDifficulty(float scale) {
        //make enemies harder as the game goes on,
        //but also worth more
        damage = (trueDamage + trueDamage * scale > trueDamage)
            ? (trueDamage * scale)
            : trueDamage;
        health = (trueHealth + trueHealth * scale > trueHealth)
            ? (trueHealth * scale)
            : trueHealth;
        maxHealth = (trueMaxHealth + trueMaxHealth * scale > trueMaxHealth)
            ? (trueMaxHealth * scale)
            : trueMaxHealth;
        score = (trueScore + trueScore * scale > trueScore)
            ? (trueScore * scale)
            : trueScore;
    }

    void setForcesOverheat(const bool b) { forcesOverheat = b; }
    bool getForcesOverheat() const { return forcesOverheat; }

    void setIsBehemothEntity(const bool b) { isBehemothEntity = b; };
    bool getIsBehemothEntity() const { return isBehemothEntity; };

    void setDropsHealth(const bool b) { dropsHealth = b; };
    bool getDropsHealth() const { return dropsHealth; };

    void setDropsCooldown(const bool b) { dropsCooldown = b;};
    bool getDropsCooldown() const { return dropsCooldown; };

    void setIsHealth(const bool b) { isHealth = b;};
    bool getIsHealth() const { return isHealth; };

    void setIsCooldown(const bool b) { isCooldown = b;};
    bool getIsCooldown() const { return isCooldown; };

    int getScore() const { return score; }

    void setLastHitByBullet(bool b) { lastHitByBullet = b; }
    bool getLastHitByBullet() const { return lastHitByBullet; }
private:
    std::string name;
    Vector2f position;
    Vector2f velocity;
    Vector2f playerPos;
    Vector2f precisePlayerPos;
    bool playerIsAlive;
    bool playerIsOverheated;
    float scale;

    int health;
    int maxHealth;
    int damage;
    int score;

    int trueHealth;
    int trueMaxHealth;
    int trueDamage;
    int trueScore;

    bool forcesOverheat;
    bool isBehemothEntity;
    bool dropsHealth;
    bool dropsCooldown;
    bool isHealth;
    bool isCooldown;

    bool lastHitByBullet;
};
#endif
