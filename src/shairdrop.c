#include "shairdrop.h"
#include "sendhelp.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int FireUpTheServer(const char *host, const char *port)
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
    return -1;
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

    // Make reuse
    // A failure here means something went really wrong!
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
      perror("server: setsockopt (DEADLY FAIL)");

      freeaddrinfo(res);

      return -1;
    }

    // Bind
    if (bind(sockfd, curr->ai_addr, curr->ai_addrlen) == -1)
    {
      perror("server: bind");
      continue;
    }

    // Listen
    if (listen(sockfd, BACKLOG) == -1)
    {
      perror("server: listen");
      continue;
    }

    break;
  }

  // Get here if server OK
  printf("Listening on %s:%s...\n", host, port);

  freeaddrinfo(res);

  return sockfd;
}

int HearOutAMothafucka(int sockfd)
{
  // WARNING: We don't do anything with their_addr stuff... Remove?

  struct sockaddr their_addr;
  socklen_t       their_addrlen = sizeof their_addr;

  // Accept a connection
  int clientsockfd;
  if ((clientsockfd = accept(sockfd, &their_addr, &their_addrlen)) == -1)
  {
    perror("server: accept");
    return -1;
  }

  return clientsockfd;
}

bool GetHisFuckingPicture(Image *img, int clientsockfd)
{
  uint8_t opcode;
  if (!ReceiveAll(clientsockfd, &opcode, sizeof opcode))
  {
    fputs("server: failed to receive opcode\n", stderr);
    return false;
  }

  printf("server: got opcode %u\n", opcode);

  if (opcode != OPC_RECEIVE_IMG)
  {
    fprintf(stderr, "server: bad opcode (%d)", opcode);
    return false;
  }

  uint16_t packetSize;
  if (!ReceiveAll(clientsockfd, &packetSize, sizeof packetSize))
  {
    fputs("server: failed to receive packet size\n", stderr);
    return false;
  }

  packetSize = ntohs(packetSize);

  printf("server: got packet size %u\n", packetSize);

  // Get the packet itself

  uint8_t *recvdPacket = (uint8_t *)malloc(packetSize);

  if (!ReceiveAll(clientsockfd, recvdPacket, packetSize))
  {
    fputs("server: failed to receive packet", stderr);
    exit(EXIT_FAILURE);
  }

  printf("server: got full packet of size %u\n", packetSize);

  // Get image attributes

  size_t offset = 0;

  uint8_t w = recvdPacket[offset];
  offset += sizeof w;

  uint8_t h = recvdPacket[offset];
  offset += sizeof h;

  uint8_t ch = recvdPacket[offset];
  offset += sizeof ch;

  size_t imgSize = w * h * ch;

  printf("server: got image of size %u * %u * %u = %zu\n", w, h, ch, imgSize);

  // Get image data and write it back

  memset(img, 0, sizeof(Image));

  img->width   = w;
  img->height  = h;
  img->mipmaps = 1;
  img->format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

  img->data = malloc(imgSize);
  memcpy(img->data, &recvdPacket[offset], imgSize);

  free(recvdPacket);

  return true;
}
