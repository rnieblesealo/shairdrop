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

int main(int argc, char *argv[])
{
  // Fire up tha server
  // Yuh
  // ...Help me

  if (argc < 3)
  {
    fputs("usage: ./shairs <host> <port>", stderr);
    exit(EXIT_FAILURE);
  }

  const char *host = argv[1];
  const char *port = argv[2];

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
    exit(EXIT_FAILURE);
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

      exit(EXIT_FAILURE);
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

  struct sockaddr their_addr;
  socklen_t       their_addrlen = sizeof their_addr;

  // Accept a connection
  int clientsockfd;
  if ((clientsockfd = accept(sockfd, &their_addr, &their_addrlen)) == -1)
  {
    perror("server: accept");
    exit(EXIT_FAILURE);
  }

  // Receive the packet
  // First, receive sizeof(size_t) to grab the packet size
  size_t packetSize;
  if (!ReceiveAll(clientsockfd, &packetSize, sizeof packetSize))
  {
    fputs("server: failed to receive packet size", stderr);
    exit(EXIT_FAILURE);
  }

  // Show size
  printf("server: got packet size %zu\n", packetSize);

  exit(EXIT_SUCCESS);
}
