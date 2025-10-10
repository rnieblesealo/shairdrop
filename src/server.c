#include "raylib.h"
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
