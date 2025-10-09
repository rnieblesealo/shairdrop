#include "sendhelp.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>

bool SendAll(int fd, void *buf, size_t len) {
  const uint8_t *startByte =
      (const uint8_t *)buf; // Start byte in buffer should never change
  size_t bytesSent = 0;

  while (bytesSent < len) {

    /*
     * Compute start byte of remaining send
     * Adjust the remaining length to match!
     * This will happen on a send retry
     */

    ssize_t n = send(fd, startByte + bytesSent, len - bytesSent, 0);

    if (n == -1) {

      /*
       * We only retry on EINTR, why?
       *
       * EINTR = This is interrupted before data is sent
       * EINTR is the only signal that doesn't say the other end is in trouble
       * Thus only if we receive this may we continue
       * Otherwise, crash and burn
       * See "man 2 send" for other sigs in case we wish to handle them diff :D
       *
       */

      if (errno == EINTR) {
        continue;
      }

      return false;
    }

    bytesSent += (ssize_t)n;
  }

  return true;
}

bool ReceiveAll(int fd, void *buf, size_t len) {
  uint8_t *startByte = (uint8_t *)buf;
  size_t bytesReceived = 0;

  while (bytesReceived < len) {
    ssize_t n = recv(fd, startByte + bytesReceived, len - bytesReceived, 0);

    // If we get 0 bytes, the peer is likely closed; nothing we can do from this
    // end
    if (n == 0) {
      return false;
    }

    if (n < 0) {
      perror("server: recvall");

      if (errno == EINTR) {
        continue;
      }

      return false;
    }

    bytesReceived += (size_t)n;
  }

  return (ssize_t)bytesReceived;
}
