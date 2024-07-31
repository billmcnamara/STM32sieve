#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> // For strlen
#include <stdbool.h>
#include <math.h>
#include "sieve.h"
#include "main.h"
#include "st7735.h"
#include "fonts.h"
#include "testimg.h"
#include <inttypes.h>


bool debug_on = false;

// The font used
int FONT_WIDTH = 7;
int FONT_HEIGHT = 10;
// Variables to keep track of the current logarithmic scales
static double last_log_x = -1.0;
static double last_log_y = -1.0;

// this is the var for the x/y axis scale
static int int_log_x = 1;
static int int_last_log_x = 1;
static int int_log_y = 1;
static int int_last_log_y = 1;

// Count of primes
int segmentSize = 100000;
static long uptoval = 0;
static long count = 0;

bool print_after_reset_scale = false;

void draw_axis() {
	// Note that the typical screen coordinate system starts from the top-left corner (0, 0).
    // The x-axis increases to the right, and the y-axis increases downward.
    // graphing logic expects a traditional Cartesian plane, you might need to flip the y-axis values.
    // the x
    for (int i = 0; i < ST7735_WIDTH; i++) {
        ST7735_DrawPixel(i, ST7735_HEIGHT - 1, ST7735_WHITE);
    }
    // the y
    for (int i = 0; i < ST7735_HEIGHT; i++) {
        ST7735_DrawPixel(0, i, ST7735_WHITE);
    }
}

void draw_axis_text() {
	// Note that the typical screen coordinate system starts from the top-left corner (0, 0).
    // The x-axis increases to the right, and the y-axis increases downward.
    // graphing logic expects a traditional Cartesian plane, you might need to flip the y-axis values.
    int OFFSET_FROM_0 = 3;

    // info box
    char count_n_string[32];
    snprintf(count_n_string, sizeof(count_n_string), "n %ld", uptoval);
    int draw_at_point_for_n = 20;
    ST7735_WriteString(20, draw_at_point_for_n, count_n_string, Font_7x10, ST7735_RED, ST7735_BLACK);

    // x/y axis
    char log_x_string[16];
    snprintf(log_x_string, sizeof(log_x_string), "x 10^(%d)", int_log_x);
    int draw_at_point_for_x = ST7735_HEIGHT - FONT_WIDTH - (strlen(log_x_string) * FONT_WIDTH);
    ST7735_WriteString(draw_at_point_for_x,ST7735_HEIGHT - OFFSET_FROM_0 - FONT_HEIGHT,log_x_string, Font_7x10, ST7735_YELLOW, ST7735_BLACK);

    char log_y_string[16];
    snprintf(log_y_string, sizeof(log_y_string), "y 10^(%d)", int_log_y);
    int draw_at_point_for_y = OFFSET_FROM_0 ;
    ST7735_WriteString(OFFSET_FROM_0, draw_at_point_for_y, log_y_string, Font_7x10, ST7735_YELLOW, ST7735_BLACK);

    // printf("log: x:10^(%d) , y: 10^(%d)\n\r", int_log_x, int_log_y);
}

void draw_line(int display_x, int display_y, uint16_t color) {
	//RED
    int plotx, ploty;
    int dx = abs(display_x);
    int dy = abs(display_y);
    int sx = display_x >= 0 ? 1 : -1;
    int sy = display_y >= 0 ? 1 : -1;
    int err = dx - dy;
    // this all depends on the orientation.....
    plotx = 0; // Start point x
    ploty = 0; // Start point y

    while (1) {
        // Plot the pixel at the current point
        ST7735_DrawPixel(plotx, ST7735_HEIGHT - 1 - ploty, color); // Invert y-axis for the display

        // Check if we've reached the endpoint
        if (plotx == display_x && ploty == display_y) break;

        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            plotx += sx;
        }
        if (e2 < dx) {
            err += dx;
            ploty += sy;
        }
    }
}

void draw_line_simple_y(int display_x, int display_y, uint16_t color) {
    float slope = (float)display_x / (float)display_y;

    int linex, liney;

    for (liney = 0; liney <= display_y; liney++) {
        linex = (int)(slope * liney); // Calculate x based on the current y
        int displayLiney = ST7735_HEIGHT - 1 - liney; // Invert the y-axis for the display

        // Plot pixel ensuring it stays within display boundaries
        if (linex >= 0 && linex < ST7735_WIDTH && displayLiney >= 0 && displayLiney < ST7735_HEIGHT) {
            ST7735_DrawPixel(linex, displayLiney, color); // Draw pixel at calculated position
        }
    }
}

