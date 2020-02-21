  
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <omp.h>

static constexpr int IMAGE_WIDTH = 1000;
static constexpr int IMAGE_HEIGHT = 600;

struct MyColor{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};


void updateImage(double zoom, double offsetX, double offsetY) ;
static const int MAX = 600; // maximum number of iterations for mandelbrot()
MyColor *myColors;
                           
uint8_t *__restrict imageColors;
uint8_t *__restrict colors;  

int mandelbrot(double startReal, double startImag) ;
MyColor getColor(int iterations) ;
MyColor getColor16(int iterations) ;
void updateImageSlice(double zoom, double offsetX, double offsetY, int minY, int maxY) ;


void initColor() {
    
    myColors = (MyColor *) calloc(16, sizeof(MyColor));
    myColors[0].red = 66; myColors[0].green = 30; myColors[0].blue = 15;
    myColors[1].red = 25; myColors[1].green = 7; myColors[1].blue = 26;
    myColors[2].red = 9; myColors[2].green = 1; myColors[2].blue = 47;
    myColors[3].red = 4; myColors[3].green = 4; myColors[3].blue = 73;
    myColors[4].red = 0; myColors[4].green = 7; myColors[4].blue = 100;
    myColors[5].red = 12; myColors[5].green = 44; myColors[5].blue = 138;
    myColors[6].red = 24; myColors[6].green = 82; myColors[6].blue = 177;
    myColors[7].red = 57; myColors[7].green = 125; myColors[7].blue = 209;
    myColors[8].red = 134; myColors[8].green = 181; myColors[8].blue = 229;
    myColors[9].red = 211; myColors[9].green = 236; myColors[9].blue = 248;
    myColors[10].red = 241; myColors[10].green = 233; myColors[10].blue = 191;
    myColors[11].red = 248; myColors[11].green = 201; myColors[11].blue = 95;
    myColors[12].red = 255; myColors[12].green = 170; myColors[12].blue = 0;
    myColors[13].red = 204; myColors[13].green = 128; myColors[13].blue = 0;
    myColors[14].red = 153; myColors[14].green = 87; myColors[14].blue = 0;
    myColors[15].red = 106; myColors[15].green = 52; myColors[15].blue = 3;
    
    for (int i=0; i <= MAX; ++i) {
        MyColor color = getColor16(i); //replace this with getColor for simpler coloring
        
        colors[i*3] = color.red;
        colors[i*3 + 1] = color.green;
        colors[i*3 + 2] = color.blue;
        
    }
    
    free(myColors);
//#pragma acc enter data copyin(colors[:(MAX+1)*3])
//#pragma acc enter data copyin(imageColors[:IMAGE_HEIGHT * IMAGE_WIDTH *4])
}

int mandelbrot(double startReal, double startImag) {
    double zReal = startReal;
    double zImag = startImag;
    #pragma acc loop seq
    for (int counter = 0; counter < MAX; ++counter) {
        double r2 = zReal * zReal;
        double i2 = zImag * zImag;
        if (r2 + i2 > 4.0) {
            return counter;
        }
        zImag = 2.0 * zReal * zImag + startImag;
        zReal = r2 - i2 + startReal;
    }
    return MAX;
}

MyColor getColor(int iterations){
    int r, g, b;
    
    // colour gradient:      Red -> Blue -> Green -> Red -> Black
    // corresponding values:  0  ->  16  ->  32   -> 64  ->  127 (or -1)
    if (iterations < 16) {
        r = 16 * (16 - iterations);
        g = 0;
        b = 16 * iterations - 1;
    } else if (iterations < 32) {
        r = 0;
        g = 16 * (iterations - 16);
        b = 16 * (32 - iterations) - 1;
    } else if (iterations < 64) {
        r = 8 * (iterations - 32);
        g = 8 * (64 - iterations) - 1;
        b = 0;
    } else { // range is 64 - 127
        r = 255 - (iterations - 64) * 4;
        g = 0;
        b = 0;
    }
    MyColor retVal;
    retVal.red = r;
    retVal.green = g;
    retVal.blue = b;
    return retVal;
}

