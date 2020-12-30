#include "mandelbrot.h"

#include <iostream>

#include <src/gameapplication.h>
#include <src/sdleventhandler.h>
#include <src/sdlgraphicsprovider.h>

using namespace std;

int main(int argc, char **argv) {
    e172::GameApplication app(argc, argv);
    SDLGraphicsProvider gp(app.arguments(), "mandelbrot", 600, 600);

    gp.loadFont("", "/usr/share/fonts/truetype/freefont/FreeMono.ttf");

    app.setGraphicsProvider(&gp);
    SDLEventHandler eh;
    app.setEventHandler(&eh);

    Mandelbrot mandelbrot;
    app.addEntity(&mandelbrot);

    return app.exec();
}
