#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <random>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include "gamedata.h"
#include "engine.h"
#include "frameGenerator.h"
#include "collisionStrategy.h"
#include "hud.h"
#include "sprite.h"
#include "multisprite.h"
#include "rotateSprite.h"
#include "reticleSprite.h"
#include "behemothSprite.h"
#include "birdSprite.h"
#include "dartSprite.h"
#include "shieldSprite.h"
#include "seekerSprite.h"
#include "pickupSprite.h"
#include "player.h"
#include "sound.h"

Engine::~Engine() {
    delete player;
    delete reticle;
    for(Drawable *dr : behemoths) {
        delete dr;
    }
    for(Drawable *dr : seekers) {
        delete dr;
    }
    for(Drawable *dr : mites) {
        delete dr;
    }
    for(Drawable *dr : behemothShields) {
        delete dr;
    }
    for(Drawable *dr : darts) {
        delete dr;
    }
    for(Drawable *dr : birdsv2) {
        delete dr;
    }
    for(Drawable *dr : healthPickups) {
        delete dr;
    }
    for(Drawable *dr : cooldownPickups) {
        delete dr;
    }
    for ( CollisionStrategy* strategy : strategies ) {
      delete strategy;
    }
    for(ExplodingSprite *dr : explodingEntities) {
        delete dr;
    }


    SDL_ShowCursor(SDL_ENABLE);
    std::cout << "Terminating program" << std::endl;
}

// ==== initialize the engine =================================================

Engine::Engine() :
    //initialize a render context singleton to make the window
    rc( RenderContext::getInstance() ),
    //initialize an IOmod singleton for writing text to screen
    io( IOmod::getInstance() ),
    //initialize a clock singleton, to track time passage in program
    clock( Clock::getInstance() ),

    gameController(NULL),

    //initialize a renderer singleton, to render an image
    renderer( rc->getRenderer() ),

    //the background star field
    starfield("starfield", Gamedata::getInstance()
        .getXmlInt("starfield/factor") ),
    //the middle distance "fog"
    starfog("starfog", Gamedata::getInstance().getXmlInt("starfog/factor") ),
    //the foreground closters of stars
    starfront("starfront", Gamedata::getInstance()
        .getXmlInt("starfront/factor") ),

    //initialize the viewport singleton, to show the screen
    viewport( Viewport::getInstance() ),

    hud( HUD::getInstance() ),

    sound(),

    behemoths(),
    behemothIsAlive(false),
    behemothShields(),
    shieldsAlive(false),
    seekers(),
    seekersAlive(false),
    mites(),
    mitesAlive(false),
    darts(),
    dartsAlive(false),
    birdsv2(),
    birdsv2Alive(false),

    enemiesAlive(false),

    healthPickups(),
    healthPickupDropRate(Gamedata::getInstance().
        getXmlInt("entityCount/healthPickupDropRate")),
    cooldownPickups(),
    cooldownPickupDropRate(Gamedata::getInstance().
        getXmlInt("entityCount/cooldownPickupDropRate")),

    //initialize the player
    player(new Player("player")),

    reticle(new ReticleSprite("reticle")),

    //collision strategies
    strategies(),
    currentStrategy(1),

    //set the current sprite to 0 for all sprites
    currentSprite(0),

    playerMoved(false),

    //do not make a video by default; can be toggled later by user
    makeVideo( false ),

    explodingEntities(),

    spawnTicks(Gamedata::getInstance().getXmlInt("entityCount/spawnTicks")),
    spawnTimer(0),
    spawnDelay(Gamedata::getInstance().getXmlInt("entityCount/spawnDelay")),
    spawnDelayer(0),

    behemothPresent(Gamedata::getInstance().
        getXmlBool("entityCount/behemothPresent")),

    hudmaxtime(0),
    hudtime(hudmaxtime),
    hudIsActive(false),

    alertTime(40),
    alertTimer(0),

    currScore(0),
    highScore(0),
    waveDifficultyMultiplier(Gamedata::getInstance().
        getXmlFloat("entityCount/waveDifficultyMultiplier")),
    wave(0),
    bonusWave(false),
    bonusWaveTicks(Gamedata::getInstance().
        getXmlInt("entityCount/bonusWaveDuration")),
    bonusWaveTimer(0),

    keepPlaying(false),
    paused(false),
    gameOver(true),

    debugMode(Gamedata::getInstance().getXmlBool("debug"))
{
    //seed the rng
    std::srand(std::time(0));

    //prepare collision strategies
    strategies.push_back( new RectangularCollisionStrategy );
    strategies.push_back( new PerPixelCollisionStrategy );
    strategies.push_back( new MidPointCollisionStrategy );


    //attach reticle to player
    reticle->attach(player);

    //initially track the player
    Viewport::getInstance().setObjectToTrack(player);

    //hide the cursor
    SDL_ShowCursor(SDL_DISABLE);

    if(SDL_NumJoysticks() < 1) {
        std::cout << "no joysticks connected." << std::endl;
    }
    else {
        //there's a joystick plugged in! load it.
        std::cout << SDL_NumJoysticks()
            << " joystick(s) connected." <<std::endl;

        gameController = SDL_JoystickOpen(0);
        if(gameController != NULL) {
            std::cout << "game controller active!" << std::endl;
            std::cout << "controller name: " << SDL_JoystickName(gameController) << std::endl;
            std::cout << "number of axes: " << SDL_JoystickNumAxes(gameController) << std::endl;
            std::cout << "number of buttons: " << SDL_JoystickNumButtons(gameController) << std::endl;
        }
        else {
            std::cout << "warning, could not load game controller :(" <<
                std::endl << "error code: " << SDL_GetError() << std::endl;
        }
    }

    std::cout << "\nLoading complete\n\n";

}