MyColor getColor16(int iterations){
    MyColor retVal;
    if(iterations < MAX && iterations > 0){

    
        int i = iterations % 16;
    
        retVal.red = myColors[i].red;
        retVal.green = myColors[i].green;
        retVal.blue = myColors[i].blue;
    
    }
    else{
        retVal.red = 0;
        retVal.green = 0;
        retVal.blue = 0;
    }
    return retVal;
    
}

void updateImageSlice(double zoom, double offsetX, double offsetY, int minY, int maxY)
{
    double realstart = 0 * zoom - IMAGE_WIDTH / 2.0 * zoom + offsetX;
    double imagstart = minY * zoom - IMAGE_HEIGHT / 2.0 * zoom + offsetY;

    #pragma acc parallel loop copy(colors[:(MAX+1)*3], imageColors[:IMAGE_HEIGHT * IMAGE_WIDTH *4])
    {
    for (int x = 0; x < IMAGE_WIDTH; x++) {
        double real = realstart + (x*zoom);
        #pragma acc loop
        for (int y = minY; y < maxY; y++) {
            double imag = imagstart + ((y-minY)*zoom);
            int value = mandelbrot(real, imag);
            imageColors[(y*IMAGE_WIDTH + x)*4 + 0] = colors[value*3];
            imageColors[(y*IMAGE_WIDTH + x)*4 + 1] = colors[value*3 + 1];
            imageColors[(y*IMAGE_WIDTH + x)*4 + 2] = colors[value*3 + 2];
            imageColors[(y*IMAGE_WIDTH + x)*4 + 3] = 255;
        }
        
    }
    }
    
}

void updateImage(double zoom, double offsetX, double offsetY)
{
    const int STEP = IMAGE_HEIGHT; //do whole image in one step for simplicity
    for (int i = 0; i < IMAGE_HEIGHT; i += STEP) {
        updateImageSlice(zoom, offsetX, offsetY, i, std::min(i+STEP, IMAGE_HEIGHT));
    }
}

int main() {
    double offsetX = -0.7; // move around
    double offsetY = 0.0;
    double zoom = 0.004; // allow the user to zoom in and out
    sf::RenderWindow window(sf::VideoMode(IMAGE_WIDTH, IMAGE_HEIGHT), "Mandelbrot");
    window.setFramerateLimit(0);
    
    
    imageColors = (uint8_t *) calloc(IMAGE_HEIGHT * IMAGE_WIDTH *4 , sizeof(uint8_t));
    colors = (uint8_t *) calloc((MAX+1)*3 , sizeof(uint8_t));
    initColor();
    sf::Image pngImage;
    
    double start = omp_get_wtime();
    updateImage(zoom, offsetX, offsetY);
    
    double end = omp_get_wtime();


    printf("Time elapsed: %lf\n", end - start);
    
    pngImage.create(IMAGE_WIDTH, IMAGE_HEIGHT, imageColors);
    //pngImage.saveToFile("test.png");
    sf::Texture texture;
    sf::Sprite sprite;
    //#pragma acc exit data delete(imageColors[:IMAGE_HEIGHT * IMAGE_WIDTH *4])
    //#pragma acc exit data delete(colors[:(MAX+1)*3])
        
    bool stateChanged = true; // track whether the image needs to be regenerated

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    stateChanged = true; // image needs to be recreated when the user changes zoom or offset
                    switch (event.key.code) {
                        case sf::Keyboard::Escape:
                            window.close();
                            break;
                        case sf::Keyboard::Equal:
                            zoom *= 0.9;
                            break;
                        case sf::Keyboard::Dash:
                            zoom /= 0.9;
                            break;
                        case sf::Keyboard::W:
                            offsetY -= 40 * zoom;
                            break;
                        case sf::Keyboard::S:
                            offsetY += 40 * zoom;
                            break;
                        case sf::Keyboard::A:
                            offsetX -= 40 * zoom;
                            break;
                        case sf::Keyboard::D:
                            offsetX += 40 * zoom;
                            break;
                        default: 
                            stateChanged = false;
                            break;
                    }
                default:
                    break;
            }
        }

        if (stateChanged) { 
            updateImage(zoom, offsetX, offsetY);
            pngImage.create(IMAGE_WIDTH, IMAGE_HEIGHT, imageColors);
            texture.loadFromImage(pngImage);
            sprite.setTexture(texture);
            stateChanged = false;
        }
        window.draw(sprite);
        window.display();
    }
    
    free(imageColors);
    free(colors);

}

