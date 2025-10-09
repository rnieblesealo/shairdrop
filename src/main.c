/*
 * VERY FUCKING SHIT DESIGN COMMENT
 *
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
 * - After further thinking, an opcode header makes a lot of sense as well
 *  - We might get a packet and a size but we have no fucking idea what the packet is meant for... How do we know what to do with it?
 *  - Do this later
 *  - I am hungry
 *  - :D
 *
 * Pipeline:
 *
 *  1. Load image using Raylib
 *  2. Extract packet contents
 *  3. Assemble packet
 *  4. Send
 *  5. Server receives image
 *  6. Server displays image
 *    - We never save it to filesystem though
 *
 * Notes:
 *
 * - Using TCP INET4 for simplicity
 * - sendall, recvall to ensure shit makes it across
 * - No network thread yet; will add tho
 *
 * Style:
 *
 * - PascalCase for functions and structs
 * - camelCase for variable names
 * - .clangd does everything else :)
 */

#include "raylib.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500
#define MAX_IMG_WIDTH 64
#define MAX_IMG_HEIGHT 64
#define RGBA_CHANNEL_COUNT 4
#define MAX_IMG_SIZE (MAX_IMG_WIDTH * MAX_IMG_HEIGHT * RGBA_CHANNEL_COUNT)

Image AssembleImagePacket(Image *image)
{
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

  // Cleanup
  memset(&imgPacket, 0, sizeof imgPacket);

  imgPacket[0] = (uint8_t)image->width;
  imgPacket[1] = (uint8_t)image->height;
  imgPacket[2] = (uint8_t)RGBA_CHANNEL_COUNT;

  size_t imgSize = image->width * image->height * RGBA_CHANNEL_COUNT;

  // WARNING: Off by 1 could happen here
  if (imgSize > MAX_IMG_SIZE)
  {
    fputs("client: image too large\n", stderr);
    exit(EXIT_FAILURE);
  }

  memcpy(&imgPacket[3], image->data, imgSize);

  // Packet is now ready!
  // Decode it to check

  uint32_t w  = (uint32_t)imgPacket[0];
  uint32_t h  = (uint32_t)imgPacket[1];
  uint32_t ch = (uint32_t)imgPacket[2];

  size_t recvImgSize = w * h * ch;

  Image reco;

  reco.width   = w;
  reco.height  = h;
  reco.mipmaps = 1;
  reco.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
  reco.data    = (void *)malloc(recvImgSize);

  memcpy(reco.data, &imgPacket[3], recvImgSize);

  puts("Assemble OK");

  return reco;
}

int main(void)
{

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SHAirDrop");
  SetTargetFPS(60);

  Image skull     = LoadImage("./assets/skull.png");
  Image recoSkull = AssembleImagePacket(&skull);

  Texture2D recoSkullTex = LoadTextureFromImage(recoSkull);

  while (!WindowShouldClose())
  {
    BeginDrawing();

    ClearBackground(BLUE);

    Vector2 drawPos   = {.x = 0, .y = 0};
    float   drawScale = 5;
    float   drawRot   = 0;
    DrawTextureEx(recoSkullTex, drawPos, drawRot, drawScale, WHITE);

    EndDrawing();
  }

  UnloadTexture(recoSkullTex);
  UnloadImage(recoSkull);
  UnloadImage(skull);

  CloseWindow();

  exit(EXIT_SUCCESS);
}