int find_lower_log10_limit(int current_v) {
    if (current_v == 0) return 1;
    int llog10_v = (int)log10(current_v);
    int lowerv_log10 = llog10_v - 1 ;
    return (int)pow(10, lowerv_log10);
}

int find_upper_log10_limit(int current_w) {
    if (current_w == 0) return 1;
    int ulog10_w = (int)log10(current_w);
    int upperw_log10 = ulog10_w + 1;
    return (int)pow(10, upperw_log10);
}

uint16_t map_to_color(float value) {
    if (value < 0) value = 0;
    if (value > 1) value = 1;
    uint16_t green = (uint16_t)((1.0 - value) * 0x07E0);
    uint16_t white = (uint16_t)(value * 0xFFFF);
    uint16_t color = green | white;
    return color;
}

void draw_scaled_pixel(int x, int y) {
    // Determine the logarithm base 10 of x and y
    double log_x = (x > 0) ? log10(x) : 1.0;
    double log_y = (y > 0) ? log10(y) : 1.0;

    // Determine the maximum x and y values using the current values
    int x_min = find_lower_log10_limit(x);
    int x_max = find_upper_log10_limit(x);
    int y_min = find_lower_log10_limit(y);
    int y_max = find_upper_log10_limit(y);

    // Check if the integer parts of the log values have changed, indicating a factor of 10 change
    bool reset_scale = false;

    // Update int_log_x and int_log_y for display purposes
    int_log_x = (int)floor(log_x);
    int_log_y = (int)floor(log_y);

    // Check for a factor of 10 change
    if ( fabs(int_log_x - int_last_log_x ) >= 1) {
        reset_scale = true;
        print_after_reset_scale = true;
        if (debug_on) {
        	printf("scale x %d, previously %d\n\r", int_log_x, int_last_log_x);
        }
        x_min = find_lower_log10_limit(x);
        x_max = find_upper_log10_limit(x);
        last_log_x = log_x;
        int_last_log_x = int_log_x;
    }

    if ( fabs(int_log_y - int_last_log_y ) >= 1) {
        reset_scale = true;
        print_after_reset_scale = true;
        if (debug_on) {
        	printf("scale y %d, previously %d \n\r", int_log_y, int_last_log_y);
        }
        y_min = find_lower_log10_limit(y);
        y_max = find_upper_log10_limit(y);
        last_log_y = log_y;
        int_last_log_y = int_log_y;
    }

    double log_x_min = log10(x_min);
    double log_x_max = log10(x_max);
    double log_y_min = log10(y_min);
    double log_y_max = log10(y_max);

	// Calculate the range of the x and y data
    double scaleX = (ST7735_WIDTH - 1) / (log_x_max - log_x_min);
    double scaleY = (ST7735_HEIGHT - 1) / (log_y_max - log_y_min);


    // Determine color based on logarithm of x
    double max_log_x = (x_max > 0) ? log10(x_max) : 0.0;
    double normalized_log_x = (x_max > 0) ? log_x / max_log_x : 0.0;
    uint16_t color = map_to_color((float)normalized_log_x);

    // Calculate the display coordinates
	// Note that the typical screen coordinate system starts from the top-left corner (0, 0).
    // The x-axis increases to the right, and the y-axis increases downward.
    // graphing logic expects a traditional Cartesian plane, you might need to flip the y-axis values.
    int display_x = (int)((log10(x) - log_x_min) * scaleX);
    int display_y = (int)((log10(y) - log_y_min) * scaleY);

    if (reset_scale) {
        ST7735_FillScreen(ST7735_BLACK);
        draw_axis();
        draw_axis_text();
    } else {
    	if (print_after_reset_scale) {
            // when resetting the scale, draw a simple line up to the new x,y
    		if (debug_on) {
    			printf("drawing line up to: (%d,%d)\n\r", display_x, display_y );
    		}
            draw_line(display_x, display_y, ST7735_RED);
            // draw_line_simple_y(display_x, display_y, ST7735_GREEN);
            if (debug_on) {
            	printf("drawing point at: (%d,%d)\n\r", display_x, display_y );
            }
    		print_after_reset_scale = false;
    	}
        ST7735_DrawPixel(display_x, ST7735_HEIGHT - display_y, color);
    }

    if (debug_on) {
     printf("x,y: (%d, %d) | log: (%d, %d) | display: (%d,%d)\n\r", x, y, int_log_x, int_log_y, display_x, display_y);
    }

    // info box
    char count_x_string[32];
    snprintf(count_x_string, sizeof(count_x_string), "x %d", x);
    int draw_at_point_for_x = 30;
    ST7735_WriteString(20, draw_at_point_for_x, count_x_string, Font_7x10, ST7735_WHITE, ST7735_BLACK);

    char count_y_string[32];
    snprintf(count_y_string, sizeof(count_y_string), "y %d", y);
    int draw_at_point_for_y = draw_at_point_for_x + FONT_HEIGHT + 1;
    ST7735_WriteString(20, draw_at_point_for_y, count_y_string, Font_7x10, ST7735_GREEN, ST7735_BLACK);

}

