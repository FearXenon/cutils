#include "cio.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>


static void cio_build_sockaddr(cio_socket_addr_t *sockaddr) {
  memset(&sockaddr->sa, 0, sizeof(sockaddr->sa));
  int len = 0;

  if (sockaddr->addr_type == IPv4) {
    struct sockaddr_in *sa4 = (struct sockaddr_in*)&sockaddr->sa;
  
    if (inet_pton(AF_INET, sockaddr->addr, &sa4->sin_addr) == 1) {
      sa4->sin_family = AF_INET;
      sa4->sin_port = htons(sockaddr->port);
    } else {
      perror("invalid ipv4 address");
    }

    len = sizeof(struct sockaddr_in);
  } else if (sockaddr->addr_type == IPv6) {
    struct sockaddr_in6 *sa6 = (struct sockaddr_in6*)&sockaddr->sa;

    if (inet_pton(AF_INET6, sockaddr->addr, &sa6->sin6_addr) == 1) {
      sa6->sin6_family = AF_INET6;
      sa6->sin6_port = htons(sockaddr->port);
    }

    len = sizeof(struct sockaddr_in6);
  } 

  if (len == 0) {
    perror("address creation failed");
  }
  sockaddr->addr_len = len;
}

static cio_socket_t cio_build_socket(cio_socket_desc_t desc) {
  cio_socket_t __cio_socket = {};

  int domain, type, protocol = 0;

  if (desc.type < IP4 && desc.type >= RAW) {
    domain = AF_PACKET; 
  } else if (desc.type < IP6) {
    domain = AF_INET;
  } else if (desc.type < CIO_END) {
    domain = AF_INET6; 
  }

  switch (desc.type) {
    // RAW
    case RAW:
    case IP4:
    case IP6:
      type = SOCK_RAW;
      break;
    case TCP4_RAW:
    case TCP6_RAW:
      type = SOCK_RAW;
      protocol = IPPROTO_TCP;
      break;
    case UDP4_RAW:
    case UDP6_RAW:
      type = SOCK_RAW;
      protocol = IPPROTO_UDP;
      break;
    // TCP
    case TCP4:
    case TCP6:
      type = SOCK_STREAM;
      protocol = IPPROTO_TCP;
      break;
    // UDP
    case UDP4:
    case UDP6:
      type = SOCK_DGRAM;
      protocol = IPPROTO_UDP;
      break;
    default:
      break;
  };

  if (domain == 0) {
    domain = desc.__domain;
    type = desc.__type;
    protocol = desc.__protocol;
  }  

  int sfd = socket(domain, type, protocol);
  __cio_socket.cio.fd = sfd;

  if (__cio_socket.cio.fd == -1)
    perror("socket initialization failed");

  __cio_socket.sockaddr.addr_type = domain == AF_INET ? IPv4 : IPv6;
  __cio_socket.sockaddr.protocol = PROTO_UDP;
  __cio_socket.sockaddr.addr = desc.addr;
  __cio_socket.sockaddr.port = desc.port;
  cio_build_sockaddr(&__cio_socket.sockaddr);

  return __cio_socket;
}

cio_socket_t cio_socket(cio_socket_desc_t desc) {
  cio_socket_t __cio_socket;

  __cio_socket = cio_build_socket(desc);

  int opt = 1;
  if (setsockopt(__cio_socket.cio.fd, 1, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("sockopt");
    cio_close(CIOH(&__cio_socket));
  }

  if (bind(__cio_socket.cio.fd, (struct sockaddr*)&__cio_socket.sockaddr.sa, __cio_socket.sockaddr.addr_len) != 0) {
    perror("could not bind to socket");
  }

  if (__cio_socket.sockaddr.protocol == PROTO_TCP) {
    listen(__cio_socket.cio.fd, desc.listen_backlog ?: 5);
  }

  return __cio_socket;
}

int cio_accept(cio_socket_t socket, char* buf, int* len) {
  int fd;

  cio_socket_addr_t client_addr = {};
  client_addr.addr_len = sizeof(struct sockaddr_in);

  if (socket.sockaddr.protocol == PROTO_TCP) {
    while ((fd = accept(socket.cio.fd, C_SOCKADDR(&client_addr), &client_addr.addr_len))) {

      char client_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &((struct sockaddr_in*)&client_addr)->sin_addr, client_ip, sizeof(client_ip));
      int client_port = ntohs(((struct sockaddr_in*)&client_addr)->sin_port);

      printf("Client connected: %s:%d\n", client_ip, client_port);
      while (recv(fd, buf, *len, 0) > 0) {
        printf("%s", buf);
      }
      printf("Client disconnected: %s:%d\n", client_ip, client_port);
    } 
  } else if (socket.sockaddr.protocol == PROTO_UDP) {
    ssize_t recv_len = recvfrom(socket.cio.fd, buf, *len, 0, C_SOCKADDR(&client_addr), &client_addr.addr_len); 
  }

  return 1;
}

void cio_close(cio_t *cio) {
  close(cio->fd);
}
