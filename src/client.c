#include "raylib.h"
#include "sendhelp.c"
#include "shairdrop.h"
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    fputs("usage: ./shairc <img path> <host> <port>", stderr);
    exit(EXIT_FAILURE);
  }

  const char *imgPath = argv[1];
  const char *host    = argv[2];
  const char *port    = argv[3];

  // Load image
  Image img = LoadImage(imgPath);

  // Convert the given image into 32-bit RGBA
  // This guarantees that the receiver will have 4 channels
  ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

  // Make packet
  uint8_t *packetBuf;
  size_t   packetLen;
  if (!AssembleImagePacket(&img, &packetBuf, &packetLen))
  {
    puts("client: failed to make packet");
    exit(EXIT_FAILURE);
  }

  // Send it
  if (SendImagePacket(packetBuf, packetLen, host, port))
  {
    puts("Sent OK");
  }
  else
  {
    puts("Send FAIL");
  }

  free(packetBuf);

  exit(EXIT_SUCCESS);
}