// ==== =======================================================================

void Engine::checkForPlayerCollisions(
    std::vector<Drawable *> *targets) {

    auto it = targets->begin();

    while(it != targets->end()) {
        if(strategies[currentStrategy]->execute(*player, **it)) {
            Drawable *hit = *it;
            if(!(hit->getIsHealth() || hit->getIsCooldown())) {
                sound[SOUND_PHIT];
            }
            if(hit->getForcesOverheat()) { player->forceOverheat(); }
            player->takeDamage(hit->getDamage());
            hit->takeDamage(player->getDamage());
            hit->setLastHitByBullet(false);
            explodeEntity(player, SMALL_EXPLODE);
        }
        ++it;
    }
}

void Engine::checkForBulletCollisions
(std::vector<Drawable *> *targets, std::list<Bullet*> *bullets) {
    std::list<Bullet*>::iterator b = bullets->begin();
    while(b != bullets->end()) {
        auto it = targets->begin();

        while(it != targets->end()) {
            if(strategies[currentStrategy]->execute(**b, **it)) {
                sound[SOUND_HIT];
                Drawable *hit = *it;
                (*b)->takeDamage(hit->getDamage());
                hit->takeDamage((*b)->getDamage());
                hit->setLastHitByBullet(true);
                explodeEntity(hit, SMALL_EXPLODE);
            }
            ++it;
        }
        ++b;
    }
}

void Engine::checkForCollisions() {
    //check each group for collision

    if(player->isCollidable()) {
        //can't collide with a dead or respawning player

        checkForPlayerCollisions(&darts);
        checkForPlayerCollisions(&birdsv2);
        checkForPlayerCollisions(&behemoths);
        checkForPlayerCollisions(&behemothShields);
        checkForPlayerCollisions(&seekers);
        checkForPlayerCollisions(&mites);
    }

    //these two cannot be shot, but can always be collided with
    checkForPlayerCollisions(&healthPickups);
    checkForPlayerCollisions(&cooldownPickups);

    checkForBulletCollisions(&darts, player->getPrimaryBullets());
    checkForBulletCollisions(&birdsv2, player->getPrimaryBullets());
    checkForBulletCollisions(&behemoths, player->getPrimaryBullets());
    checkForBulletCollisions(&behemothShields, player->getPrimaryBullets());
    checkForBulletCollisions(&seekers, player->getPrimaryBullets());
    checkForBulletCollisions(&mites, player->getPrimaryBullets());

    checkForBulletCollisions(&darts, player->getSecondaryBullets());
    checkForBulletCollisions(&birdsv2, player->getSecondaryBullets());
    checkForBulletCollisions(&behemoths, player->getSecondaryBullets());
    checkForBulletCollisions(&behemothShields, player->getSecondaryBullets());
    checkForBulletCollisions(&seekers, player->getSecondaryBullets());
    checkForBulletCollisions(&mites, player->getSecondaryBullets());
}

