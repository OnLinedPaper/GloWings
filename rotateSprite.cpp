#include "rotateSprite.h"
#include "gamedata.h"
#include "renderContext.h"

rotateSprite::rotateSprite( const std::string& name ) :
    MultiSprite(name)
{ }

void rotateSprite::draw() const {
  	images[currentFrame]->drawRotate(getX(), getY(), getScale(),
        getScaledWidth(), getScaledHeight(), getVelocityX(), getVelocityY());
}

void rotateSprite::update(Uint32 ticks) {
	MultiSprite::update(ticks);
}
