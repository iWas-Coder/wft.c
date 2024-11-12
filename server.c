#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "wft.h"

#define PORT 6969

static char *buf = 0;

void handle_client(int fd) {
  memset(buf, 0, GIGABYTE * sizeof(char));
  char filename[1024] = {0};
  if (-1 == read(fd, filename, 1024)) {
    LOG_ERROR("unable to read requested file's name");
    close(fd);
    return;
  }
  LOG_INFO("Requested file `%s`", filename);
  FILE *file_fd = fopen(filename, "rb");
  if (!fd) {
    LOG_ERROR("unable to open file (`%s`)", filename);
    close(fd);
    return;
  }
  while (!feof(file_fd)) {
    size_t bytes_read = fread(buf, 1, GIGABYTE, file_fd);
    if (bytes_read) send(fd, buf, bytes_read, 0);
  }
  LOG_INFO("File `%s` sent successfully", filename);
  fclose(file_fd);
  close(fd);
}

int main(void) {
  int fd = wft_socket_create();
  if (-1 == fd) return 1;
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = INADDR_ANY,
    .sin_port = htons(PORT)
  };
  socklen_t addr_len = sizeof(addr);
  if (!wft_socket_bind(fd, &addr)) return 1;
  if (!wft_socket_listen(fd)) return 1;
  LOG_INFO("Server listening on port %d", PORT);
  buf = malloc(GIGABYTE * sizeof(char));
  int client_fd;
  while (-1 != (client_fd = accept(fd, (struct sockaddr *) &addr, &addr_len))) {
    handle_client(client_fd);
  }
  free(buf);
  return 0;
}