void Engine::explodeEntity(const Drawable *hit, bool explodeSize) {

    //explode it
    Sprite sprite(
        dynamic_cast<const MultiSprite *>(hit)->getName(),
        dynamic_cast<const MultiSprite *>(hit)->getPosition(),
        dynamic_cast<const MultiSprite *>(hit)->getVelocity(),
        dynamic_cast<const MultiSprite *>(hit)->getCurrentFrame()
    );
    explodingEntities.push_back(new ExplodingSprite(sprite, explodeSize));

}

void Engine::removeDestroyedEntities(std::vector<Drawable *> *d) {
    //all entities that have 0 or less health are dealt with here
    //normal entities are detached from the player's observer list
    //behemoth entities are also detached from the behemoth
    //score is then added

    std::vector<Drawable*>::iterator dr = d->begin();
    while(dr != d->end()) {
        if((*dr)->getHealth() <= 0) {
            if(!((*dr)->getIsHealth() || (*dr)->getIsCooldown()))
            sound[SOUND_DEST];
            //this entity was destroyed.

            Drawable *hit = *dr;

            //add its score, if it was destroyed by the player
            if(hit->getLastHitByBullet()){
                currScore += (
                    hit->getScore() +
                    (hit->getScore()/2) *
                    (player->getHealthFraction() *
                    (bonusWave ? 2 : 1) //double points for bonus wave
                )
                    //give player a bonus for keeping lots of health, up to 150%
                );
            }

            if(hit->getIsHealth() && !gameOver) {
                //health pickup
                if(!(dynamic_cast<pickupSprite *>(hit)->getSelfDestructed())) {
                    //player ran over it
                    sound[SOUND_HEAL];

                    //add 10% health
                    player->addHealth(player->getMaxHealth() / 3);
                }
            }

            if(hit->getIsCooldown() && !gameOver) {
                //cooldown pickup
                if(!(dynamic_cast<pickupSprite *>(hit)->getSelfDestructed())) {
                    //player ran over it
                    sound[SOUND_COOL];

                    //cool down player
                    player->instantCool();
                    player->addCoolTicks(400);
                }
            }

            if(hit->getDropsHealth()) {
                //drops a health pickup when destroyed

                //double drop rate in bonus round
                int dropRate =
                    (bonusWave
                        ? healthPickupDropRate / 2
                        : healthPickupDropRate);
                if((rand() % dropRate) == 0) {

                    spawnHealthPickup(hit->getPosition());
                }
            }

            if(hit->getDropsCooldown()) {
                //drops a cooldown pickup when destroyed

                //double drop rate in bonus round
                    int dropRate =
                        (bonusWave
                            ? cooldownPickupDropRate / 2
                            : cooldownPickupDropRate);

                if((rand() % dropRate) == 0) {

                    spawnCooldownPickup(hit->getPosition());
                }
            }

            //explode it
            explodeEntity(hit, BIG_EXPLODE);

            //detach it

            player->detach(hit);
            if(hit->getIsBehemothEntity() && behemoths.size() > 0) {
                dynamic_cast<behemothSprite *>
                    (behemoths.front())->detachShield(hit);
            }
            delete hit;
            dr = d->erase(dr);
        }
        else {
            dr++;
        }
    }
}

void Engine::spawnBehemothShields() {
    //start spawning shields if the behemoth's health is down 1/4
    if(behemoths.size() > 0) {
        behemothSprite *b = dynamic_cast<behemothSprite *>(behemoths.front());
        if(
            b->getHealth() < ((b->getMaxHealth() * 3) / 4) &&
            b->getShieldCount() < b->getMaxShieldCount() &&
            b->checkShieldSpawn()
        ) {
                shieldSprite* s = new shieldSprite("shield",b->getPosition());
                s->setIsBehemothEntity(true);
                s->scaleToDifficulty(((wave) * waveDifficultyMultiplier));
                behemothShields.push_back(s);
                b->attachShield(s);
                player->attach(s);
        }
    }
}

