#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "wft.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    LOG_ERROR("usage: %s [ADDR] [PORT] [FILE]", argv[0]);
    return 1;
  }
  const char *server_ip = argv[1];
  const int server_port = atoi(argv[2]);
  const char *filename = argv[3];
  int fd = wft_socket_create();
  if (-1 == fd) return 1;
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(server_port)
  };
  if (0 >= inet_pton(AF_INET, server_ip, &addr.sin_addr)) {
    LOG_ERROR("invalid IP address");
    close(fd);
    return 1;
  }
  if (!wft_socket_connect(fd, &addr)) return 1;
  LOG_INFO("Connected to %s:%d", server_ip, server_port);
  send(fd, filename, strlen(filename), 0);
  FILE *file_fd = fopen(filename, "wb");
  if (!file_fd) {
    LOG_ERROR("unable to open file (`%s`)", filename);
    close(fd);
    return 1;
  }
  char *buf = malloc(GIGABYTE * sizeof(char));
  memset(buf, 0, GIGABYTE * sizeof(char));
  int bytes_recv;
  while (-1 != (bytes_recv = recv(fd, buf, GIGABYTE, 0))) {
    LOG_INFO("bytes_recv = %d", bytes_recv);
    fwrite(buf, 1, bytes_recv, file_fd);
    break;
  }
  LOG_INFO("File (`%s`) received successfully", filename);
  free(buf);
  fclose(file_fd);
  close(fd);
  return 0;
}
