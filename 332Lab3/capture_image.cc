#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include "bmp_utility.h"

#define HW_REGS_BASE       (0xff200000)
#define HW_REGS_SPAN       (0x00200000)
#define HW_REGS_MASK       (HW_REGS_SPAN - 1)
#define VIDEO_BASE         0x0000
#define PUSH_BASE          0x3010

#define FPGA_ONCHIP_BASE   (0xC8000000)
#define IMAGE_WIDTH        320
#define IMAGE_HEIGHT       240
#define IMAGE_SPAN         (IMAGE_WIDTH * IMAGE_HEIGHT * 4)
#define IMAGE_MASK         (IMAGE_SPAN - 1)

int main(void) {
    unsigned short pixels[IMAGE_HEIGHT*IMAGE_WIDTH];
    unsigned char pixels_bw[IMAGE_HEIGHT*IMAGE_WIDTH];
    volatile unsigned int *video_in_dma = NULL;
    volatile unsigned short *video_mem = NULL;
    void *virtual_base;      // Using "virtual_base" for HW regs mapping.
    void *virtual_base2;
    int fd;

    
    

    // Open /dev/mem to access physical memory.
    if ((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1) {
        printf("ERROR: could not open \"/dev/mem\"...\n");
        return 1;
    }

    // Map physical memory into our virtual address space for HW registers.
    virtual_base = mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE);
    if (virtual_base == MAP_FAILED) {
        printf("ERROR: mmap() for HW registers failed...\n");
        close(fd);
        return 1;
    }

    // Map the on‑chip memory where the captured image is stored.
    virtual_base2 = mmap(NULL, IMAGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, FPGA_ONCHIP_BASE);
    if (virtual_base2 == MAP_FAILED) {
        printf("ERROR: mmap() for video memory failed...\n");
        munmap(virtual_base, HW_REGS_SPAN);
        close(fd);
        return 1;
    }

    // Setup pointers for video and buttons
    key_ptr = (unsigned short*)(virtual_base + ((PUSH_BASE)&(HW_REGS_MASK)));
    video_in_dma = (unsigned int *)(virtual_base + VIDEO_BASE);
    video_mem = (volatile unsigned short *)(virtual_base2 + (FPGA_ONCHIP_BASE & IMAGE_MASK));

    // Enable video capture
    *(video_in_dma + 3) = 0x4;  // Enable video capture.
    int value = *(video_in_dma + 3);
    printf("Video In DMA register value: 0x%x\n", value);
    

    
    
        // iterate over image pixels to capture current frame
        while (1) {
            unsigned short btn_val = *key_ptr;
            if (btn_val < 7 && btn_val > 0) {
                printf("KEY %d pressed\n", btn_val);
        
                // Capture frame after button press
                for (int y = 0; y < IMAGE_HEIGHT; y++) {
                    for (int x = 0; x < IMAGE_WIDTH; x++) {
                        int index = (y * IMAGE_WIDTH) + x;
                        pixels[index] = video_mem[(y << 9) + x];
                        pixels_bw[index] = video_mem[(y << 9) + x];
                    }
                }
        
                saveImageShort("final_image_color.bmp", pixels, IMAGE_WIDTH, IMAGE_HEIGHT);
                printf("Image saved to final_image_color.bmp\n");
        
                saveImageGrayscale("final_image_bw.bmp", pixels_bw, IMAGE_WIDTH, IMAGE_HEIGHT);
                printf("Image saved to final_image_bw.bmp\n");
        
                printf("Images saved.\n");
                break;
            }
        }

    // Cleanup: unmap the memory regions.
    if (munmap(virtual_base2, IMAGE_SPAN) != 0) {
        printf("ERROR: munmap() for video memory failed...\n");
        close(fd);
        return 1;
    }
    if (munmap(virtual_base, HW_REGS_SPAN) != 0) {
        printf("ERROR: munmap() for HW registers failed...\n");
        close(fd);
        return 1;
    }
    close(fd);
    return 0;
}