void Engine::spawnSeeker() {
    //spawn seekers if the behemoth's health is down 3/4

    if(behemoths.size() > 0) {
        behemothSprite *b = dynamic_cast<behemothSprite *>(behemoths.front());
        if(
            b->getHealth() < ((b->getMaxHealth() * 1) / 5) &&
            b->getSeekersSpawned() < b->getSeekerCount()
        )
        {
            //make a seeker
            seekerSprite *seeker =
                new seekerSprite("seeker", b->getPrecisePos());
            seeker->scaleToDifficulty(((wave) * waveDifficultyMultiplier));

            player->attach(seeker);
            seekers.push_back(seeker);

            //don't spawn more seekers than needed
            b->incrementSeekersSpawned();
        }
    }
}

void Engine::spawnMites() {
    //spawn mites if the behemoth's health is down 3/4

    if(behemoths.size() > 0) {
        behemothSprite *b = dynamic_cast<behemothSprite *>(behemoths.front());
        if(
            b->getHealth() < ((b->getMaxHealth() * 2) / 4) &&
            b->getMitesSpawned() < b->getMiteCount()
        )
        {
            //make a seeker
            seekerSprite *mite =
                new seekerSprite("mite", b->getPrecisePos());
            mite->scaleToDifficulty(((wave) * waveDifficultyMultiplier));

            player->attach(mite);
            mites.push_back(mite);

            //don't spawn more seekers than needed
            b->incrementMitesSpawned();
        }
    }
}

void Engine::spawnHealthPickup(const Vector2f position) {
    if(gameOver) {return;}

    pickupSprite *p = new pickupSprite("healthPickup", position);
    p->setIsHealth(true);
    explodeEntity(p, BIG_EXPLODE);
    player->attach(p);
    healthPickups.push_back(p);
}

void Engine::spawnCooldownPickup(const Vector2f position) {
    if(gameOver) {return;}

    pickupSprite *p = new pickupSprite("cooldownPickup", position);
    p->setIsCooldown(true);
    explodeEntity(p, BIG_EXPLODE);
    player->attach(p);
    cooldownPickups.push_back(p);
}

void Engine::spawnEntities() {
    if((!behemothIsAlive && !bonusWave)) {
        //the behemoth is dead and it's not a bonus, or all emenies are cleared
        //count down the new spawn timer and then spawn new enemies.
        if(((wave+1) % 3 == 0) && (bonusWaveTicks >= bonusWaveTimer)) {
            //every third wave is a bonus wave

            bonusWave = true;
        }
        else {
            bonusWave = false;
        }

        if(spawnTimer >= spawnTicks) {
            //spawn the enemies and deal with the score and player

            spawnTimer = 0;

            player->makeInvincible();
            //give player invincibility while enemies spawn

            for(int i = 0; i <
            Gamedata::getInstance().getXmlInt("entityCount/dart"); i++) {
                //push some darts into the vector
                dartSprite *d = new dartSprite("dart");
                d->setDropsCooldown(true);
                d->scaleToDifficulty(((wave) * waveDifficultyMultiplier));
                player->attach(d);
                darts.push_back(d);
            }
            if (behemothPresent && !bonusWave) {
                    behemothIsAlive = true;
                    //push zero or one behemoths into the vector, never more
                    //don't spawn for bonus wave
                    behemothSprite *b = new behemothSprite("behemoth");
                    b->augmentSeekerCount(wave+1);
                    b->scaleToDifficulty(((wave) * waveDifficultyMultiplier));
                    player->attach(b);
                    behemoths.push_back(b);
            }
            else {
                behemothIsAlive = false;
            }
            for(int i=0; i <
            Gamedata::getInstance().getXmlInt("entityCount/bird"); i++) {
                //push some birds into the vector
                birdSprite *bv2 = new birdSprite("bird");
                bv2->setDropsHealth(true);
                bv2->scaleToDifficulty(((wave) * waveDifficultyMultiplier));
                player->attach(bv2);
                birdsv2.push_back(bv2);
            }

            //scale all existing entities to new difficulty
            for(Drawable *dr : darts) {
                dr->scaleToDifficulty(waveDifficultyMultiplier * wave);
            }
            for(Drawable *dr : birdsv2) {
                dr->scaleToDifficulty(waveDifficultyMultiplier * wave);
            }
            for(Drawable *dr : behemoths) {
                dr->scaleToDifficulty(waveDifficultyMultiplier * wave);
            }
            for(Drawable *dr : behemothShields) {
                dr->scaleToDifficulty(waveDifficultyMultiplier * wave);
            }

            wave++;
            resetAlertTimer();
        }
        else if (spawnTimer == 0){
            //add bonus based on remaining health and wave
            currScore += (
                25000 * player->getHealthFraction() *
                    waveDifficultyMultiplier * wave
            );

            if(bonusWave) {
                //restore half of the player's missing health
                player->addHealth(
                    (player->getMaxHealth() - player->getHealth()) / 2
                );
            }
            spawnTimer++;
        }
        else {
            spawnTimer++;
        }
    }
    else if(bonusWave) {
        //it's a bonus wave, incrememnt the timer
        if((bonusWaveTicks >= bonusWaveTimer) && enemiesAlive) {
            bonusWaveTimer++;

            //POWERUP TIME!!
            player->keepInvincible();
            player->instantCool();
            player->boostFireRate();
            player->shootPrimary();
            player->shootSecondary();
        }
        else {
            bonusWave = false;
            bonusWaveTimer = 0;

            player->dropFireRate();
            player->dropInvincible();

            wave++;
            alertTimer++;
        }

    }
    return;
}

