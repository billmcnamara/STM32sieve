# STM32sieve

### STM32 CubeIDE project implementing the prime counting function.
[ref: wikipedia](https://en.wikipedia.org/wiki/Prime-counting_function)

default setting in [Core\Src\main.c](Core\Inc\st7735.h)


The following calculations are perfomed in the main.c

Expected output is shown:

    π(123456);        //     11601
    π(1234567);       //     95360
    π(12345678);      //    809227
    π(123456789);     //   7027260
    π(234567890);     //  12879447


#### ST7735 display

Displays on ST7735 TFT (128x128) by default.
If you want to display to a 160x128, 1680x80 etc, you will need to edit the [Core\Inc\st7735.h](Core\Inc\st7735.h)

#### pinouts

The ST7735 TFT GPIO pinout and wiring are shown via images in Img/ directory:

STM CubeIDE GPIO settings see:

![STM CubeIDE GPIO settings](Img/wiring5.jpg?raw=true "on STM Nucleo-F411RE")

For Pinouts to the TFT see:

![Wiring to ST7735 TFT](Img/wiring3.jpeg?raw=true "128x128")
![Wiring to Nucleo](Img/wiring2.jpeg?raw=true "Nucleo-F411RE")

#### Notes
rescaling and graph reset complete via a single line approximation.
Tested slightly beyond x = 10^9

Room for improvement: change int to unsigned int and continue for ever...
