#include "ansihelp.h"
#include "raylib.h"
#include "sendhelp.c"
#include "shairdrop.h"
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_IMG_PATH_LEN 128

// Message & error macros
// No need to worry about newlines!
#define CLIENT_MESSAGE(message) puts(ANSI_GREEN "[CLIENT] " ANSI_RESET message);
#define CLIENT_MESSAGEF(message, ...)                                                    \
  printf(ANSI_GREEN "[CLIENT] " ANSI_RESET message "\n", __VA_ARGS__);
#define CLIENT_WARNING(warning_msg)                                                      \
  puts(ANSI_YELLOW "[CLIENT WARNING] " ANSI_RESET warning_msg);
#define CLIENT_ERROR(error_msg)                                                          \
  fputs(ANSI_RED "[CLIENT ERROR] " ANSI_RESET error_msg "\n", stderr);
#define CLIENT_ERRORF(error_msg, ...)                                                    \
  fprintf(stderr, ANSI_RED "[CLIENT ERROR] " ANSI_RESET error_msg "\n", __VA_ARGS__);

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fputs("usage: ./shairc <host> <port>", stderr);
    exit(EXIT_FAILURE);
  }

  // Disable raylib logging in favor of our own
  SetTraceLogLevel(LOG_NONE);

  const char *host = argv[1];
  const char *port = argv[2];

  // Connect to server, begin loop
  int sockfd;
  if ((sockfd = ConnectToServer(host, port)) == -1)
  {
    CLIENT_ERRORF("Failed to connect to %s:%s...", host, port);
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
    CLIENT_MESSAGE("Please enter an image filepath...")
    scanf("%s", imgPath);

    // Load image at that path, verifying it exists
    // Raylib image's data will be NULL if it failed to load

    memset(&img, 0, sizeof img); // Need to ensure is clean!

    img = LoadImage(imgPath);
    if (img.data == NULL)
    {
      CLIENT_ERRORF("Image %s does not exist, aborting...", imgPath);
      exit(EXIT_FAILURE);
    }

    // Convert the given image into 32-bit RGBA
    // This guarantees that the receiver will have 4 channels
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    if (AssembleImagePacket(&img, &packetBuf, &packetLen) != RC_SUCCESS)
    {
      CLIENT_ERROR("Failed to assemble image packet");
      exit(EXIT_FAILURE);
    }

    // Now that we're connected, send the image; return the status of this
    if (!SendAll(sockfd, (void *)packetBuf, packetLen))
    {
      CLIENT_ERROR("Failed to send image packet");
    }
    else
    {
      CLIENT_MESSAGE("Image packet send OK");
    }

    free(packetBuf);
  }

  exit(EXIT_SUCCESS);
}
