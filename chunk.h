#ifndef CHUNK__H
#define CHUNK__H

#include <iostream>
#include "sprite.h"
#include "gamedata.h"

class Chunk : public Sprite {
public:
  explicit Chunk( const Vector2f& pos, const Vector2f vel,
                  const string& name, Image* fm,
                  int cWidth, int cHeight, bool fullEx) :
    Sprite(name, pos, vel, fm),
    distance(0),
    maxDistance(Gamedata::getInstance().getXmlInt(name+"Attrs/distance")),
    elapsedTicks(0),
    maxTicks(Gamedata::getInstance().getXmlInt(name+"Attrs/maxTicks")),
    tooFar(false),
    image(fm),
    worldWidth(Gamedata::getInstance().getXmlInt("world/width")),
    worldHeight(Gamedata::getInstance().getXmlInt("world/height")),
    chunkWidth(cWidth),
    chunkHeight(cHeight),
    fullExplosion(fullEx)
  {
      if(!fullExplosion) {
          //reduce all parameters for a partial explosion
          maxDistance /= 2;
          maxTicks /= 2;
      }
  }

  Chunk(const Chunk& )=default;
  Chunk(      Chunk&&)=default;
  Chunk& operator=(const Chunk& )=default;
  Chunk& operator=(      Chunk&&)=default;

  virtual ~Chunk(){
    delete image;
  }
  virtual void update(Uint32 ticks);
  bool goneTooFar() const { return tooFar; }
  void reset() {
    tooFar = false;
    distance = 0;
  }
private:
  float distance;
  float maxDistance;
  int elapsedTicks;
  int maxTicks;
  bool tooFar;
  Image* image;


  int worldWidth;
  int worldHeight;

  int chunkWidth;
  int chunkHeight;

  bool fullExplosion;
};
#endif
