#include <iostream>
#include <cmath>
#include "bullet.h"

void Bullet::update(Uint32 ticks) {
    Vector2f pos = getPosition();
    rotateSprite::update(ticks);

    setVelocity(getVelocity() * accel);

    distance += ( hypot(getX()-pos[0], getY()-pos[1]) );
    elapsedTicks += ticks;
    if (elapsedTicks > maxTicks || distance > maxDistance) {
        tooLong = true;
    }
}
