#include "raylib.h"
#include "sendhelp.c"
#include "shairdrop.h"
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_IMG_PATH_LEN

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fputs("usage: ./shairc <host> <port>", stderr);
    exit(EXIT_FAILURE);
  }

  const char *host = argv[1];
  const char *port = argv[2];

  // Connect to server, begin loop
  int sockfd;
  if ((sockfd = ConnectToServer(host, port)) == -1)
  {
    fputs("client: failed to connect to server\n", stderr);
    exit(EXIT_FAILURE);
  }

  Image    img;
  char     imgPath[MAX_IMG_PATH_LEN + 1];
  uint8_t *packetBuf;
  size_t   packetLen;

  bool connected = true;
  while (connected)
  {
    // Ask for image path
    scanf("%s", imgPath);

    // Load image at that path
    img = LoadImage(imgPath);

    // Convert the given image into 32-bit RGBA
    // This guarantees that the receiver will have 4 channels
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    if (AssembleImagePacket(&img, &packetBuf, &packetLen) != RC_SUCCESS)
    {
      puts("client: failed to make packet\n");
      exit(EXIT_FAILURE);
    }

    // Now that we're connected, send the image; return the status of this
    if (!SendAll(sockfd, (void *)packetBuf, packetLen))
    {
      fputs("client: failed to send out image\n", stderr);
    }
    else
    {
      fputs("client: image send OK\n", stderr);
    }

    free(packetBuf);
  }

  exit(EXIT_SUCCESS);
}
