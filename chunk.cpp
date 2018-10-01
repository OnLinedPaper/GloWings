#include <iostream>
#include <cmath>
#include <random>

#include "chunk.h"
#include "gamedata.h"

void Chunk::update(Uint32 ticks) {
  float yincr = getVelocityY() * static_cast<float>(ticks) * 0.001;
  setY( std::abs(getY())- yincr );
  float xincr = getVelocityX() * static_cast<float>(ticks) * 0.001;
  setX( std::abs(getX())- xincr );

  if ( getY() < 0) {
    setVelocityY(getVelocityY() * -0.5);
  }
  if ( getY() > worldHeight-chunkHeight) {
    setVelocityY(getVelocityY() * -0.5);
  }

  if ( getX() < 0) {
    setVelocityX(getVelocityX() * -0.5);
  }
  if ( getX() > worldWidth-chunkWidth) {
    setVelocityX(getVelocityX() * -0.5);
  }

  distance += ( hypot(xincr, yincr) );
  elapsedTicks += (ticks + (std::rand() % 100));
  if (distance > maxDistance || elapsedTicks > maxTicks) tooFar = true;
}
