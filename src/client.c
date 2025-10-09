// The client ASSEMBLES THE PACKET and SENDS IT OVER

#include "raylib.h"
#include "sendhelp.c"
#include "sendhelp.h"
#include <netdb.h>
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

uint8_t *AssembleImagePacket(Image *image)
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
   *    - Put this at beginning to tell receiver size
   */

  // Determine packet size

  size_t imgSize         = image->width * image->height * RGBA_CHANNEL_COUNT;
  size_t totalPacketSize = 1 * sizeof(size_t) + 3 * sizeof(uint8_t) + imgSize;

  // Make packet object

  uint8_t *imgPacket = (uint8_t *)malloc(totalPacketSize);
  memset((void *)imgPacket, 0, sizeof imgPacket);

  // Copy packet size

  size_t recvPacketSize = totalPacketSize - sizeof(size_t);
  memcpy(&imgPacket[0], &recvPacketSize, sizeof recvPacketSize);

  size_t offset = recvPacketSize;

  // Copy remaining members

  imgPacket[offset + 1] = (uint8_t)image->width;
  imgPacket[offset + 2] = (uint8_t)image->height;
  imgPacket[offset + 3] = (uint8_t)RGBA_CHANNEL_COUNT;

  // Try to copy image data
  // WARNING: Off by 1 could happen here?

  if (imgSize > MAX_IMG_SIZE)
  {
    fputs("client: image too large\n", stderr);
    return NULL;
  }

  memcpy(&imgPacket[offset + 4], image->data, imgSize);

  return imgPacket;
}

bool SendImagePacket(const uint8_t *imgPacket,
                     size_t         imgPacketLen,
                     const char    *host,
                     const char    *port)
{
  // Resolve host
  struct addrinfo hints;

  memset(&hints, 0, sizeof hints);

  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo *res;
  int              rc;
  if ((rc = getaddrinfo(host, port, &hints, &res)) != 0)
  {
    fprintf(stderr, "client: getaddrinfo: %s\n", gai_strerror(rc));
    return false;
  }

  // Bind to first available
  struct addrinfo *curr;
  int              sockfd;
  for (curr = res; curr != NULL; curr = curr->ai_next)
  {
    // Open socket
    if ((sockfd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol)) == -1)
    {
      perror("client: socket");
      continue;
    }

    // A failure here means something went really wrong!
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
      perror("setsockopt (DEADLY FAIL)");

      freeaddrinfo(res);

      return false;
    }

    // No bind since this isn't a server; our identity is not relevant (right now!)

    // Connect!
    if (connect(sockfd, curr->ai_addr, curr->ai_addrlen) == -1)
    {
      perror("client: connect");

      // TODO: We could probably retry a failed connection, but it's ok
      return false;
    }

    break;
  }

  freeaddrinfo(res);

  // Now that we're connected, send the image; return the status of this
  return SendAll(sockfd, (void *)imgPacket, imgPacketLen);
}

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
  uint8_t *imgPacket = AssembleImagePacket(&img);

  // Send it
  if (SendImagePacket(imgPacket, sizeof imgPacket, host, port))
  {
    puts("Sent OK");
  }
  else
  {
    puts("Send FAIL");
  }

  free(imgPacket);

  exit(EXIT_SUCCESS);
}
