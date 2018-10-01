#ifndef HUD__H
#define HUD__H

#include <sstream>
#include "player.h"

class HUD {
public:
    static HUD &getInstance();
    void drawPlayerUpdates(const Player *p);
    void drawHud(const Player *p) const;
    void drawHealth(const Player *p) const;
    void drawHeat(const Player *p) const;
    void drawWarn(const Player *p);
    void update();

    void setVisible(const bool v) { visible = v; }
    bool getVisible() const { return visible; }
    void toggleVisible() { visible = !visible; }

    void setTimer(const int i) { timeLeft = i; }
    int getTimer() const { return timeLeft; }

private:
    const Image * hudImage;
    const Image * heatOkay;
    const Image * heatWarn;
    const Image * heatOver;
    const Image * heatChill;

protected:
    int healthXPos;
    int healthYPos;
    int healthWidth;
    int healthHeight;

    int heatXPos;
    int heatYPos;
    int heatWidth;
    int heatHeight;

    int warnXPos;
    int warnYPos;
    int warnWidth;
    int warnHeight;
    int interval;
    int blink;
    bool showing;

    bool visible;
    int timeout;
    int timeLeft;

    HUD();
    HUD(const HUD&);
    HUD& operator=(const HUD&);
};

#endif
