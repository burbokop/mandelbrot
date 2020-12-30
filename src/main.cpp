#include <src/gameapplication.h>
#include <src/sdleventhandler.h>
#include <src/sdlgraphicsprovider.h>
#include <src/math/math.h>
#include <src/graphics/imageview.h>

const size_t width = 600;
const size_t height = 600;

int main(int argc, char **argv) {
    size_t depth = 32;
    e172::Color colorMask = 0xff0000ff;
    if(argc > 1)
        depth = std::stoul(argv[1]);

    if(argc > 2)
        colorMask = std::stoul(argv[2], nullptr, 16);

    e172::GameApplication app(argc, argv);
    SDLGraphicsProvider graphicsProvider(app.arguments(), "Mandelbrot", width, height);
    SDLEventHandler eventHandler;

    e172::ImageView mandelbrotView = graphicsProvider.createImage(width, height, e172::Math::mandelbrot<uint32_t>(depth, colorMask));
    app.setGraphicsProvider(&graphicsProvider);
    app.setEventHandler(&eventHandler);
    app.addEntity(&mandelbrotView);
    return app.exec();
}
