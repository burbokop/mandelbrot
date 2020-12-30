#include <src/gameapplication.h>
#include <src/sdleventhandler.h>
#include <src/sdlgraphicsprovider.h>
#include <src/math/math.h>
#include <src/graphics/imageview.h>

const size_t width = 600;
const size_t height = 600;
const size_t depth = 32;
const e172::Color colorMask = e172::argb(0xff, 0xff, 0x88, 0);

int main(int argc, char **argv) {
    e172::GameApplication app(argc, argv);
    SDLGraphicsProvider graphicsProvider(app.arguments(), "Mandelbrot", width, height);
    SDLEventHandler eventHandler;

    e172::ImageView mandelbrotView = graphicsProvider.createImage(width, height, e172::Math::mandelbrot<uint32_t>(depth, colorMask));
    app.setGraphicsProvider(&graphicsProvider);
    app.setEventHandler(&eventHandler);
    app.addEntity(&mandelbrotView);
    return app.exec();
}
