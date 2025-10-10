/**
 * @brief Headers related to the SHAirDrop protocol
 */

#ifndef SHAIRDROP_H
#define SHAIRDROP_H

// ========================================================================
// Includes
// ========================================================================

#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ========================================================================
// Constants
// ========================================================================

#define BACKLOG 3
#define MAX_IMG_WIDTH 64
#define MAX_IMG_HEIGHT 64
#define MAX_IMG_SIZE (MAX_IMG_WIDTH * MAX_IMG_HEIGHT * RGBA_CHANNEL_COUNT)
#define RGBA_CHANNEL_COUNT 4

// ========================================================================
// Enumerations
// ========================================================================

/**
 * @brief Opcodes for various packets
 */
enum OPCODES : uint8_t
{
  /**
   * @brief Instructs to receive an image
   */
  OPC_RECEIVE_IMG = 0x01
};

// ========================================================================
// Packeting Functions
// ========================================================================

/**
 * @brief Builds a packet out of the data in a raylib image
 * @param image The image to build packet from
 * @param packetBuf Pointer to the packet buffer to write image to
 * @param packetLen Pointer to the packet size
 * @returns Pointer to the packet
 */
bool AssembleImagePacket(Image *image, uint8_t **packetBuf, size_t *packetLen);

/**
 * @brief Sends an image packet to a server
 * @param packet Buffer where packet is stored
 * @param packetLen Size of buffer where packet is stored
 * @param host Hostname of target server
 * @param port Port of target server
 */
bool SendImagePacket(const uint8_t *packet,
                     size_t         packetLen,
                     const char    *host,
                     const char    *port);

// ========================================================================
// Server Handler Functions
// ========================================================================

/**
 * @brief Starts a listener server
 * @param host The server's hostname
 * @param port The server's port
 * @returns The listener file descriptor if OK, or -1 on error
 */
int FireUpTheServer(const char *host, const char *port);

/**
 * @brief Accepts a connection on the given listener file descriptor
 * @param sockfd The listener file descriptor
 * @returns The accepted connection's file descriptor, or -1 on error
 */
int HearOutAMothafucka(int sockfd);

/**
 * @brief Receives an image packet, writing it into a Raylib image if successful
 * @param img The image object we'd like to write our received image to
 * @param clientsockfd fd of the client we would like to receive image from
 * @returns true if the operation succeeded, false otherwise
 */
bool GetHisFuckingPicture(Image *img, int clientsockfd);

#endif
