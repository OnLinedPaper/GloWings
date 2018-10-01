#include "reticleSprite.h"
#include "gamedata.h"
#include "renderContext.h"
#include "viewport.h"

ReticleSprite::ReticleSprite( const std::string& name ) :
    MultiSprite(name),
    observers()
{ }

void ReticleSprite::draw() const {
    images[currentFrame]->draw(
        getX()-(imageWidth/2), getY()-(imageHeight/2), getScale()
    );
}

void ReticleSprite::detach( Player* p ) {
    std::list<Player*>::iterator ptr = observers.begin();
    while( ptr != observers.end() ) {
        if( *ptr == p ) {
            ptr = observers.erase(ptr);
            return;
        }
        ++ptr;
    }
}

void ReticleSprite::notify() {
    std::list<Player*>::iterator ptr = observers.begin();
    while( ptr != observers.end() ) {
        (*ptr)->setCursorPos( getPosition() );
        ++ptr;
    }
}

void ReticleSprite::update(Uint32 ticks) {
	MultiSprite::update(ticks);
    int x, y = 0;
    SDL_GetMouseState(&x, &y);
    setX(x+Viewport::getInstance().getX());
    setY(y+Viewport::getInstance().getY());

    notify();
}