void sieveofe(int c)
{
	if (debug_on) {
		printf("%d \n\r", c);
	}
	//int n;
	int n = 20;
	int prime[n+1];
	//Loading the array with numbers from 1 to n
	for(int i = 1; i <= n; i++)
	{
		prime[i] = i;
	}
	//Start with least prime number, which is 2.
	//No need to check for numbers greater than square root of n.
	//They will be already marked.
	for(int i = 2; i*i <= n; i++)
	{
		if(prime[i] != -1)
		{
			//Mark all the multiples of i as -1.
			for(int j = 2*i; j <=n ; j += i)
				prime[j] = -1;
		}
	}
	printf("Prime numbers are: \n\r");
	for(int i=2; i <= n; i++)
	{
		if(prime[i] != -1)
		{
			printf("%d \n\r", i);
		}
	}
}

void simpleSieve(long limit, bool* prime) {
    for (long i = 0; i <=  limit; i++) prime[i] = true;
    for (long p = 2; p * p <= limit; p++) {
        if (prime[p]) {
            for (long i = p * p; i <= limit; i += p) prime[i] = false;
        }
    }
}

// Segmented Sieve Algorithm
void segmentedSieve(long n) {
	if ( n > 700000000 ){
		segmentSize = 10000;
	}
	if ( n > 7000000000 ){
		segmentSize = 1000;
	}
    long sqrtN = (long)sqrt((double)n);

    // Allocate memory for the prime array up to sqrtN
    bool* prime = (bool*)malloc((sqrtN + 1) * sizeof(bool));
    if (!prime) {
        fprintf(stderr, "\nsieve.c:306: Memory allocation failed\n\r");
        return;
    }

    // Find all primes up to sqrtN
    simpleSieve(sqrtN, prime);

    // Array to mark non-primes in the current segment
    bool* isPrime = (bool*)malloc(segmentSize * sizeof(bool));
    if (!isPrime) {
        fprintf(stderr, "\nsieve.c:316: Memory allocation failed\n\r");
        free(prime);
        return;
    }

     count = 0;
    // Process each segment
    for (long low = 2; low <= n; low += segmentSize) {
        long high = low + segmentSize;
        if (high > n + 1) high = n + 1;  // Adjust high for the last segment

        // Initialize the segment as all primes
        for (long i = 0; i < segmentSize && low + i <= n; i++) {
            isPrime[i] = true;
        }

        // Mark non-primes in the current segment
        for (long i = 2; i <= sqrtN; i++) {
            if (prime[i]) {
                // Find the minimum number in the current segment that is a multiple of i
                long loLim = fmax(i * i, ((low + i - 1) / i) * i);
                if (loLim >= high) continue;
                for (long j = loLim; j < high; j += i) {
                    if (j >= low) {
                        isPrime[j - low] = false;
                    }
                }
            }
        }

        // Count primes in the current segment
        long segmentCount = 0;
        for (long i = 0; i < segmentSize && low + i <= n; i++) {
            if (isPrime[i] && (low + i) > 1) {  // Avoid counting 1 which is not a prime
                segmentCount++;
                count++;
            }
        }

        // Print the segment range and total primes so far
        //printf("%ld,%ld\n", high - 1, count);
        draw_axis_text();
        draw_scaled_pixel(high - 1, count);
    }

    free(prime);
    free(isPrime);
    printf("\nTotal number of primes less than %ld: %ld\n\r", n, count);
}

void π(long n) {
	uptoval = n;
	if (debug_on) {
		printf("\n\rup to %ld\n\r", uptoval);
	}

    // reset TFT
    ST7735_Init();
    ST7735_FillScreen(ST7735_BLACK);
    ST7735_DrawImage(0,0, ST7735_WIDTH, ST7735_HEIGHT,  (uint16_t*)test_img_128x128);
    HAL_Delay(2000);

    draw_axis();
    HAL_Delay(1000);

    // call the prime calculator
    segmentedSieve(uptoval);
}