void Engine::resetGame() {

    for(Drawable *dr : behemoths) {
        dr->killHealth();
    }
    for(Drawable *dr : seekers) {
        dr->killHealth();
    }
    for(Drawable *dr : mites) {
        dr->killHealth();
    }
    for(Drawable *dr : behemothShields) {
        dr->killHealth();
    }
    for(Drawable *dr : darts) {
        dr->killHealth();
    }
    for(Drawable *dr : birdsv2) {
        dr->killHealth();
    }
    for(Drawable *dr : healthPickups) {
        dr->killHealth();
    }
    for(Drawable *dr : cooldownPickups) {
        dr->killHealth();
    }

    removeDestroyedEntities(&behemoths);

    removeDestroyedEntities(&behemothShields);
    shieldsAlive = (behemothShields.size() <= 0) ? false : true;
    removeDestroyedEntities(&seekers);
    seekersAlive = (seekers.size() <= 0) ? false : true;
    removeDestroyedEntities(&darts);
    dartsAlive = (darts.size() <= 0) ? false : true;
    removeDestroyedEntities(&birdsv2);
    birdsv2Alive = (birdsv2.size() <= 0) ? false : true;
    removeDestroyedEntities(&mites);
    mitesAlive = (mites.size() <= 0) ? false : true;

    enemiesAlive = behemothIsAlive ||
        shieldsAlive ||
        seekersAlive ||
        dartsAlive ||
        birdsv2Alive ||
        mitesAlive;

    removeDestroyedEntities(&healthPickups);
    removeDestroyedEntities(&cooldownPickups);

    player->respawn();
    wave = 0;
    currScore = 0;
    spawnTimer = 0;
    gameOver = false;
}

