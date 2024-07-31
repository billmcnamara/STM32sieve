# STM32sieve

### STM32 CubeIDE project implementing the prime counting function.
[ref: wikipedia](https://en.wikipedia.org/wiki/Prime-counting_function)

default setting in [Core\Src\main.c](Core\Inc\st7735.h)

    π(9900000); 


#### ST7735 display

Displays on ST7735 TFT (128x128) by default.
If you want to display to a 160x128, 1680x80 etc, you will need to edit the [Core\Inc\st7735.h](Core\Inc\st7735.h)

#### pinouts
The ST7735 TFT GPIO pinout and wiring are shown via images:

STM CubeIDE GPIO settings see:
![STM CubeIDE GPIO settings](/Img/wiring5.jpg?raw=true "on STM Nucleo-F411RE")

For Pinouts to the TFT see:
![Wiring to ST7735 TFT ](/Img/wiring3.jpg?raw=true "128x128")
![Wiring to Nucleo](/Img/wiring2.jpg?raw=true "Nucleo-F411RE")

#### Notes
rescaling and graph reset is still a (when I have time) WIP, tested only up to x = 10^7
