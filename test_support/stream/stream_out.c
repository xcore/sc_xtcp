#ifndef __WIN32__
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

int data[1460/4];

int main(int argc, char *argv[]) {
  unsigned long addr;
  SOCKET s;
  struct sockaddr_in sa;
  int i;

#ifdef __WIN32__
  {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
  }
#endif

  if (argc < 3) {
    puts("Please provide IP address in dotted decimal form");
    exit(0);
  }

  sa.sin_family = AF_INET;
  sa.sin_port = htons(atoi(argv[2]));
  sa.sin_addr.s_addr = inet_addr(argv[1]);
  if (sa.sin_addr.s_addr == INADDR_NONE) {
    puts("Invalid IP address");
    exit(0);
  }

  // Open a socket
  //s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s == INVALID_SOCKET) {
    puts("Unable to open socket");
    printf("%d\n", WSAGetLastError());
    exit(0);
  }

  if (connect(s, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) != 0) {
    puts("Unable to connect");
    exit(0);
  }

  for (i=0; i<1000000; ++i) {
    data[0] = i;
    send(s, data, sizeof(data), 0);
  }

  return 0;
}
