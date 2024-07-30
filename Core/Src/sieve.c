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

// Variables to keep track of the current logarithmic scales
static double last_log_x = -1.0; // Use -1.0 to indicate the uninitialized state
static double last_log_y = -1.0; // Use -1.0 to indicate the uninitialized state
static bool first_point = true;  // To track if it's the first point being processed
// this is the var for the x/y axis scale
static int int_log_x = 1;
static int int_log_y = 1;

static int int_last_log_x = 1;
static int int_last_log_y = 1;

// Count of primes
static int32_t count = 0;
static int int_y = 1;
static int int_x = 1;

void draw_axis() {
    // the x
    for (int32_t i = 0; i < ST7735_WIDTH; i++) {
        ST7735_DrawPixel(i, 0, ST7735_WHITE);
    }
    // the y
    for (int32_t i = 0; i < ST7735_HEIGHT; i++) {
        ST7735_DrawPixel(0, i, ST7735_WHITE);
    }
}


void draw_axis_text() {
    char log_y_string[16];
    char log_x_string[16];
    int_log_x = (int)floor(last_log_x);
    int_log_y = (int)floor(last_log_y);
    // printf("write: x:10^(%d) , y: 10^(%d)\n\r", int_log_x, int_log_y);
    // Format strings with logarithmic values
    snprintf(log_x_string, sizeof(log_x_string), "x 10^(%d)", int_log_x);
    snprintf(log_y_string, sizeof(log_y_string), "y 10^(%d)", int_log_y);
    int32_t OFFSET_FROM_0 = 3;
    int32_t FONT_WIDTH = 7;
    int32_t FONT_HEIGHT = 10;

    // Determine positions for axis labels
    int32_t draw_at_point_for_x = ST7735_WIDTH - FONT_WIDTH - FONT_HEIGHT;
    int32_t draw_at_point_for_y = ST7735_HEIGHT - FONT_HEIGHT - (strlen(log_y_string) * FONT_WIDTH);

    // Draw the x axis label
    ST7735_WriteString(OFFSET_FROM_0, draw_at_point_for_x, log_x_string, Font_7x10, ST7735_YELLOW, ST7735_BLACK);
    // Draw the y axis label
    ST7735_WriteString(draw_at_point_for_y, OFFSET_FROM_0, log_y_string, Font_7x10, ST7735_YELLOW, ST7735_BLACK);
}

uint32_t find_lower_log10_limit_x(uint32_t current_x) {
    if (current_x == 0) return 1;
    int llog10_x = (int)log10(current_x);
    int lowery_log10 = llog10_x - 1;
    return (uint32_t)pow(10, lowery_log10);
}

uint32_t find_lower_log10_limit_y(uint32_t current_y) {
    if (current_y == 0) return 1;
    int llog10_y = (int)log10(current_y);
    int lowery_log10 = llog10_y - 1 ;
    return (uint32_t)pow(10, lowery_log10);
}

uint32_t find_upper_log10_limit_x(uint32_t current_x) {
    if (current_x == 0) return 1;
    int ulog10_x = (int)log10(current_x);
    int upperx_log10 = ulog10_x + 1;
    return (uint32_t)pow(10, upperx_log10);
}

uint32_t find_upper_log10_limit_y(uint32_t current_y) {
    if (current_y == 0) return 1;
    int ulog10_y = (int)log10(current_y);
    int upperx_log10 = ulog10_y + 1;
    return (uint32_t)pow(10, upperx_log10);
}

