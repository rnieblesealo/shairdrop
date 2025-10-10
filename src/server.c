#include "raylib.h"
#include "shairdrop.h"
#include <netdb.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500

// ========================================================================
// Shared State (Will make you cry) (Update: It didn't because I'm him >:D)
// ========================================================================

Image           xImg        = {0};
Texture2D       xImgTexture = {0};
bool            xImgInited  = false;
bool            xTexDirty   = false;
pthread_mutex_t xImgLock    = PTHREAD_MUTEX_INITIALIZER;

// ========================================================================
// Structures
// ========================================================================

typedef struct NetArgs
{
  int servsockfd;
} NetArgs;

// ========================================================================
// Function Definitions
// ========================================================================

void *NetworkThread(void *arg)
{
  NetArgs *args = (NetArgs *)arg;

  int clientsockfd = HearOutAMothafucka(args->servsockfd);

  pthread_mutex_lock(&xImgLock);

  if (!GetHisFuckingPicture(&xImg, clientsockfd))
  {
    fputs("server: unable to get picture\n", stderr);

    pthread_mutex_unlock(&xImgLock);
    pthread_exit(NULL);
  }

  xImgInited = true;
  xTexDirty  = true;

  puts("Image load OK!\n");

  pthread_mutex_unlock(&xImgLock);

  return NULL;
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

  const char *host = argv[1];
  const char *port = argv[2];

  int sockfd = FireUpTheServer(host, port);

  // ====================================================================================
  // Network Thread Start
  // ====================================================================================

  pthread_t networkThread;
  NetArgs   networkThreadArgs = {.servsockfd = sockfd};
  int       rc;
  if ((rc = pthread_create(&networkThread, NULL, NetworkThread, &networkThreadArgs)) != 0)
  {
    fprintf(stderr, "server: pthread_create error, rc: %d\n", rc);
    exit(EXIT_FAILURE);
  }

  // ====================================================================================
  // Render Thread Start
  // ====================================================================================

  while (!WindowShouldClose())
  {
    BeginDrawing();

    ClearBackground(BLUE);

    float drawScale = 10;
    float drawRot   = 0;

    if (xImgInited)
    {
      pthread_mutex_lock(&xImgLock);

      if (xTexDirty)
      {
        xImgTexture = LoadTextureFromImage(xImg);
        xTexDirty   = false;
      }

      Vector2 drawPos = {
          .x = (float)SCREEN_WIDTH / 2 - (float)xImgTexture.width * drawScale / 2,
          .y = (float)SCREEN_HEIGHT / 2 - (float)xImgTexture.height * drawScale / 2};

      DrawTextureEx(xImgTexture, drawPos, drawRot, drawScale, WHITE);

      pthread_mutex_unlock(&xImgLock);
    }

    EndDrawing();
  }

  // ====================================================================================
  // Deinitialization
  // ====================================================================================

  pthread_mutex_lock(&xImgLock);

  UnloadTexture(xImgTexture);
  UnloadImage(xImg);

  pthread_mutex_unlock(&xImgLock);

  CloseWindow();
}
