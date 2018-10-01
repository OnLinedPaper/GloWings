#include "player.h"
#include "gamedata.h"
#include <math.h>

Player::~Player() {
    delete primary;
    delete secondary;
}

Player::Player(const std::string &name) :
    rotateSprite(name),
    primary(),
    primaryBulletName(Gamedata::getInstance().
        getXmlStr(name+"Attrs/projectileTraits/primary/bullet")),
    secondary(),
    secondaryBulletName(Gamedata::getInstance().
        getXmlStr(name+"Attrs/projectileTraits/secondary/bullet")),
    observers(),
    maxSpeed(Gamedata::getInstance().
        getXmlFloat(name+"Attrs/movementTraits/MaxSpeed")),
    acceleration(
        Gamedata::getInstance().
            getXmlFloat(name+"Attrs/movementTraits/Acceleration")),
    damping(Gamedata::getInstance().
        getXmlFloat(name+"Attrs/movementTraits/Damping")),
    movementPenalty(Gamedata::getInstance().
        getXmlFloat(name+"Attrs/movementTraits/overheatPenalty")),
    spread(Gamedata::getInstance().
        getXmlFloat(name+"Attrs/projectileTraits/primary/spread")),
    heat(0),
    maxHeat(Gamedata::getInstance().
        getXmlInt(name+"Attrs/heatTraits/maxHeat")),
    coolRate(Gamedata::getInstance().
        getXmlInt(name+"Attrs/heatTraits/coolRate")),
    coolRatePenalty(Gamedata::getInstance().
        getXmlInt(name+"Attrs/heatTraits/overheatPenalty")),
    coolTimer(0),
    overheated(false),
    respawnTime(Gamedata::getInstance().
        getXmlFloat(name+"Attrs/healthTraits/respawnTime")),
    respawnTimer(0),
    isInvincible(false),
    invincibilityFrames(Gamedata::getInstance().
        getXmlFloat(name+"Attrs/healthTraits/invincibilityFrames")),
    invincibilityTimer(0),
    isDead(false),
    explodeMe(false),
    accelVec(0,0),
    cursorPos(0,0)
{
    primary = new bulletStruct(
        primaryBulletName,
        std::list<Bullet *>(),
        std::list<Bullet *>(),
        Gamedata::getInstance().
            getXmlInt(primaryBulletName+"Attrs/movementTraits/speed"),
        Gamedata::getInstance().
            getXmlInt(name+"Attrs/projectileTraits/primary/fireRate"),
        0,
        Gamedata::getInstance().
            getXmlInt(name+"Attrs/projectileTraits/primary/heatRateShoot")
    );

    secondary = new bulletStruct(
        secondaryBulletName,
        std::list<Bullet *>(),
        std::list<Bullet *>(),
        Gamedata::getInstance().
            getXmlInt(secondaryBulletName+"Attrs/movementTraits/speed"),
        Gamedata::getInstance().
            getXmlInt(name+"Attrs/projectileTraits/secondary/fireRate"),
        0,
        Gamedata::getInstance().
            getXmlInt(name+"Attrs/projectileTraits/secondary/heatRateShoot")
    );

    //spawn player in center of the world
    setPosition(Vector2f(
        Gamedata::getInstance().getXmlInt("world/width") / 2,
        Gamedata::getInstance().getXmlInt("world/height") / 2
    ));

}

void Player::detach(Drawable* o) {
    std::list<Drawable*>::iterator ptr = observers.begin();
    while(ptr != observers.end()) {
        if(*ptr == o) {
            ptr = observers.erase(ptr);
            return;
        }
        ++ptr;
    }
}

void Player::left() {
    accelVec[0] -= 1;
}

void Player::right() {
    accelVec[0] += 1;
}

void Player::up() {
    accelVec[1] -= 1;
}

void Player::down() {
    accelVec[1] += 1;
}

void Player::damp() {
    //damp player speed so they slow to a stop
    //don't allow them to accelVecerate past max speed

    //get the magnitude
    float magnitude = getVelocity().magnitude();

    if(magnitude > .01) {
        setVelocity(getVelocity() * damping);
    }
    else {
        setVelocity(getVelocity() * 0);
    }
}

float Player::getAngle() {
    float angle = 0;
    if(cursorPos[0]-getX() != 0){
        angle =
            atan(
                (cursorPos[1]-getY()-imageHeight/2) /
                (cursorPos[0]-getX()-imageWidth/2)
            ) * (RAD_TO_DEG) + 90;
        if(cursorPos[0]-getX() < 0) {
            angle += 180;
        }
    }
    else{
        if(cursorPos[1]-getY() >= 0){
            angle = 0;
        }
        else {
            angle = 180;
        }
    }
    return angle;
}

