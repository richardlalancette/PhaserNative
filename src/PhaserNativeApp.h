#ifndef PHASERNATIVEAPP_H
#define PHASERNATIVEAPP_H

#include <vector>

#include <SDL2/SDL_gamecontroller.h>
#include <PhaserNativeScript.h>
#include <PhaserNativeWindow.h>

class PhaserNativeApp
{
public:

    struct GameDevice
    {
        SDL_GameController *gameController = nullptr;
        SDL_JoystickID joystickID = 0;
    };


    PhaserNativeApp();
    ~PhaserNativeApp();

    int run(int argc, char* argv[]);

private:

    void renderFrame();
    void processEvent();

    PhaserNativeScript m_scriptEngine;
    PhaserNativeWindow m_window;

    std::vector<const char *> javascriptFiles;
    bool m_running = false;
    std::vector<GameDevice> m_gameDevices;


};

#endif // PHASERNATIVEAPP_H