// Function to map a value from [0, 1] to an RGB565 color gradient
uint16_t map_to_color(float value) {
    if (value < 0) value = 0;
    if (value > 1) value = 1;

    uint16_t green = (uint16_t)((1.0 - value) * 0x07E0); // Green part
    uint16_t white = (uint16_t)(value * 0xFFFF);         // White part

    uint16_t color = green | white;
    return color;
}
// Function to draw a scaled pixel on the display using the current x and y values
void draw_scaled_pixel(uint32_t x, uint32_t y) {
    // Determine the logarithm base 10 of x and y
    double log_x = (x > 0) ? log10(x) : 0.0;
    double log_y = (y > 0) ? log10(y) : 0.0;

    // Determine the maximum x and y values using the current values
    uint32_t x_min = find_lower_log10_limit_x(x);
    uint32_t x_max = find_upper_log10_limit_x(x);

    uint32_t y_min = find_lower_log10_limit_y(y);
    uint32_t y_max = find_upper_log10_limit_y(y);

    // Check if the integer parts of the log values have changed, indicating a factor of 10 change
    bool reset_scale = false;

    // Update log_x and log_y for display purposes

    int_log_x = (int)floor(log_x);
    int_log_y = (int)floor(log_y);

    // Check for a factor of 10 change
    if (first_point || fabs(int_log_x - int_last_log_x) >= 1) {
        printf("scale x %d, previously %d\n\r", int_log_x, int_last_log_x);
        reset_scale = true;
        x_min = find_lower_log10_limit_x(x);
        x_max = find_upper_log10_limit_x(x);
    }

    // Check for a factor of 10 change
    if (first_point || fabs(int_log_x - int_last_log_x) >= 1) {
        printf("scale y %d, previously %d \n\r", int_log_y, int_last_log_y);
        reset_scale = true;
        y_min = find_lower_log10_limit_y(y);
        y_max = find_upper_log10_limit_y(y);
    }

    // Calculate the range of the x and y data
    uint32_t x_range = x_max - x_min;
    uint32_t y_range = y_max - y_min;

    if (reset_scale) {
        ST7735_FillScreen(ST7735_BLACK);

        // Update log_x and log_y for display purposes
        int_last_log_x = (int)floor(last_log_x);
        int_last_log_y = (int)floor(last_log_y);

        // Update the last log values
        last_log_x = log_x;
        last_log_y = log_y;
        first_point = false;

        // Update log_x and log_y for display purposes
        int_last_log_x = (int)floor(last_log_x);
        int_last_log_y = (int)floor(last_log_y);
        int_log_x = (int)floor(last_log_x);
        int_log_y = (int)floor(last_log_y);

        draw_axis();
        draw_axis_text();
    }

    // Calculate the display coordinates
    int display_x = (int)(((x - x_min) * (ST7735_WIDTH -1 )) / x_range);
    int display_y = (int)(((y - y_min) * (ST7735_HEIGHT -1 )) / y_range);

    if (reset_scale) {
      display_x = 0;
      display_y = 0;
    }
    // Determine color based on logarithm of x
    double max_log_x = (x_max > 0) ? log10(x_max) : 0.0;
    double normalized_log_x = (x_max > 0) ? log_x / max_log_x : 0.0;

    // Map the normalized log value to a color
    uint16_t color = map_to_color((float)normalized_log_x);

    // Draw the pixel on the display with the calculated color
    printf("x, y: %lu, %lu | (%d, %d) | %d, %d\n\r", (unsigned long)x, (unsigned long)y, int_log_x, int_log_y, display_x, display_y);

    ST7735_DrawPixel(display_x, display_y, color);
    char count_x_string[16];
    char count_y_string[16];
    int_x = (int)floor(x);
    int_y = (int)floor(y);
    snprintf(count_x_string, sizeof(count_x_string), "x %d", int_x);
    snprintf(count_y_string, sizeof(count_y_string), "y %d", int_y);
    // Determine positions for axis labels
    int32_t draw_at_point_for_x = 50;
    int32_t draw_at_point_for_y = 70;
    ST7735_WriteString(10, draw_at_point_for_x, count_x_string, Font_7x10, ST7735_WHITE, ST7735_BLACK);
    ST7735_WriteString(10, draw_at_point_for_y, count_y_string, Font_7x10, ST7735_GREEN, ST7735_BLACK);
/*
 *
 *
 *
 *
 */
}

void sieveofe(int32_t c) {
    printf("%" PRId32 "\n\r", c);
    int32_t n = 20;
    int32_t prime[n + 1];
    //Loading the array with numbers from 1 to n
    for (int32_t i = 1; i <= n; i++) {
        prime[i] = i;
    }
    //Start with least prime number, which is 2.
    //No need to check for numbers greater than square root of n.
    //They will be already marked.
    for (int32_t i = 2; i * i <= n; i++) {
        if (prime[i] != -1) {
            //Mark all the multiples of i as -1.
            for (int32_t j = 2 * i; j <= n; j += i)
                prime[j] = -1;
        }
    }
    printf("Prime numbers are: \n\r");
    for (int32_t i = 2; i <= n; i++) {
        if (prime[i] != -1) {
            printf("%" PRId32 "\n\r", i);
        }
    }
}

void simpleSieve(int32_t limit, bool prime[]) {
    for (int32_t p = 2; p * p <= limit; p++) {
        if (prime[p]) {
            for (int32_t i = p * p; i <= limit; i += p)
                prime[i] = false;
        }
    }
}

// Segmented Sieve Algorithm
void segmentedSieve(int32_t n) {
    int32_t segmentSize = 100000; // Segment size
    int32_t sqrtN = (int32_t)sqrt(n);

    // Array to store primes up to sqrtN
    bool* prime = malloc((sqrtN + 1) * sizeof(bool));
    for (int32_t i = 0; i <= sqrtN; i++) prime[i] = true;
    simpleSieve(sqrtN, prime);

    // Array to mark non-primes in the current segment
    bool* isPrime = malloc(segmentSize * sizeof(bool));


    // Process each segment
    for (int32_t low = 1; low <= n; low += segmentSize) {
        int32_t high = low + segmentSize;
        if (high > n + 1) high = n + 1;  // Adjust high for the last segment

        // Initialize the segment as all primes
        for (int32_t i = 0; i < segmentSize && low + i <= n; i++) {
            isPrime[i] = true;
        }

        // Mark non-primes in the current segment
        for (int32_t i = 2; i <= sqrtN; i++) {
            if (prime[i]) {
                // Find the minimum number in the current segment that is a multiple of i
                int32_t loLim = (int32_t)fmax(i * i, (low + i - 1) / i * i);
                if (loLim >= high) continue;
                for (int32_t j = loLim; j < high; j += i) {
                    if (j >= low) {
                        isPrime[j - low] = false;
                    }
                }
            }
        }

        // Count primes in the current segment
        int32_t segmentCount = 0;
        for (int32_t i = 0; i < segmentSize && low + i <= n; i++) {
            if (isPrime[i] && (low + i) > 1) {  // Avoid counting 1 which is not a prime
                segmentCount++;
                count++;
            }
        }

        draw_axis_text();

        draw_scaled_pixel(high - 1, count);
    }

    free(prime);
    free(isPrime);
    printf("Total number of primes less than %" PRId32 ": %" PRId32 "\n\r", n, count);
    HAL_Delay(10000);
}

void π(int n) {
    printf("up to %d\n\r", n);
    ST7735_Init();
    ST7735_FillScreen(ST7735_BLACK);
    draw_axis();

    HAL_Delay(1000);

    segmentedSieve(n);
}
