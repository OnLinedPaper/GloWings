#include <sstream>
#include "hud.h"
#include "gamedata.h"
#include "ioMod.h"
#include "renderContext.h"
#include "player.h"
#include "viewport.h"

HUD& HUD::getInstance() {
    static HUD hud;

    return hud;
}

HUD::HUD() :
    hudImage(RenderContext::getInstance()->getImage("HUD/overlay")),
    heatOkay(RenderContext::getInstance()->getImage("HUD/okay")),
    heatWarn(RenderContext::getInstance()->getImage("HUD/warn")),
    heatOver(RenderContext::getInstance()->getImage("HUD/over")),
    heatChill(RenderContext::getInstance()->getImage("HUD/chill")),
    healthXPos(Gamedata::getInstance().getXmlInt("hudAttrs/healthBar/xLoc")),
    healthYPos(Gamedata::getInstance().getXmlInt("hudAttrs/healthBar/yLoc")),
    healthWidth(Gamedata::getInstance().getXmlInt("hudAttrs/healthBar/width")),
    healthHeight(Gamedata::getInstance().getXmlInt("hudAttrs/healthBar/height")),
    heatXPos(Gamedata::getInstance().getXmlInt("hudAttrs/heatBar/xLoc")),
    heatYPos(Gamedata::getInstance().getXmlInt("hudAttrs/heatBar/yLoc")),
    heatWidth(Gamedata::getInstance().getXmlInt("hudAttrs/heatBar/width")),
    heatHeight(Gamedata::getInstance().getXmlInt("hudAttrs/heatBar/height")),
    warnXPos(Gamedata::getInstance().getXmlInt("hudAttrs/heatWarn/xLoc")),
    warnYPos(Gamedata::getInstance().getXmlInt("hudAttrs/heatWarn/yLoc")),
    warnWidth(Gamedata::getInstance().getXmlInt("hudAttrs/heatWarn/width")),
    warnHeight(Gamedata::getInstance().getXmlInt("hudAttrs/heatWarn/height")),
    interval(Gamedata::getInstance().getXmlInt("hudAttrs/heatWarn/interval")),
    blink(interval),
    showing(true),
    visible(Gamedata::getInstance().getXmlInt("hudAttrs/visibleOnStart")),
    timeout(Gamedata::getInstance().getXmlInt("hudAttrs/timeout")),
    timeLeft(Gamedata::getInstance().getXmlInt("hudAttrs/timeout"))

{ }

void HUD::drawHud(const Player *p) const {

    Vector2f pPos = p->getPosition();

    hudImage->draw(
        pPos[0]-8,
        pPos[1]-23,
        1
    );
}

void HUD::drawHealth(const Player *p) const {
    SDL_Rect health = SDL_Rect();
    float currHealth = (p->getHealth() / p->getMaxHealth());

    Vector2f pPos = p->getPosition();
    Vector2f vPos = Viewport::getInstance().getPosition();

    health.x = healthXPos + pPos[0] - vPos[0];
    health.y = healthYPos + pPos[1] - vPos[1];
    health.w = int(float(healthWidth) * currHealth);
    health.h = healthHeight;


    SDL_Color temp{255,255,255,0};
    temp.r -= (100 * (1 - currHealth));
    temp.g -= (100 * (1 - currHealth));

    //make health bar green
    SDL_SetRenderDrawColor(
        RenderContext::getInstance()->getRenderer(),
        temp.r, temp.g, temp.b, temp.a
    );
    SDL_RenderFillRect(RenderContext::getInstance()->getRenderer(), &health);
}

void HUD::drawHeat(const Player *p) const {
    SDL_Rect heat = SDL_Rect();
    SDL_Color temp{0,0,0,0};

    Vector2f pPos = p->getPosition();
    Vector2f vPos = Viewport::getInstance().getPosition();

    if(p->hasCoolTicks()) {
        heat.x = heatXPos + pPos[0] - vPos[0];
        heat.y = heatYPos + pPos[1] - vPos[1];
        heat.w = heatWidth *
            ((p->getCoolTicks() / 400) < 1 ? p->getCoolTicks() / 400 : 1);
        heat.h = heatHeight;

        temp.r = 64;
        temp.g = 255;
        temp.b = 255;
        temp.a = 0;
    }
    else {
        float currHeat = (p->getHeat() / p->getMaxHeat());

        heat.x = heatXPos + pPos[0] - vPos[0];
        heat.y = heatYPos + pPos[1] - vPos[1];
        heat.w = heatWidth * currHeat;
        heat.h = heatHeight;

        float threshold = .6;

        if(!p->isOverheated()) {
            if(currHeat < threshold) {
                temp.r = 255 * (currHeat / threshold);
                temp.g = 128 + 128 * (currHeat / threshold);
                temp.b = 0;
            }
            else {
                temp.r = 255 - (100 * ((currHeat - threshold) / (1 - threshold)));
                temp.g = 255 - (255 * ((currHeat - threshold) / (1 - threshold)));
                temp.b = 0;
            }
        }
        else {
            temp.r = 155 + 100 * currHeat;
            temp.g = 0;
            temp.b = 0;
        }
    }

    //color heat bar based on current heat
    SDL_SetRenderDrawColor(
        RenderContext::getInstance()->getRenderer(),
        temp.r, temp.g, temp.b, temp.a
    );
    SDL_RenderFillRect(RenderContext::getInstance()->getRenderer(), &heat);
}

void HUD::drawWarn(const Player *p) {

    if(blink <= 0)
    {
        showing = !showing;
        blink = interval;
    }
    blink--;

    float currHeat = (p->getHeat() / p->getMaxHeat());

    Vector2f pPos = p->getPosition();

    int xPos = warnXPos + pPos[0];
    int yPos = warnYPos + pPos[1];

    float threshold = .6;

    if(currHeat > threshold && !(p->isOverheated())) {
        //heat warning, but not overheated

        if(!showing) {
            //blink the indicator
            return;
        }

        heatWarn->draw(xPos, yPos, 1);
    }
    else if(p->isOverheated()) {
        //overheated
        heatOver->draw(xPos, yPos, 1);
    }
    else if(p->hasCoolTicks()) {
        //frozen powerup
        if(!showing) {
            //blink the indicator
            return;
        }
        heatChill->draw(xPos, yPos, 1);
    }
    else {
        heatOkay->draw(xPos, yPos, 1);
    }
}

void HUD::drawPlayerUpdates(const Player *p) {
    if(p->getIsDead()) {
        //don't draw when the player is dead
        return;
    }

    //preserve current renderer color
    SDL_Color temp{0,0,0,0};
    SDL_GetRenderDrawColor(
        RenderContext::getInstance()->getRenderer(),
        &(temp.r), &(temp.g), &(temp.b), &(temp.a)
    );

    drawHealth(p);
    drawHeat(p);
    drawWarn(p);
    drawHud(p);

    //restore current renderer color
    SDL_SetRenderDrawColor(
        RenderContext::getInstance()->getRenderer(),
        temp.r, temp.g, temp.b, temp.a
    );
}

void HUD::update() {
    if(timeLeft > 0) {
        timeLeft -= 1;
        if(timeLeft <= 0) {
            visible = false;
        }
    }
}