void Engine::draw() const {
    starfield.draw();
    starfog.draw();
    starfront.draw();

    for(Drawable *dr : healthPickups) {
        dr->draw();
    }
    for(Drawable *dr : cooldownPickups) {
        dr->draw();
    }
    for(Drawable *dr : darts) {
        dr->draw();
    }
    for(Drawable *dr : birdsv2) {
        dr->draw();
    }
    for(Drawable *dr : seekers) {
        dr->draw();
    }
    for(Drawable *dr : mites) {
        dr->draw();
    }
    for(Drawable *dr : behemoths) {
        dr->draw();
    }
    for(Drawable *dr : behemothShields) {
        dr->draw();
    }
    for(ExplodingSprite *dr : explodingEntities) {
        dr->draw();
    }

    reticle->draw();
    player->draw();

    hud.drawPlayerUpdates(player);


    if(hudIsActive || hudtime > 0) {
        viewport.draw("primary bullets", 1500, 30);
        viewport.draw("active: "+std::to_string(player->getPrimaryBulletSize()), 1500, 60);
        viewport.draw("free: "+std::to_string(player->getPrimaryFreeSize()), 1500, 90);

        viewport.draw("secondary bullets", 1500, 120);
        viewport.draw("active: "+std::to_string(player->getSecondaryBulletSize()), 1500, 150);
        viewport.draw("free: "+std::to_string(player->getSecondaryFreeSize()), 1500, 180);

        viewport.draw("wasd to move, mouse to aim, left and right click to fire.", 30, 120);
        viewport.draw("keep an eye on your heat bar, below the ship.", 30, 150);

    }

    std::stringstream sc;
    sc << "score: " << std::fixed << std::setprecision(0) << currScore;

    std::stringstream sh;
    sh << "high score: " << std::fixed << std::setprecision(0) << highScore;


    std::stringstream sp;
    if(!behemothIsAlive && !bonusWave) {
        sp << "wave spawning in: " <<
            ((spawnTicks - spawnTimer) / 40) +1;
    }
    else if(bonusWave) {
        sp << "bonus round! enemy points doubled! " <<
            ((bonusWaveTicks - bonusWaveTimer) / 40) + 1;
    }
    else if(((wave+1) % 3) == 0) {
        sp << "destroy boss to start bonus round!";
    }
    else{
        sp << "destroy boss to trigger next wave!";
    }

    std::stringstream sd;
    sd << "sacrifice half your score to continue? (y/n)";

    std::stringstream sg;
    sg << "game over! press (r) to play.";

    std::stringstream sb;
    sb<< "bonus round";

    std::stringstream title;
    title << "glowings\n\n\n\npress (r)\nto play";

    std::stringstream spause;
    spause << " paused\n\n\n\n  (p) to\n resume";

    std::stringstream instructions;
    instructions << "move: w a s d\nprimary: l mouse\nsecondary: r mouse\n";
    instructions << "\npause: p\nclose: esc\nhelp: f2\n";
    instructions << "\nhealth above ship\nheat below ship";



    if(paused) {
        viewport.draw(spause.str(), 1920/2-75, 1080/2-100);
    }
    if(gameOver) {
        if(((alertTimer / 5) % 10) != 0) {
            viewport.draw(sg.str(), 30, 30);
        }
        if(!paused) {
            viewport.draw(sh.str(), 30, 90);
            viewport.draw(title.str(), 1920/2-75, 1080/2-100);
        }
        viewport.draw(instructions.str(), 30, 180);
    }
    else if(alertTime > alertTimer) {
        if((((alertTimer / 5) % 2) == 0)) {
        }
        else {
            if(!player->getIsDead()) {
                viewport.draw(sp.str(), 30, 30);
            }
        }
    }
    else {
        if(bonusWave) {
            viewport.draw(sb.str(), 1920/2-100, 1080/2-100);
        }
        if(!player->getIsDead()) {
            viewport.draw(sp.str(), 30, 30);
        }
        else {
            if(((alertTimer / 5) % 4) != 0) {
                viewport.draw(sd.str(), 30, 30);
            }
        }
    }


    viewport.draw(sc.str(), 30, 60);

    viewport.draw();

    SDL_RenderPresent(renderer);
}

