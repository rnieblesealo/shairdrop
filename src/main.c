#include "raylib.h"
#include "sendhelp.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char *argv[]) {}
