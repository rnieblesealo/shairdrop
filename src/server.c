#include "raylib.h"
#include "sendhelp.h"
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BACKLOG 3
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
 * @brief Starts a listener server
 * @param host The server's hostname
 * @param port The server's port
 * @returns The listener file descriptor if OK, or -1 on error
 */
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

/**
 * @brief Accepts a connection on the given listener file descriptor
 * @param sockfd The listener file descriptor
 * @returns The accepted connection's file descriptor, or -1 on error
 */
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

/**
 * @brief Receives an image packet, writing it into a Raylib image if successful
 * @param clientsockfd fd of the client we would like to receive image from
 * @returns true if the operation succeeded, false otherwise
 */
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

int main(int argc, char *argv[])
{
  /*
   * 1. Start raylib, server, listening
   * 2. Wait for a connection (blocking!)
   * 3. If connection OK, receive image packet (blocking!)
   * 4. If receive OK, begin draw loop
   * 5. Draw image from received packet
   * 6. Exit on normal window close
   */

  if (argc < 3)
  {
    fputs("usage: ./shairs <host> <port>\n", stderr);
    exit(EXIT_FAILURE);
  }

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SHAirDrop");
  SetTargetFPS(60);

  const char *host = argv[1];
  const char *port = argv[2];

  int sockfd       = FireUpTheServer(host, port);
  int clientsockfd = HearOutAMothafucka(sockfd);

  Image img;
  if (!GetHisFuckingPicture(&img, clientsockfd))
  {
    fputs("server: unable to get picture\n", stderr);
    exit(EXIT_FAILURE);
  }

  Texture2D imgTex = LoadTextureFromImage(img);

  puts("Image load OK!\n");

  while (!WindowShouldClose())
  {
    BeginDrawing();

    ClearBackground(BLUE);

    float   drawScale = 10;
    float   drawRot   = 0;
    Vector2 drawPos = {.x = (float)SCREEN_WIDTH / 2 - (float)imgTex.width * drawScale / 2,
                       .y = (float)SCREEN_HEIGHT / 2 -
                            (float)imgTex.height * drawScale / 2};

    DrawTextureEx(imgTex, drawPos, drawRot, drawScale, WHITE);

    EndDrawing();
  }

  UnloadTexture(imgTex);
  UnloadImage(img);

  CloseWindow();
}
