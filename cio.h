#ifndef C_IO
#define C_IO

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#ifdef __linux__
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>
#endif

#define CIO_BUFSIZE 2<<16

// cio handle
#define CIOH(c) ((cio_t*)c)

#define C_SOCKADDR(addr) (struct sockaddr*)addr

enum CIO_ADDRTYPE {
  IPv4,
  IPv6
};

/*
 * SECTION X
 *  > RAW
 *  > TCP
 *  > UDP
 *  > ICMP
 * */
enum CIO_SOCKETTYPE {
  /*
   * SECTION RAW
   * */
  RAW = 0x01,

  /*
   * SECTION IPv4
   * */
  IP4 = 0x10,
  TCP4,
  TCP4_RAW,
  UDP4,
  UDP4_RAW,

  /*
   * SECTION IPv6
   * */
  IP6 = 0x30,
  TCP6,
  TCP6_RAW,
  UDP6,
  UDP6_RAW,

  CIO_END = 0xff
};

enum CIO_PROTOCOL {
  PROTO_RAW,
  PROTO_TCP,
  PROTO_UDP,
};

typedef struct {
  enum CIO_ADDRTYPE addr_type;
  enum CIO_PROTOCOL protocol;
  const char* addr;
  unsigned short port;
  struct sockaddr_storage sa;
  unsigned int addr_len;
} cio_socket_addr_t;

typedef struct {
  int fd;
} cio_t;

typedef struct {
  cio_t cio;
  cio_socket_addr_t sockaddr;
} cio_socket_t;

typedef struct {
  enum CIO_SOCKETTYPE type;
  const char* addr;
  unsigned short port;
  int listen_backlog;
  int async;
  int __domain;
  int __type;
  int __protocol;
} cio_socket_desc_t;

typedef struct {
  cio_socket_desc_t src;
  cio_socket_desc_t dst;
} cio_socket_connect_desc_t;

cio_socket_t cio_socket(cio_socket_desc_t desc);
cio_socket_t cio_connect(cio_socket_connect_desc_t dest);
int cio_accept(cio_socket_t socket, char* buf, int *len);

void cio_close(cio_t *cio);

void cio_send();
void cio_send_to();

void cio_recv();
void cio_recv_from();

cio_t cio_file();

static cio_socket_t cio_build_socket(cio_socket_desc_t socket_desc);
static void cio_build_sockaddr(cio_socket_addr_t *socket_addr);

#endif
