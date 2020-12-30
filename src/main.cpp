#include <src/gameapplication.h>
#include <src/sdleventhandler.h>
#include <src/sdlgraphicsprovider.h>
#include <src/math/math.h>
#include <src/graphics/imageview.h>

const size_t width = 600;
const size_t height = 600;

bool generateMandelbrotImage(e172::AbstractGraphicsProvider *graphicsProvider, const std::string &path, size_t N, size_t depth, e172::Color mask) {
    return graphicsProvider->createImage(N, N, e172::Math::mandelbrot<uint32_t>(depth, mask)).save(path);
}

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

    generateMandelbrotImage(&graphicsProvider, "./image1024.png", 1024, 32, colorMask);
    generateMandelbrotImage(&graphicsProvider, "./image2048.png", 2048, 32, colorMask);
    generateMandelbrotImage(&graphicsProvider, "./image4096.png", 4096, 32, colorMask);

    return app.exec();
}
