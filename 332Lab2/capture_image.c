#include <stdio.h>
#include <time.h>

#define KEY_BASE              0xFF200050
#define VIDEO_IN_BASE         0xFF203060
#define FPGA_ONCHIP_BASE      0xC8000000
#define CHAR_BUFFER_BASE      0xC9000000

volatile char *char_buffer = (char *)CHAR_BUFFER_BASE;
volatile int *KEY_ptr = (int *)KEY_BASE;
volatile int *Video_In_DMA_ptr = (int *)VIDEO_IN_BASE;
volatile short *Video_Mem_ptr = (short *)FPGA_ONCHIP_BASE;

void overlay(int image_count);
void flip();  // Changed name to be more descriptive
void mirror();
void bnw();  // Changed name for clarity
void invert();  // Changed name for clarity

// Function to flip and mirror the image


void flip() {
    int x, y;
    for (y = 0; y < 120; y++) {  // Process only the top half
        for (x = 0; x < 320; x++) {  // Process the full width of the image
            int topPixel = (y << 9) + x;  // Calculate top pixel index
            int bottomPixel = ((239 - y) << 9) + x;  // Calculate bottom pixel index

            // Swap the top and bottom pixels
            short temp = *(Video_Mem_ptr + topPixel);
            *(Video_Mem_ptr + topPixel) = *(Video_Mem_ptr + bottomPixel);
            *(Video_Mem_ptr + bottomPixel) = temp;
        }
    }
}

void mirror() {
    int x, y;
    for (y = 0; y < 240; y++) {  // Process the full height of the image
        for (x = 0; x < 160; x++) {  // Process only the left half
            int leftPixel = (y << 9) + x;  // Calculate left pixel index
            int rightPixel = (y << 9) + (319 - x);  // Calculate right pixel index

            // Swap the left and right pixels
            short temp = *(Video_Mem_ptr + leftPixel);
            *(Video_Mem_ptr + leftPixel) = *(Video_Mem_ptr + rightPixel);
            *(Video_Mem_ptr + rightPixel) = temp;
        }
    }
}

// Function to convert the image to black and white
void bnw() {
    int x, y;
    short pixel;
    short grayscale;

    for (y = 0; y < 240; y++) {
        for (x = 0; x < 320; x++) {
            pixel = *(Video_Mem_ptr + (y << 9) + x);

            short red = (pixel >> 11) & 0x1F;
            short green = (pixel >> 5) & 0x3F;
            short blue = pixel & 0x1F;

            // Use a weighted average for grayscale (0.2989 * red + 0.5870 * green + 0.1140 * blue)
            grayscale = (red * 77 + green * 150 + blue * 29) >> 8;  // This avoids floating-point math

            // Convert grayscale to black (0x0000) or white (0xFFFF) depending on threshold
            short bw = (grayscale > 16) ? 0xFFFF : 0x0000;

            *(Video_Mem_ptr + (y << 9) + x) = bw;
        }
    }
}

// Function to invert the image (invert black and white)
void invert() {
    int x, y;
    short pixel;
    short inverted_pixel;

    for (y = 0; y < 240; y++) {
        for (x = 0; x < 320; x++) {
            pixel = *(Video_Mem_ptr + (y << 9) + x);

            // Invert the pixel using the NOT (~) operation
            inverted_pixel = ~pixel;  // Bitwise NOT operation

            // Assign the inverted pixel value
            *(Video_Mem_ptr + (y << 9) + x) = inverted_pixel;
        }
    }
}

// Function to overlay timestamp on the image
void overlay(int image_count) {
    time_t now;
    struct tm *time_info;
    char overlay_text[40];
    int i;

    time(&now);
    time_info = localtime(&now);
    time_info->tm_hour -= 4;  // Adjust for timezone, e.g., UTC-4
    sprintf(overlay_text, "Time: %02d:%02d:%02d | Image: %d", time_info->tm_hour, time_info->tm_min, time_info->tm_sec, image_count);

    int row = 5, col = 5;
    volatile char *char_ptr = char_buffer + (row << 7) + col;

    for (i = 0; overlay_text[i] != '\0'; i++) {
        *(char_ptr + i) = overlay_text[i]; // Increasing font by spacing characters
    }
}

int main(void) {
    int filter_counter = 0;
    int image_count = 0;
    *(Video_In_DMA_ptr + 3) = 0x4;

    while (1) {
        int button_press = *KEY_ptr;

        if (button_press == 0x1) {  // Button 1 pressed
            *(Video_In_DMA_ptr + 3) = 0x0;  // Disable video to capture frame
            overlay(image_count);

            // Apply the appropriate filter based on the filter counter
            if (++filter_counter == 1) {
                flip();  // Flip and mirror the image
            } else if (filter_counter == 2) {
                mirror();
            } else if (filter_counter == 3) {
                bnw();  // Convert to black and white
            } else if (filter_counter == 4) {
                invert();  // Invert colors (black/white)
            } else {
                filter_counter = 0;  // Reset filter counter after flipping and mirroring
            }

            image_count++;
            while (*KEY_ptr != 0);  // Wait for button release
        } else if (button_press == 0x2) {  // Button 2 pressed
            *(Video_In_DMA_ptr + 3) = 0x4;  // Reset camera by enabling video
            filter_counter = 0;
            while (*KEY_ptr != 0);  // Wait for button release
        }
    }
    return 0;
}
