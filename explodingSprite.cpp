#include <iostream>
#include <cmath>
#include "explodingSprite.h"

ExplodingSprite::ExplodingSprite(const Sprite& s, bool fullExplosion) :
  Sprite(s),
  chunks(),
  freeList(){
  makeChunks(
    Gamedata::getInstance().getXmlInt(s.getName()+"/chunkAttrs/size"),
    fullExplosion
  );
}

ExplodingSprite::~ExplodingSprite() {
  for ( Chunk* c : chunks ) {
    delete c;
  }
  for ( Chunk* c : freeList ) {
    delete c;
  }
  chunks.clear();     // clearing a vector is a waste of time, but ...
  freeList.clear();   // still ...
}

void ExplodingSprite::draw() const {
  // We use the draw in Chunk, which derives from Sprite.
  // So the draw we're using is actually in Sprite
  for ( Chunk* chunk : chunks ) {
    chunk->draw();
  }
}

void ExplodingSprite::update(Uint32 ticks) {
  std::list<Chunk*>::iterator ptr = chunks.begin();
  while (ptr != chunks.end()) {
    (*ptr)->update(ticks);
    if ( (*ptr)->goneTooFar()) {  // Check to see if we should free a chunk
      freeList.push_back(*ptr);
      ptr = chunks.erase(ptr);
    }
    else ++ptr;
  }
}

void ExplodingSprite::makeChunks(unsigned int m, bool fullExplosion) {
  // Break the SDL_Surface into n*n squares; where each square
  // has width and height of imageWidth/n and imageHeight/n
  // Note that "n" s/b a perfect square.
  unsigned int n = m;
  if(!fullExplosion) {
      //make chunks twice as small
      n *= 2;
  }

  int chunk_width = std::max(1u, getImage()->getWidth()/n);
  int chunk_height = std::max(1u, getImage()->getHeight()/n);
  int speedx = static_cast<int>(getVelocityX()); // Wanna test for zero...
  int speedy = static_cast<int>(getVelocityY()); // Make sure it's an int.
  if (speedx == 0) speedx = 1; // Make sure it's not 0;
  if (speedy == 0) speedy = 1; // Make sure it's not 0;

  const Image* proto = getImage();

  int source_y = 0;
  while  (source_y+chunk_height < getImage()->getHeight() ) {
    int source_x = 0;
    while ( source_x+chunk_width < getImage()->getWidth() ) {
      // Give each chunk it's own speed/direction:
      float sx = (rand() % (abs(speedx) + 1)) * (rand()%2?-1:1) - speedx; // 'cause %0 is
      float sy = (rand() % (abs(speedy) + 1)) * (rand()%2?-1:1) - speedy; // float except
      if(fabs(sx) <= 20) { sx = (rand() % 300) * (rand()%2?-1:1); }
      if(fabs(sy) <= 20) { sy = (rand() % 300) * (rand()%2?-1:1); }

      if(fullExplosion || !(rand() % 100)) {
          //only put one in every n chunks in
          Image* image =
            proto->crop({source_x,source_y,chunk_width,chunk_height});
          Chunk* chunk = new Chunk(
                    Vector2f(getX()+source_x,   // x coord of destination
                             getY()+source_y),  // y coord of destination
                    Vector2f(sx, sy),
                    getName()+"/chunk",
                    image,
                    chunk_width,
                    chunk_height,
                    fullExplosion);
          chunk->setScale( getScale() );
          chunks.push_back(chunk);
      }
      source_x += chunk_width;
    }
    source_y += chunk_height;
  }
}
