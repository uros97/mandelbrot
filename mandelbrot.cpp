  
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <stdint.h>

static constexpr int IMAGE_WIDTH = 1000;
static constexpr int IMAGE_HEIGHT = 600;

struct MyColor{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};


void updateImage(double zoom, double offsetX, double offsetY) ;
static const int MAX = 127; // maximum number of iterations for mandelbrot()
                           // don't increase MAX or the colouring will look strange
                           
//uint8_t *__restrict imageColors = (uint8_t *) calloc(IMAGE_HEIGHT * IMAGE_WIDTH *4 , sizeof(uint8_t));
//uint8_t *__restrict colors = (uint8_t *) calloc((MAX+1)*3 , sizeof(uint8_t));
uint8_t *__restrict imageColors;
uint8_t *__restrict colors;  

int mandelbrot(double startReal, double startImag) ;
MyColor getColor(int iterations) ;
void updateImageSlice(double zoom, double offsetX, double offsetY, int minY, int maxY) ;


void initColor() {
    for (int i=0; i <= MAX; ++i) {
        MyColor color = getColor(i);
        
        colors[i*3] = color.red;
        colors[i*3 + 1] = color.green;
        colors[i*3 + 2] = color.blue;
        
    }
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

void updateImageSlice(double zoom, double offsetX, double offsetY, int minY, int maxY)
{
    double realstart = 0 * zoom - IMAGE_WIDTH / 2.0 * zoom + offsetX;
    double imagstart = minY * zoom - IMAGE_HEIGHT / 2.0 * zoom + offsetY;

    
    //#pragma acc kernels
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
    imageColors = (uint8_t *) calloc(IMAGE_HEIGHT * IMAGE_WIDTH *4 , sizeof(uint8_t));
    colors = (uint8_t *) calloc((MAX+1)*3 , sizeof(uint8_t));
    initColor();
    sf::Image pngImage;
    updateImage(zoom, offsetX, offsetY);
    pngImage.create(IMAGE_WIDTH, IMAGE_HEIGHT, imageColors);
    pngImage.saveToFile("test.png");
    
    //#pragma acc exit data delete(imageColors[:IMAGE_HEIGHT * IMAGE_WIDTH *4])
    //#pragma acc exit data delete(colors[:(MAX+1)*3])
    
    free(imageColors);
    free(colors);

}

