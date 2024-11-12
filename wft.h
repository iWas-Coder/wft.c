#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LOG_INFO(msg, ...) printf("[*] " __FILE__ ":%d (%s) :: " msg "\n", __LINE__, __func__, ##__VA_ARGS__);
#define LOG_ERROR(msg, ...) fprintf(stderr, "[!] " __FILE__ ":%d (%s) :: " msg "\n", __LINE__, __func__, ##__VA_ARGS__)

#define GIGABYTE (1024*1024*1024)

int wft_socket_create(void) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == fd) {
    LOG_ERROR("unable to create socket (%s)", strerror(errno));
    exit(1);
  }
  return fd;
}

void wft_socket_bind(int fd, const struct sockaddr_in * restrict addr) {
  if (-1 != bind(fd, (const struct sockaddr *) addr, sizeof(*addr))) return;
  LOG_ERROR("unable to bind socket (%s)", strerror(errno));
  close(fd);
  exit(1);
}

void wft_socket_listen(int fd) {
  if (-1 != listen(fd, 3)) return;
  LOG_ERROR("unable to listen through socket (%s)", strerror(errno));
  close(fd);
  exit(1);
}

void wft_socket_connect(int fd, const struct sockaddr_in * restrict addr) {
  if (-1 != connect(fd, (const struct sockaddr *) addr, sizeof(*addr))) return;
  LOG_ERROR("could not connect to server");
  close(fd);
  exit(1);
}
