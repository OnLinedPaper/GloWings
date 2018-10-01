#ifndef ENGINE__H
#define ENGINE__H

#include <vector>
#include <SDL.h>
#include "ioMod.h"
#include "renderContext.h"
#include "clock.h"
#include "world.h"
#include "viewport.h"
#include "player.h"
#include "reticleSprite.h"
#include "behemothSprite.h"
#include "explodingSprite.h"
#include "hud.h"
#include "sound.h"

#define SMALL_EXPLODE 0
#define BIG_EXPLODE 1

#define CONTROLLER_DEADZONE 8000

#define LSTICK_LR 0
#define LSTICK_UD 1
#define LTRIGGER 2
#define RSTICK_LR 3
#define RSTICK_UD 4
#define RTRIGGER 5

#define SOUND_PRIM 0
#define SOUND_SEC 1
#define SOUND_DEST 2
#define SOUND_HIT 3
#define SOUND_OHEAT 4
#define SOUND_COOL 5
#define SOUND_HEAL 6
#define SOUND_PHIT 7
#define SOUND_PDEST 8

class CollisionStrategy;

class Engine {
public:
    Engine ();                            //allow only default constructor
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    ~Engine ();                           //destructor
    void play();
    void switchSprite();

private:
    const RenderContext* rc;
    const IOmod& io;
    Clock& clock;

    SDL_Joystick *gameController;

    SDL_Renderer * const renderer;
    World starfield;
    World starfog;
    World starfront;
    Viewport& viewport;
    HUD& hud;

    SDLSound sound;

    std::vector<Drawable*> behemoths;
    bool behemothIsAlive;
    std::vector<Drawable*> behemothShields;
    bool shieldsAlive;
    std::vector<Drawable*> seekers;
    bool seekersAlive;
    std::vector<Drawable*> mites;
    bool mitesAlive;
    std::vector<Drawable*> darts;
    bool dartsAlive;
    std::vector<Drawable*> birdsv2;
    bool birdsv2Alive;

    bool enemiesAlive;

    std::vector<Drawable*> healthPickups;
    int healthPickupDropRate;
    std::vector<Drawable*> cooldownPickups;
    int cooldownPickupDropRate;


    Player* player;
    ReticleSprite* reticle;
    std::vector<CollisionStrategy*> strategies;
    int currentStrategy;
    int currentSprite;

    bool playerMoved;

    bool makeVideo;

    std::vector<ExplodingSprite *> explodingEntities;

    void draw() const;
    void update(Uint32);

    void printScales() const;
    void checkForCollisions();
    void checkForPlayerCollisions (std::vector<Drawable*> *);
    void checkForBulletCollisions
        (std::vector<Drawable *> *, std::list<Bullet*> *);
    void explodeEntity(const Drawable *, bool);
    void removeDestroyedEntities(std::vector<Drawable*> *);
    void spawnBehemothShields();
    void spawnSeeker();
    void spawnMites();
    void spawnHealthPickup(const Vector2f);
    void spawnCooldownPickup(const Vector2f);
    void spawnEntities();

    int spawnTicks;
    int spawnTimer;
    int spawnDelay;
    int spawnDelayer;

    bool behemothPresent;

    int hudmaxtime;
    int hudtime;
    bool hudIsActive;

    int alertTime;
    int alertTimer;
    void resetAlertTimer() {alertTimer = 0;}

    float currScore;
    float highScore;
    float waveDifficultyMultiplier;
    int wave;
    bool bonusWave;
    int bonusWaveTicks;
    int bonusWaveTimer;

    bool keepPlaying;
    bool paused;
    bool gameOver;
    void resetGame();

    bool debugMode;
};

#endif
