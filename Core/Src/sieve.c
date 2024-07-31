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
#include <inttypes.h>

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
static int uptoval = 0;
static int count = 0;

void draw_axis() {
	// warning the fonts are rotated differently to the orientation of the screen and the position of the 0,0
	// therefore the x looks wrong..
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
	// warning the fonts are rotated differently to the orientation of the screen and the position of the 0,0
	// therefore the y looks wrong..
    int OFFSET_FROM_0 = 3;

    // info box
    char count_n_string[16];
    snprintf(count_n_string, sizeof(count_n_string), "n %d", uptoval);
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
	// Calculate the range of the x and y data
    int x_range = x_max ;// - x_min;
    int y_range = y_max ;// - x_min;
    // Update int_log_x and int_log_y for display purposes
    int_log_x = (int)floor(log_x);
    int_log_y = (int)floor(log_y);

    // Check if the integer parts of the log values have changed, indicating a factor of 10 change
    bool reset_scale = false;

    // Check for a factor of 10 change
    if ( fabs(int_log_x - int_last_log_x ) >= 1) {
        reset_scale = true;
        printf("scale x %d, previously %d\n\r", int_log_x, int_last_log_x);
        x_min = find_lower_log10_limit(x);
        x_max = find_upper_log10_limit(x);
        last_log_x = log_x;
        int_last_log_x = int_log_x;
    }

    if ( fabs(int_log_y - int_last_log_y ) >= 1) {
        reset_scale = true;
        printf("scale y %d, previously %d \n\r", int_log_y, int_last_log_y);
        y_min = find_lower_log10_limit(y);
        y_max = find_upper_log10_limit(y);
        last_log_y = log_y;
        int_last_log_y = int_log_y;
    }

    if (reset_scale) {
        ST7735_FillScreen(ST7735_BLACK);
        draw_axis();
        draw_axis_text();
    }

    // Calculate the display coordinates
	// warning the fonts are rotated differently to the orientation of the screen and the position of the 0,0
    // therefore the y looks wrong..
    int display_x = (((x - x_min) * (ST7735_WIDTH -1 )) / x_range);
    int display_y = ST7735_HEIGHT - (((y - y_min) * (ST7735_HEIGHT -1 )) / y_range);

    // Determine color based on logarithm of x
    double max_log_x = (x_max > 0) ? log10(x_max) : 0.0;
    double normalized_log_x = (x_max > 0) ? log_x / max_log_x : 0.0;
    uint16_t color = map_to_color((float)normalized_log_x);

    // debug
    printf("x,y: (%d, %d) | log: (%d, %d) | display: (%d,%d)\n\r", x, y, int_log_x, int_log_y, display_x, display_y);

    // info box
    ST7735_DrawPixel(display_x, display_y, color);
    char count_x_string[16];
    snprintf(count_x_string, sizeof(count_x_string), "x %d", x);
    int draw_at_point_for_x = 30;
    ST7735_WriteString(20, draw_at_point_for_x, count_x_string, Font_7x10, ST7735_WHITE, ST7735_BLACK);

    char count_y_string[16];
    snprintf(count_y_string, sizeof(count_y_string), "y %d", y);
    int draw_at_point_for_y = draw_at_point_for_x + FONT_HEIGHT + 1;
    ST7735_WriteString(20, draw_at_point_for_y, count_y_string, Font_7x10, ST7735_GREEN, ST7735_BLACK);

}

void sieveofe(int c)
{
	printf("%d \n\r", c);
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

void simpleSieve(int limit, bool prime[]) {
    for (int p = 2; p * p <= limit; p++) {
        if (prime[p]) {
            for (int i = p * p; i <= limit; i += p)
                prime[i] = false;
        }
    }
}

// Segmented Sieve Algorithm
void segmentedSieve(int n) {
    int segmentSize = 100000; // Segment size
    int sqrtN = (int)sqrt(n);

    // Array to store primes up to sqrtN
    bool* prime = malloc((sqrtN + 1) * sizeof(bool));
    for (int i = 0; i <= sqrtN; i++) prime[i] = true;
    simpleSieve(sqrtN, prime);

    // Array to mark non-primes in the current segment
    bool* isPrime = malloc(segmentSize * sizeof(bool));

    // Process each segment
    for (int low = 1; low <= n; low += segmentSize) {
        int high = low + segmentSize;
        if (high > n + 1) high = n + 1;  // Adjust high for the last segment

        // Initialize the segment as all primes
        for (int i = 0; i < segmentSize && low + i <= n; i++) {
            isPrime[i] = true;
        }

        // Mark non-primes in the current segment
        for (int i = 2; i <= sqrtN; i++) {
            if (prime[i]) {
                // Find the minimum number in the current segment that is a multiple of i
                int loLim = fmax(i * i, (low + i - 1) / i * i);
                if (loLim >= high) continue;
                for (int j = loLim; j < high; j += i) {
                    if (j >= low) {
                        isPrime[j - low] = false;
                    }
                }
            }
        }

        // Count primes in the current segment
        int segmentCount = 0;
        for (int i = 0; i < segmentSize && low + i <= n; i++) {
            if (isPrime[i] && (low + i) > 1) {  // Avoid counting 1 which is not a prime
                segmentCount++;
                count++;
            }
        }

        // Print the segment range and total primes so far
        //printf("%d,%d\n\r", high - 1, count);
        draw_axis_text();
        draw_scaled_pixel(high - 1, count);
    }

    free(prime);
    free(isPrime);
    printf("Total number of primes less than %d: %d\n\r", n, count);
}

void π(int n) {
	uptoval = n;
    printf("\n\rup to %d\n\r", uptoval);

    // reset TFT
    ST7735_Init();
    ST7735_FillScreen(ST7735_BLACK);
    draw_axis();
    HAL_Delay(1000);

    // call the prime calculator
    segmentedSieve(uptoval);
}
