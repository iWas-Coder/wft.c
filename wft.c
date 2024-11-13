#include "wft.h"

static char *buf = 0;
static size_t buf_sz = (1 << 30) * sizeof(char);

void wft_get_file_from_server(const char *ip, const int port, const char *file) {
  int fd = wft_socket_create();
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(port)
  };
  if (0 >= inet_pton(AF_INET, ip, &addr.sin_addr)) {
    LOG_ERROR("invalid IP address");
    close(fd);
    exit(1);
  }
  wft_socket_connect(fd, &addr);
  LOG_INFO("Connected to %s:%d", ip, port);
  send(fd, file, strlen(file), 0);
  FILE *file_fd = fopen(file, "wb");
  if (!file_fd) {
    LOG_ERROR("unable to open file (`%s`)", file);
    close(fd);
    exit(1);
  }
  buf = malloc(buf_sz);
  memset(buf, 0, buf_sz);
  int bytes_recv = 0;
  while (-1 != (bytes_recv = recv(fd, buf, buf_sz, 0))) {
    LOG_INFO("bytes_recv = %d", bytes_recv);
    fwrite(buf, 1, bytes_recv, file_fd);
    break;
  }
  LOG_INFO("File (`%s`) received successfully", file);
  free(buf);
  fclose(file_fd);
  close(fd);
}

void wft_serve_handle_client(int fd) {
  memset(buf, 0, buf_sz);
  char file[1024] = {0};
  if (-1 == read(fd, file, 1024)) {
    LOG_ERROR("unable to read requested file's name");
    close(fd);
    return;
  }
  LOG_INFO("Requested file `%s`", file);
  FILE *file_fd = fopen(file, "rb");
  if (!fd) {
    LOG_ERROR("unable to open file (`%s`)", file);
    close(fd);
    return;
  }
  while (!feof(file_fd)) {
    size_t bytes_read = fread(buf, 1, buf_sz, file_fd);
    if (bytes_read) send(fd, buf, bytes_read, 0);
  }
  LOG_INFO("File `%s` sent successfully", file);
  fclose(file_fd);
  close(fd);
}

void wft_serve_dir(const int port) {
  int fd = wft_socket_create();
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = INADDR_ANY,
    .sin_port = htons(port)
  };
  socklen_t addr_len = sizeof(addr);
  wft_socket_bind(fd, &addr);
  wft_socket_listen(fd);
  LOG_INFO("Server listening on port %d", port);
  buf = malloc(buf_sz);
  int client_fd = 0;
  while (-1 != (client_fd = accept(fd, (struct sockaddr *) &addr, &addr_len))) {
    wft_serve_handle_client(client_fd);
  }
  free(buf);
}

int main(int argc, char **argv) {
  if (argc < 2) goto defer;
  if (!strcmp(argv[1], "get")) {
    if (argc != 5) {
      LOG_ERROR("usage: %s get [IP] [PORT] [FILE]", argv[0]);
      return 1;
    }
    wft_get_file_from_server(argv[2], atoi(argv[3]), argv[4]);
    return 0;
  }
  if (!strcmp(argv[1], "serve")) {
    if (argc != 3) {
      LOG_ERROR("usage: %s serve [PORT]", argv[0]);
      return 1;
    }
    wft_serve_dir(atoi(argv[2]));
    return 0;
  }
 defer:
  LOG_ERROR("usage: %s [get|serve]", argv[0]);
  return 1;
}
