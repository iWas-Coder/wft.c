#include "common.h"

#ifdef _WIN32
#define WFT_FILENAME_MAX_LEN 256
#else
#define WFT_FILENAME_MAX_LEN 4096
#endif

static unsigned char *buf = 0;
static size_t buf_sz = 1 << 30;

void wft_get_directory_listing(const char *ip, const int port) {
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
  // TODO: implement `ls` client command functionality
  LOG_ERROR("Not implemented yet");
  close(fd);
}

void wft_get_file_from_server(const char *ip, const int port, const char *file) {
  int fd = wft_socket_create();
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(port)
  };
  if (0 >= inet_pton(AF_INET, ip, &addr.sin_addr)) {
    LOG_ERROR("invalid IP address");
    goto defer;
  }
  wft_socket_connect(fd, &addr);
  LOG_INFO("Connected to %s:%d", ip, port);
  LOG_INFO("--> %zd (`%s`)", write(fd, file, strlen(file)), file);
  shutdown(fd, SHUT_WR);
  FILE *ffd = fopen(file, "wb");
  if (!ffd) {
    LOG_ERROR("unable to open file (`%s`)", file);
    goto defer;
  }
  buf = calloc(buf_sz, sizeof(unsigned char));
  ssize_t bytes_read = 0;
  while (0 < (bytes_read = read(fd, buf, buf_sz))) {
    LOG_INFO("<-- %zd", bytes_read);
    fwrite(buf, 1, bytes_read, ffd);
  }
  // TODO: check file contents integrity with CRC32
  shutdown(fd, SHUT_RD);
  free(buf);
  fclose(ffd);
  close(fd);
  return;
 defer:
  close(fd);
  exit(1);
}

void wft_serve_handle_client(int fd) {
  char file[WFT_FILENAME_MAX_LEN] = {0};
  LOG_INFO("<-- %zd (`%s`)", read(fd, file, WFT_FILENAME_MAX_LEN), file);
  shutdown(fd, SHUT_RD);
  FILE *ffd = fopen(file, "rb");
  if (!fd) {
    LOG_ERROR("unable to open file (`%s`)", file);
    goto defer;
  }
  memset(buf, 0, buf_sz);
  while (!feof(ffd)) {
    size_t bytes_read = fread(buf, 1, buf_sz, ffd);
    if (bytes_read) LOG_INFO("--> %zd", write(fd, buf, bytes_read));
  }
  // TODO: check file contents integrity with CRC32
  shutdown(fd, SHUT_WR);
  fclose(ffd);
 defer:
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
  if (!strcmp(argv[1], "ls")) {
    if (argc != 4) {
      LOG_ERROR("usage: %s ls [IP] [PORT]", argv[0]);
      return 1;
    }
    wft_get_directory_listing(argv[2], atoi(argv[3]));
    return 0;
  }
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