void Engine::update(Uint32 ticks) {
    if(!gameOver) {
        checkForCollisions();
    }

    spawnBehemothShields();
    spawnSeeker();
    spawnMites();

    if(behemoths.size() <= 0) {
        behemothIsAlive = false;
    }
    spawnEntities();

    for(Drawable *dr : healthPickups) {
        dr->update(ticks);
    }
    for(Drawable *dr : cooldownPickups) {
        dr->update(ticks);
    }
    for(Drawable *dr : behemoths) {
        dr->update(ticks);
    }
    for(Drawable *dr : behemothShields) {
        dr->update(ticks);
    }
    for(Drawable *dr : seekers) {
        dr->update(ticks);
    }
    for(Drawable *dr : mites) {
        dr->update(ticks);
    }
    for(Drawable *dr : darts) {
        dr->update(ticks);
    }
    for(Drawable *dr : birdsv2) {
        dr->update(ticks);
    }
    for(ExplodingSprite *dr : explodingEntities) {
        dr->update(ticks);
    }

    std::vector<ExplodingSprite*>::iterator e = explodingEntities.begin();
    while(e != explodingEntities.end()) {
        if ((*e)->chunkCount() <= 0) {
            ExplodingSprite *hit = *e;
            e = explodingEntities.erase(e);
            delete hit;
        }
        else {
            e++;
        }
    }


    removeDestroyedEntities(&behemoths);

    removeDestroyedEntities(&behemothShields);
    shieldsAlive = (behemothShields.size() <= 0) ? false : true;
    removeDestroyedEntities(&seekers);
    seekersAlive = (seekers.size() <= 0) ? false : true;
    removeDestroyedEntities(&darts);
    dartsAlive = (darts.size() <= 0) ? false : true;
    removeDestroyedEntities(&birdsv2);
    birdsv2Alive = (birdsv2.size() <= 0) ? false : true;
    removeDestroyedEntities(&mites);
    mitesAlive = (mites.size() <= 0) ? false : true;

    enemiesAlive = behemothIsAlive ||
        shieldsAlive ||
        seekersAlive ||
        dartsAlive ||
        birdsv2Alive ||
        mitesAlive;

    removeDestroyedEntities(&healthPickups);
    removeDestroyedEntities(&cooldownPickups);

    player->update(ticks);
    if(player->primedToExplode()) {
        explodeEntity(player, BIG_EXPLODE);
        sound[SOUND_PDEST];
    }
    if(player->isOverheated()) {
        sound[SOUND_OHEAT];
    }
    else {
        sound.stopSound(SOUND_OHEAT);
    }

    alertTimer++;

    reticle->update(ticks);
    starfront.update();
    starfog.update();
    starfield.update();
    hud.update();
    viewport.update(); // always update viewport last


            hudtime--;
}

