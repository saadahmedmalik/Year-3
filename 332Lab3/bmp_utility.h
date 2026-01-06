#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring> 


#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t file_type; // "BM"
    uint32_t file_size; // Size of the file (in bytes)
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset_data; // Start position of pixel data (bytes from the beginning of the file)

    BMPFileHeader() 
        : file_type(0x4D42), // "BM"
          file_size(0),
          reserved1(0),
          reserved2(0),
          offset_data(54) {}
};

struct BMPInfoHeader {
    uint32_t size;        // Size of this header (in bytes)
    int32_t width;        // width of bitmap in pixels
    int32_t height;       // height of bitmap in pixels
    uint16_t planes;      // No. of planes for the target device, this is always 1
    uint16_t bit_count;   // No. of bits per pixel
    uint32_t compression; // 0 or 3 - uncompressed
    uint32_t size_image;  // Size of the image's pixel data in bytes
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;      // No. color indexes in the color table
    uint32_t colors_important; // No. of colors used for displaying the bitmap

    BMPInfoHeader() 
        : size(40),
          width(0),
          height(0),
          planes(1),
          bit_count(24),
          compression(0),
          size_image(0),
          x_pixels_per_meter(0),
          y_pixels_per_meter(0),
          colors_used(0),
          colors_important(0) {}
};
#pragma pack(pop)


void writeHeaders(std::ofstream &of, const BMPFileHeader &file_header, const BMPInfoHeader &info_header) {
    of.write((const char*)&file_header, sizeof(file_header));
    of.write((const char*)&info_header, sizeof(info_header));
}





void saveImageGrayscale(const char* filename, const unsigned char* temp, int width, int height) {
    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    // Calculate padding to align rows to a 4-byte boundary
    int paddingAmount = (4 - (width * 3) % 4) % 4;
    file_header.file_size = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + (width * 3 + paddingAmount) * height;

    std::ofstream of(filename, std::ios_base::binary);

    if (!of) {
        std::cerr << "Could not open the output image file." << std::endl;
        return;
    }

    // Initialize BMP headers
    info_header.width = width;
    info_header.height = height;

    // Write headers
    of.write((const char*)&file_header, sizeof(file_header));
    of.write((const char*)&info_header, sizeof(info_header));

    // Write pixel data
    for (int y = height - 1; y >= 0; y--) { // BMP images are stored upside-down
        for (int x = 0; x < width; x++) {
            unsigned char grayValue = temp[x+y*width];
            unsigned char pixel[3] = {grayValue, grayValue, grayValue}; // Grayscale is represented by repeating the same value in R, G, and B
            of.write((char*)pixel, 3);
        }
        // Write padding bytes at the end of each row, if necessary
        unsigned char padding[3] = {0, 0, 0};
        of.write((char*)padding, paddingAmount);
    }

    of.close();
    std::cout << "Image saved to " << filename << std::endl;
}


void saveImageShort(const char* filename, const unsigned short* temp, int width, int height) {
    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    // Calculate padding to align rows to a 4-byte boundary
    int paddingAmount = (4 - (width * 3) % 4) % 4;
    file_header.file_size = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + (width * 3 + paddingAmount) * height;

    std::ofstream of(filename, std::ios_base::binary);

    if (!of) {
        std::cerr << "Could not open the output image file." << std::endl;
        return;
    }

    // Initialize BMP headers
    info_header.width = width;
    info_header.height = height;

    // Write headers
    of.write((const char*)&file_header, sizeof(file_header));
    of.write((const char*)&info_header, sizeof(info_header));

    // Write pixel data
    for (int y = height - 1; y >= 0; y--) { // BMP images are stored upside-down
        for (int x = 0; x < width; x++) {
            short value = temp[x+y*width];
            // Extract the color components
            unsigned char red = (value >> 11) & 0x1F;
            unsigned char green = (value >> 5) & 0x3F;
            unsigned char blue = value & 0x1F;
            // Scale the color components
            red = (red << 3) | (red >> 2);
            green = (green << 2) | (green >> 4);
            blue = (blue << 3) | (blue >> 2);
            // Write the color components
            unsigned char pixel[3] = {blue, green, red}; // BMP format uses BGR
            of.write((char*)pixel, 3);
        }
        // Write padding bytes at the end of each row
        unsigned char padding[3] = {0};
        of.write((char*)padding, paddingAmount);
    }

    of.close();
    std::cout << "Image saved to " << filename << std::endl;
}



struct BMPHeader {
    uint32_t size;
    uint16_t reserved1, reserved2;
    uint32_t offset;
};







