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

/**
 * @brief Response codes for all protocol operations
 */
enum SHAIRRC : uint8_t
{
  /**
   * @brief The operation was successful
   */
  RC_SUCCESS = 0,

  /**
   * @brief An image of excessive size was received for an operation
   */
  RC_IMG_TOO_LARGE_ERROR,

  /**
   * @brief An error ocurred with address resolution
   */
  RC_GAI_ERROR,

  /**
   * @brief A socket error ocurred
   */
  RC_SOCKET_ERROR,

  /**
   * @brief A connection error ocurred
   */
  RC_CONNECTION_ERROR,

  /**
   * @brief An error ocurred when sending data
   */
  RC_SEND_ERROR,

  /**
   * @brief An error ocurred when receiving data 
   */
  RC_RECEIVE_ERROR,

  /**
   * @brief Packet size failed to be received or is otherwise malformed
   */
  RC_BAD_PACKET_SIZE_ERROR,
};

// ========================================================================
// Packeting Functions
// ========================================================================

/**
 * @brief Builds a packet out of the data in a raylib image
 * @param image The image to build packet from
 * @param packetBuf Pointer to the packet buffer to write image to
 * @param packetLen Pointer to the packet size
 * @returns Successful response code if OK, specific error code on failure
 */
enum SHAIRRC AssembleImagePacket(Image *image, uint8_t **packetBuf, size_t *packetLen);

/**
 * @brief Sends an image packet to a server
 * @param packet Buffer where packet is stored
 * @param packetLen Size of buffer where packet is stored
 * @param host Hostname of target server
 * @param port Port of target server
 * @returns Successful response code if OK, specific error code on failure
 */
enum SHAIRRC SendImagePacket(const uint8_t *packet,
                             size_t         packetLen,
                             const char    *host,
                             const char    *port);

/**
 * @brief Decodes an image packet into a Raylib Image
 * @note Opcode should be decoded separately! This operation pertains to OPC_RECEIVE_IMG
 * @param img The image object we'd like to write our received image to
 * @param clientsockfd fd of the client we would like to receive image from
 * @returns Successful response code if OK, specific error code on failure
 */
enum SHAIRRC DecodeImagePacket(Image *img, int clientsockfd);

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
 * @brief Establish connection to SHAirDrop server
 * @param host Server hostname
 * @param port Server port
 * @returns The fd of the connection if successful, or -1 on error
 */
int ConnectToServer(const char *host, const char *port);

/**
 * @brief Accepts a connection on the given listener file descriptor
 * @param sockfd The listener file descriptor
 * @returns The accepted connection's file descriptor, or -1 on error
 */
int HearOutAMothafucka(int sockfd);

#endif
