#include "raylib.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_IMG_WIDTH 64
#define MAX_IMG_HEIGHT 64
#define RGBA_CHANNEL_COUNT 4
#define MAX_IMG_SIZE (MAX_IMG_WIDTH * MAX_IMG_HEIGHT * RGBA_CHANNEL_COUNT)

void AssembleImagePacket(Image *image) {
  /*
   * Avatar Packet:
   *
   * - Width - 1 byte; 8 bits (max. 2^8 - 1 size; 255); boundscheck this!
   * - Height - 1 byte as well
   * - Channel count - 1 byte, can only be 1 (grayscale), 3 (RGB), or 4 (RGBA)
   *    - We will convert all lower chan. counts into RGBA!
   * - Pixel data itself
   *    - Will need to manip. this for the above effect
   * - Also very important is the size
   *    - Width * height * channel count
   * - And how big the packet is
   *    - 1 + 1 + 1 + (width * height * channel count)
   */

  // Convert the given image into 32-bit RGBA
  // This guarantees that the receiver will have 4 channels
  ImageFormat(image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

  // Packet must be big enough for the largest supported image
  uint8_t imgPacket[3 * sizeof(uint8_t) + MAX_IMG_SIZE];

  imgPacket[0] = (uint8_t)image->width;
  imgPacket[1] = (uint8_t)image->height;
  imgPacket[2] = (uint8_t)RGBA_CHANNEL_COUNT;

  size_t imgSize = image->width * image->height * RGBA_CHANNEL_COUNT;

  // WARNING: Off by 1 could happen here
  if (imgSize > MAX_IMG_SIZE) {
    fputs("client: image too large\n", stderr);
    exit(EXIT_FAILURE);
  }

  memcpy(&imgPacket[3], image->data, imgSize);

  // Packet is now ready!
}
