#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#define LOG_INFO(msg, ...) printf("[*] " __FILE__ ":%d (%s) :: " msg "\n", __LINE__, __func__, ##__VA_ARGS__);
#define LOG_ERROR(msg, ...) fprintf(stderr, "[!] " __FILE__ ":%d (%s) :: " msg "\n", __LINE__, __func__, ##__VA_ARGS__)

int wft_socket_create(void) {
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
    LOG_ERROR("unable to init Winsock (%d)", WSAGetLastError());
    exit(1);
  }
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (INVALID_SOCKET == (long long unsigned int) fd) {
    LOG_ERROR("unable to create socket (%d)", WSAGetLastError());
    WSACleanup();
    exit(1);
  }
  return fd;
#else
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == fd) {
    LOG_ERROR("unable to create socket (%s)", strerror(errno));
    exit(1);
  }
  return fd;
#endif
}

void wft_socket_bind(int fd, const struct sockaddr_in * restrict addr) {
#ifdef _WIN32
  if (SOCKET_ERROR != bind(fd, (const struct sockaddr *) addr, sizeof(*addr))) return;
  LOG_ERROR("unable to bind socket (%d)", WSAGetLastError());
  closesocket(fd);
  WSACleanup();
#else
  if (-1 != bind(fd, (const struct sockaddr *) addr, sizeof(*addr))) return;
  LOG_ERROR("unable to bind socket (%s)", strerror(errno));
  close(fd);
#endif
  exit(1);
}

void wft_socket_listen(int fd) {
#ifdef _WIN32
  if (SOCKET_ERROR != listen(fd, 3)) return;
  LOG_ERROR("unable to listen through socket (%d)", WSAGetLastError());
  closesocket(fd);
  WSACleanup();
#else
  if (-1 != listen(fd, 3)) return;
  LOG_ERROR("unable to listen through socket (%s)", strerror(errno));
  close(fd);
#endif
  exit(1);
}

void wft_socket_connect(int fd, const struct sockaddr_in * restrict addr) {
#ifdef _WIN32
  if (SOCKET_ERROR != connect(fd, (const struct sockaddr *) addr, sizeof(*addr))) return;
  LOG_ERROR("could not connect to server (%d)", WSAGetLastError());
  closesocket(fd);
  WSACleanup();
#else
  if (-1 != connect(fd, (const struct sockaddr *) addr, sizeof(*addr))) return;
  LOG_ERROR("could not connect to server");
  close(fd);
#endif
  exit(1);
}
