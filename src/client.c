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

enum OPCODES : uint8_t
{
  OPC_RECEIVE_IMG = 1
};

/**
 * @brief Builds a packet out of the data in a raylib image
 * @param image The image to build packet from
 * @returns Pointer to the packet
 */
bool AssembleImagePacket(Image *image, uint8_t **packetBuf, size_t *packetLen)
{
  /*
   * ┌──────────────────────────────┐
   * │ OPCODE (1 byte)              │
   * ├──────────────────────────────┤
   * │ RECV PACKET SIZE (8 bytes)   │
   * ├──────────────────────────────┤
   * │ IMG WIDTH (1 byte)           │
   * ├──────────────────────────────┤
   * │ IMG HEIGHT (1 byte)          │
   * ├──────────────────────────────┤
   * │ IMG CHANNEL COUNT (1 byte)   │
   * ├──────────────────────────────┤
   * │ IMG DATA (w * h * ch bytes)  │
   * └──────────────────────────────┘
   */

  uint8_t  opcode         = OPC_RECEIVE_IMG;
  uint16_t recvPacketSize = 0; // We don't know this yet
  uint8_t  w              = image->width;
  uint8_t  h              = image->height;
  uint8_t  ch             = RGBA_CHANNEL_COUNT;
  size_t   imgSize        = w * h * ch;

  if (imgSize > MAX_IMG_SIZE)
  {
    fputs("client: image too large\n", stderr);
    return false;
  }

  size_t totalPacketSize =
      sizeof opcode + sizeof recvPacketSize + sizeof w + sizeof h + sizeof ch + imgSize;

  recvPacketSize = totalPacketSize - sizeof opcode - sizeof recvPacketSize;

  *packetBuf   = (uint8_t *)malloc(totalPacketSize);
  uint8_t *buf = *packetBuf; // Alias for easier legibility

  // Build out the packet

  memset(buf, 0, totalPacketSize); // Ensure packet memory is clean!

  size_t offset = 0;

  buf[offset] = opcode;
  offset += sizeof opcode;

  uint16_t nRecvPacketSize = htons(recvPacketSize);

  memcpy(&buf[offset], &nRecvPacketSize, sizeof nRecvPacketSize);
  offset += sizeof recvPacketSize;

  buf[offset] = w;
  offset += sizeof w;

  buf[offset] = h;
  offset += sizeof h;

  buf[offset] = ch;
  offset += sizeof ch;

  // TODO: Why don't we need to htons the image data itself?
  memcpy(&buf[offset], image->data, imgSize);

  // Update size
  *packetLen = totalPacketSize;

  return true;
}

bool SendImagePacket(const uint8_t *packet,
                     size_t         packetLen,
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

  size_t opc = packet[0];

  // Now that we're connected, send the image; return the status of this
  return SendAll(sockfd, (void *)packet, packetLen);
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
