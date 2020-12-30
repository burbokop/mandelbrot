#include <src/gameapplication.h>
#include <src/sdleventhandler.h>
#include <src/sdlgraphicsprovider.h>
#include <src/math/math.h>
#include <src/graphics/imageview.h>

#include <iostream>

const size_t width = 600;
const size_t height = 600;

bool generateMandelbrotImage(e172::AbstractGraphicsProvider *graphicsProvider, const std::string &path, size_t N, size_t depth, e172::Color mask) {
    return graphicsProvider->createImage(N, N, e172::Math::mandelbrot<uint32_t>(depth, mask)).save(path);
}

e172::Image composeWithMandelbrot(e172::AbstractGraphicsProvider *graphicsProvider, const e172::Image &image, const e172::Vector &offset, size_t depth, e172::Color mask) {
    const auto size = image.size().min() - offset.max();
    const auto mandelbrotImage = graphicsProvider->createImage(size, size, e172::Math::mandelbrot<uint32_t>(depth, mask));
    return image.blit(mandelbrotImage, offset.x(), offset.y()).fragment(offset.x(), offset.y(), size, size);
}

int main(int argc, char **argv) {
    size_t depth = 32;
    e172::Color colorMask = 0xffffffff;
    if(argc > 1)
        depth = std::stoul(argv[1]);

    if(argc > 2)
        colorMask = std::stoul(argv[2], nullptr, 16);

    e172::GameApplication app(argc, argv);
    SDLGraphicsProvider graphicsProvider(app.arguments(), "Mandelbrot", width, height);
    SDLEventHandler eventHandler;

    const auto composedImage = composeWithMandelbrot(
                &graphicsProvider,
                graphicsProvider.loadImage("/home/boris/Documents/photo_2019-11-24_14-20-01.jpg"),
                { 100, 50 },
                depth,
                colorMask
                );

    composedImage.save("./composed2.png");

    e172::ImageView mandelbrotView = composedImage;

    app.setGraphicsProvider(&graphicsProvider);
    app.setEventHandler(&eventHandler);
    app.addEntity(&mandelbrotView);

    //generateMandelbrotImage(&graphicsProvider, "./image1024.png", 1024, 32, colorMask);
    //generateMandelbrotImage(&graphicsProvider, "./image2048.png", 2048, 32, colorMask);
    //generateMandelbrotImage(&graphicsProvider, "./image4096.png", 4096, 32, colorMask);

    return app.exec();
}
