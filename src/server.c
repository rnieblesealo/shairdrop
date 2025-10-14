#include "ansihelp.h"
#include "raylib.h"
#include "sendhelp.h"
#include "shairdrop.h"
#include <netdb.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500
#define MAX_CLIENT_CONNS 2

// Message & error macros
// No need to worry about newlines!
#define SERVER_MESSAGE(message) puts(ANSI_GREEN "[SERVER] " ANSI_RESET message);
#define SERVER_MESSAGEF(message, ...)                                                    \
  printf(ANSI_GREEN "[SERVER] " ANSI_RESET message "\n", __VA_ARGS__);
#define SERVER_WARNING(warning_msg)                                                      \
  puts(ANSI_YELLOW "[SERVER WARNING] " ANSI_RESET warning_msg);
#define SERVER_ERROR(error_msg)                                                          \
  fputs(ANSI_RED "[SERVER ERROR] " ANSI_RESET error_msg "\n", stderr);
#define SERVER_ERRORF(error_msg, ...)                                                    \
  fprintf(stderr, ANSI_RED "[SERVER ERROR]" ANSI_RESET error_msg "\n", __VA_ARGS__);

// ========================================================================
// Structures
// ========================================================================

typedef struct NetworkThreadArgs
{
  int servsockfd;
} NetworkThreadArgs;

typedef struct ClientThreadArgs
{
  int clientsockfd;
} ClientThreadArgs;

typedef struct ClientThreadState
{
  bool connected;
} ClientThreadState;

// ========================================================================
// Shared State
// ========================================================================

Image           xImg        = {0};
Texture2D       xImgTexture = {0};
bool            xImgInited  = false;
bool            xTexDirty   = false;
pthread_mutex_t xLock       = PTHREAD_MUTEX_INITIALIZER;

// Thread data
pthread_t         xClientThreads[MAX_CLIENT_CONNS];
ClientThreadState xClientThreadState[MAX_CLIENT_CONNS];
uint32_t          xClientThreadsTail = 0;

// ========================================================================
// Function Definitions
// ========================================================================

void *ClientThread(void *arg)
{
  ClientThreadArgs *args = (ClientThreadArgs *)arg;

  bool connected = true;
  while (connected)
  {
    SERVER_MESSAGE("Client thread waiting for image...");

    // Get the opcode
    // If it pertains to image reception, then do that

    uint8_t opcode;
    if (!ReceiveAll(args->clientsockfd, &opcode, sizeof opcode))
    {
      SERVER_WARNING("Client thread received invalid packet, terminating it...");
      pthread_exit(NULL);
    }

    if (opcode != OPC_RECEIVE_IMG)
    {
      SERVER_ERRORF("Bad opcode (%d)", opcode);
      continue;
    }

    pthread_mutex_lock(&xLock);

    if (DecodeImagePacket(&xImg, args->clientsockfd) != RC_SUCCESS)
    {
      SERVER_ERROR("Unable to decode image packet, terminating client thread...")
      pthread_mutex_unlock(&xLock);
      pthread_exit(NULL);
    }

    if (!xImgInited)
    {
      xImgInited = true;
    }

    xTexDirty = true;

    SERVER_MESSAGE("Client thread image load successful!");
    pthread_mutex_unlock(&xLock);
  }

  pthread_exit(NULL);
}

void *NetworkThread(void *arg)
{
  NetworkThreadArgs *args = (NetworkThreadArgs *)arg;

  bool connected = true;
  while (connected)
  {
    // Listen for new connections

    int connsockfd;
    if ((connsockfd = HearOutAMothafucka(args->servsockfd)) == -1)
    {
      SERVER_ERROR("Received a connection, but failed to accept");
      continue;
    }

    pthread_mutex_lock(&xLock);

    // Upon receiving one, create a thread for it alongside its state

    ClientThreadArgs cThreadArgs = {.clientsockfd = connsockfd};

    int rc;
    if ((rc = pthread_create(
             &xClientThreads[xClientThreadsTail], NULL, ClientThread, &cThreadArgs)) != 0)
    {
      SERVER_ERRORF("Failed to create thread for connection socket %d", connsockfd);
      pthread_mutex_unlock(&xLock);
      continue;
    }

    xClientThreadState[xClientThreadsTail].connected = true;

    xClientThreadsTail++;

    SERVER_MESSAGEF("Created thread for socket %d", connsockfd);

    pthread_mutex_unlock(&xLock);
  }

  pthread_exit(NULL);
}

// ========================================================================
// MAIN
// ========================================================================

int main(int argc, char *argv[])
{
  /*
   * 1. Start raylib, server
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

  // ====================================================================================
  // Initialization
  // ====================================================================================

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SHAirDrop");
  SetTargetFPS(60);
  SetTraceLogLevel(LOG_NONE); // Disable raylib logging in favor of our own

  const char *host = argv[1];
  const char *port = argv[2];

  // Clean up thread data
  memset(&xClientThreads, 0, sizeof xClientThreads);

  int sockfd;
  if ((sockfd = FireUpTheServer(host, port)) == -1)
  {
    SERVER_ERROR("Failed to start server");
    exit(EXIT_FAILURE);
  }

  SERVER_MESSAGEF("Listening on %s:%s...", host, port);

  // ====================================================================================
  // Network Thread Start
  // ====================================================================================

  pthread_t         networkThread;
  NetworkThreadArgs networkThreadArgs = {.servsockfd = sockfd};
  int               rc;
  if ((rc = pthread_create(&networkThread, NULL, NetworkThread, &networkThreadArgs)) != 0)
  {
    SERVER_ERROR("Failed to create network thread, (RC %d); exiting...");
    exit(EXIT_FAILURE);
  }

  // ====================================================================================
  // Render Thread Start
  // ====================================================================================

  while (!WindowShouldClose())
  {
    BeginDrawing();

    ClearBackground(BLUE);

    float drawScale = 20;
    float drawRot   = 0;

    if (xImgInited)
    {
      pthread_mutex_lock(&xLock);

      if (xTexDirty)
      {
        xImgTexture = LoadTextureFromImage(xImg);
        xTexDirty   = false;
      }

      Vector2 drawPos = {
          .x = (float)SCREEN_WIDTH / 2 - (float)xImgTexture.width * drawScale / 2,
          .y = (float)SCREEN_HEIGHT / 2 - (float)xImgTexture.height * drawScale / 2};

      DrawTextureEx(xImgTexture, drawPos, drawRot, drawScale, WHITE);

      pthread_mutex_unlock(&xLock);
    }

    EndDrawing();
  }

  // ====================================================================================
  // Deinitialization
  // ====================================================================================

  pthread_mutex_lock(&xLock);

  UnloadTexture(xImgTexture);
  UnloadImage(xImg);

  pthread_mutex_unlock(&xLock);

  CloseWindow();
}