void Engine::play() {
    SDL_Event event;
    const Uint8* keystate;
    bool done = false;
    Uint32 ticks = clock.getElapsedTicks();
    unsigned int ms = 1000000 / Gamedata::getInstance().getXmlInt("fps");
    FrameGenerator frameGen;

    while ( !done ) {
        // The next loop polls for events, guarding against key bounce:
        while ( SDL_PollEvent(&event) ) {
            keystate = SDL_GetKeyboardState(NULL);
            if (event.type ==  SDL_QUIT) { done = true; break; }

            if(event.type == SDL_KEYDOWN || event.type == SDL_JOYBUTTONDOWN) {
                if (keystate[SDL_SCANCODE_ESCAPE])
                {
                    done = true;
                    break;
                }
                if ( keystate[SDL_SCANCODE_P]
                    || (!gameOver && event.jbutton.button == 7)) {
                    if ( clock.isPaused() ) {
                        paused = false;
                        clock.unpause();
                    }
                    else {
                        paused = true;
                        draw();
                        clock.pause();
                    }
                }
                if(player->getIsDead()){
                    //player died. ask to continue
                    if(keystate[SDL_SCANCODE_Y] || event.jbutton.button == 0) {
                        //they want to continue
                        currScore /= 2;
                        player->respawn();
                    }
                    else if(keystate[SDL_SCANCODE_N]
                        || event.jbutton.button == 1) {
                        //they don't
                        player->respawn();
                        if(currScore > highScore) {
                            highScore = currScore;
                        }
                        gameOver = true;
                    }
                }
                if(gameOver) {
                    //wait for them to restart with r
                    if(keystate[SDL_SCANCODE_R] || event.jbutton.button == 7) {
                        //7 is start
                        resetGame();
                    }
                }
                if (keystate[SDL_SCANCODE_F2]) {
                    hudIsActive = !hudIsActive;
                }
                if(debugMode) {
                    //some helpful debugging binds
                    if (keystate[SDL_SCANCODE_F4] && !makeVideo) {
                        std::cout << "Initiating frame capture" << std::endl;
                        makeVideo = true;
                    }
                    else if (keystate[SDL_SCANCODE_F4] && makeVideo) {
                        std::cout << "Terminating frame capture" << std::endl;
                        makeVideo = false;
                    }
                    if (keystate[SDL_SCANCODE_E]) {
                        //explode the player
                        explodeEntity(player, BIG_EXPLODE);
                    }
                    if (keystate[SDL_SCANCODE_T]) {
                        //chip away at the player
                        explodeEntity(player, SMALL_EXPLODE);
                    }
                    if (keystate[SDL_SCANCODE_I] || event.jbutton.button == 3) {
                        //make the player invincible
                        player->keepInvincible();
                    }
                }
            }
        }

        // In this section of the event loop we allow key bounce:

        ticks = clock.getElapsedTicks();
        if ( ticks > 0 ) {
            clock.incrFrame();

            //-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
            //player movement, mouse

            if (keystate[SDL_SCANCODE_A]) {
                //accelerate player left
                player->left();
                playerMoved = true;
            }
            if (keystate[SDL_SCANCODE_D]) {
                //accelerate player right
                player->right();
                playerMoved = true;
            }
            if (keystate[SDL_SCANCODE_W]) {
                //accelerate player up
                player->up();
                playerMoved = true;
            }
            if (keystate[SDL_SCANCODE_S]) {
                //accelerate player down
                player->down();
                playerMoved = true;
            }

            //-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
            //player movement, joystick

            if( gameController ) {
                if(debugMode) {
                    std::cout << "\n\n";
                    std::cout << "LSTICK LEFT/RIGHT:\t" <<
                    SDL_JoystickGetAxis(gameController, 0) << std::endl;
                    std::cout << "LSTICK UP/DOWN:\t" <<
                    SDL_JoystickGetAxis(gameController, 1) << std::endl;
                    std::cout << "RSTICK LEFT/RIGHT:\t" <<
                    SDL_JoystickGetAxis(gameController, 3) << std::endl;
                    std::cout << "RSTICK UP/DOWN:\t" <<
                    SDL_JoystickGetAxis(gameController, 4) << std::endl;
                }

                int steerLR = SDL_JoystickGetAxis(gameController, LSTICK_LR);
                int steerUD = SDL_JoystickGetAxis(gameController, LSTICK_UD);

                if(abs(steerLR) > 2000) {
                    if(steerLR < 0) {
                        player->left();
                        playerMoved = true;
                    }
                    if(steerLR > 0) {
                        player->right();
                        playerMoved = true;
                    }
                }
                if(abs(steerUD) > 2000) {
                    if(steerUD < 0) {
                        player->up();
                        playerMoved = true;
                    }
                    if(steerUD > 0) {
                        player->down();
                        playerMoved = true;
                    }
                }
            }

            //-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

            if (!(playerMoved)) {
                //if the user did not put any directional input in

                //slow the ship down
                player->damp();
            }
            else {
                playerMoved = false;
            }

            //-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
            //joystick aiming

            if(gameController){
                Vector2f aimVec(
                    SDL_JoystickGetAxis(gameController, RSTICK_LR),
                    SDL_JoystickGetAxis(gameController, RSTICK_UD)
                );

                if(aimVec.magnitude() > 2000) {
                    aimVec = aimVec.normalizeSafe() * 400;
                }
                else {
                    aimVec = Vector2f(
                        player->getVelocityX(),
                        player->getVelocityY());
                    if(aimVec.magnitude() > 1){
                        aimVec = aimVec.normalizeSafe() * 100;
                    }
                }


                SDL_WarpMouseInWindow(
                    rc->getWindow(),
                    aimVec[0] - (Viewport::getInstance().getX() -
                        player->getPrecisePos()[0]),
                    aimVec[1] - (Viewport::getInstance().getY() -
                        player->getPrecisePos()[1])
                );
            }

            //-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

            if (SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(SDL_BUTTON_LEFT)) {
                if(player->shootPrimary()) {sound[SOUND_PRIM];}
            }
            if(SDL_GetMouseState(NULL, NULL)&SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                if(player->shootSecondary()) {sound[SOUND_SEC];}
            }

            //-   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

            if(gameController) {
                if(SDL_JoystickGetAxis(gameController, LTRIGGER) > 20000) {
                    if(player->shootPrimary()) {sound[SOUND_PRIM];}
                }
                if(SDL_JoystickGetAxis(gameController, RTRIGGER) > 20000) {
                    if(player->shootSecondary()) {sound[SOUND_SEC];}
                }
            }

            if (keystate[SDL_SCANCODE_K] && !gameOver) {
                //kill the player
                player->killHealth();
            }

            draw();
            update(ticks);
            if ( makeVideo ) {
                frameGen.makeFrame();
            }
        }
        usleep(ms); //don't busy-wait
    }
}
