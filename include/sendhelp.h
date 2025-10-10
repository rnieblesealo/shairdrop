/**
 * @brief Headers related to network helpers
 */

#ifndef SEND_HELP_H
#define SEND_HELP_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

/**
 * @brief Sends all data in a buffer; retries send() until all data is through
 * @param fd The socket file descriptor we wish to send data through
 * @param buf The buffer where the data we want to send is
 * @param len The size of the data in the buffer, in bytes
 * @returns true if all data went through, false otherwise
 */
bool SendAll(int fd, void *buf, size_t len);

/**
 * @brief Receives all data from a socket; retries recv() until it is all
 * through
 * @param fd The socket file descriptor we wish to send data through
 * @param buf The buffer where the data we want to send is
 * @param len The size of the data in the buffer, in bytes
 * @returns true if all data went through, false otherwise
 */
bool ReceiveAll(int fd, void *buf, size_t len);

#endif