bool Player::shoot(bulletStruct *b) {
    if(overheated || isDead) {
        //can't shoot when overheated or dead
        return false;
    }

    //push a bullet onto the vector and shoot it
    if((b->timeSinceLastFrame) < (b->interval))
    {
        return false; //too soon to shoot
    }

    Bullet *bullet;
    if(!(b->freeBullets.empty())){
        //get a bullet from free list
        bullet = b->freeBullets.front();
        b->freeBullets.pop_front();

        //reset it, which includes resetting its health
        bullet->reset();
    }
    else {
        //new bullet
        bullet = new Bullet(b->bulletName);
    }

    //make a new bullet position
    Vector2f newPos(
        getScaledWidth()/2-bullet->getScaledWidth()/2,
        getScaledHeight()/2-bullet->getScaledHeight()/2
    );
    newPos += getPosition();

    //position it at the position described
    bullet->setPosition(newPos);

    //make a new bullet velocity, and randomize it according to xml
    //velocity will remain constant, trajectory may be slightly skewed

    if(spread > 0){
        float randX = fmod(rand(), spread) - fmod(rand(), 2*spread);
        float randY = fmod(rand(), spread) - fmod(rand(), 2*spread);

        Vector2f newVel(
            cursorPos[0]-(getX()+getScaledWidth()/2),
            cursorPos[1]-(getY()+getScaledHeight()/2)
        );

        if(newVel[0] != 0 || newVel[1] != 0) {
            newVel = newVel.normalizeSafe() * b->bulletSpeed;
        }

        newVel[0] += randX;
        newVel[1] += randY;

        if(newVel[0] != 0 || newVel[1] != 0) {
            newVel = newVel.normalizeSafe() * b->bulletSpeed;
        }
        newVel += getVelocity();

        bullet->setVelocity(newVel);
    }

    b->bullets.push_back(bullet);
    b->timeSinceLastFrame = 0;

    if(coolTimer <= 0) {
        //no cool ticks left, heat up
        heatUpShoot(b->heatRateShoot);
    }
    else {
        //player has ticks from the cooldown powerup, don't heat up
        coolTimer -= b->heatRateShoot / 4;
        coolTimer = (coolTimer < 0) ? 0 : coolTimer;
    }
    return true;
}

void Player::heatUpShoot(int h) {
    heat += h;
    if(heat > maxHeat) {
        heat = maxHeat;
        overheated = true;
    }
}

void Player::coolDown() {
    if(overheated) {
        //overheat penalty
        heat -= coolRatePenalty;
    }
    else {
        heat -= coolRate;
    }

    if(heat < 0) {
        heat = 0;
        overheated = false;
    }
}

void Player::draw() const {
    for (const Bullet* bullet : primary->bullets ) {
        bullet->draw();
    }
    for (const Bullet* bullet : secondary->bullets ) {
        bullet->draw();
    }

    //don't draw a dead player
    if(isDead) { return; }

    //make the player blink while invincible
    if(isInvincible) {
        if(invincibilityTimer > (invincibilityFrames * 3) / 4) {
            if(((invincibilityTimer / 3) % 2) == 0) {
                return;
            }
        }
        else if(((invincibilityTimer / 5) % 2) == 0)
        {
            return;
        }
    }

    Vector2f pos = getPosition();

    images[currentFrame]->drawRotate(getX(), getY(), getScale(),
        getScaledWidth(), getScaledHeight(),
        cursorPos[0]-pos[0]-imageWidth/2, cursorPos[1]-pos[1]-imageHeight/2);

}

void Player::update(Uint32 ticks) {


    //update player position of the others
    std::list<Drawable*>::iterator ptr = observers.begin();
    while(ptr != observers.end()) {
        (*ptr)->setPlayerPos(getPosition());
        (*ptr)->setPrecisePlayerPos(Vector2f(
            (getPosition()[0] + (getScaledWidth()/2)),
            (getPosition()[1] + (getScaledHeight()/2))
        ));
        (*ptr)->setPlayerIsAlive(!isDead);
        (*ptr)->setPlayerIsOverheated(overheated);
        ++ptr;
    }


    //update primary bullets
    std::list<Bullet*>::iterator bulletp = primary->bullets.begin();
    while(bulletp != primary->bullets.end()) {
        (*bulletp)->update(ticks);
        if((*bulletp)->goneTooLong() || (*bulletp)->getHealth() <= 0) {
            Bullet *b = *bulletp;
            //delete it
            primary->freeBullets.push_back(b);
            bulletp = primary->bullets.erase(bulletp);
        }
        else {
            ++bulletp;
        }
    }
    primary->timeSinceLastFrame += ticks;


    //update secondary bullets
    std::list<Bullet*>::iterator bullet = secondary->bullets.begin();
    while(bullet != secondary->bullets.end()) {
        (*bullet)->update(ticks);
        if((*bullet)->goneTooLong() || (*bullet)->getHealth() <= 0) {
            Bullet *b = *bullet;
            //delete it
            secondary->freeBullets.push_back(b);
            bullet = secondary->bullets.erase(bullet);
        }
        else {
            ++bullet;
        }
    }
    secondary->timeSinceLastFrame += ticks;

//-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -


    if(!isDead && getHealth() <= 0) {
        isDead = true;
        explodeMe = true;
        isInvincible = true;
    }

    if(isInvincible && !isDead) {
        invincibilityTimer++;
        if(invincibilityTimer >= invincibilityFrames) {
            isInvincible = false;
        }
    }

    coolDown();


    if(!isDead) {
        //some input was provided, and the ship is not dead

        //normalize accelVec so the
        accelVec = accelVec.normalizeSafe();
        accelVec[0] *= acceleration;
        accelVec[1] *= acceleration;

        setVelocity(getVelocity() + accelVec);

        accelVec[0] = 0;
        accelVec[1] = 0;
    }
    else if (isDead) {
        //slow the player dwon, so the camera pans to a stop
        damp();
    }

    float limitSpeed = maxSpeed;

    if(overheated) limitSpeed *= movementPenalty;

    if(getVelocity().magnitude() > limitSpeed) {
        //don't let player accelVecerate past max speed
        setVelocity(getVelocity().normalizeSafe() * (limitSpeed));
    }

    rotateSprite::update(ticks);
}
