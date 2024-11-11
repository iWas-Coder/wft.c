#pragma once

#include <stdio.h>

#define LOG_INFO(msg, ...) printf("[*] " __FILE__ ":%d (%s) :: " msg "\n", __LINE__, __func__, ##__VA_ARGS__);
#define LOG_ERROR(msg, ...) fprintf(stderr, "[!] " __FILE__ ":%d (%s) :: " msg "\n", __LINE__, __func__, ##__VA_ARGS__)

#define false 0
#define true 1
#define GIGABYTE (1024*1024*1024)

typedef _Bool bool;

int wft_socket_create(void) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == fd) LOG_ERROR("unable to create socket (%s)", strerror(errno));
  return fd;
}

bool wft_socket_bind(int fd, const struct sockaddr_in * restrict addr) {
  if (-1 == bind(fd, (const struct sockaddr *) addr, sizeof(*addr))) {
    LOG_ERROR("unable to bind socket (%s)", strerror(errno));
    close(fd);
    return false;
  }
  return true;
}

bool wft_socket_listen(int fd) {
  if (-1 == listen(fd, 3)) {
    LOG_ERROR("unable to listen through socket (%s)", strerror(errno));
    close(fd);
    return false;
  }
  return true;
}

bool wft_socket_connect(int fd, const struct sockaddr_in * restrict addr) {
  if (-1 == connect(fd, (const struct sockaddr *) addr, sizeof(*addr))) {
    LOG_ERROR("could not connect to server");
    close(fd);
    return false;
  }
  return true;
}
