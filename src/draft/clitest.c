#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>

#define BUFF_SIZE 1024

int main(int argc, int argv[]){
  char buffer[BUFF_SIZE];
  int bytes;
  struct addrinfo hints;
  struct addrinfo *results, *rp;
  int sfd, s;
  struct sockaddr_in sin;
  char* ret;

  char *hostname = "localhost";
  char *port = "8080";

  /* create socket */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if(s == -1){
    printf("socket\n");
    return EXIT_FAILURE;
  }

  /* connect to server */
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;      /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;  /* Stream socket */
  hints.ai_flags = 0;
  hints.ai_protocol = 0;            /* Any protocol */

  s = getaddrinfo(hostname, port, &hints, &results);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return EXIT_FAILURE;
  }

  /* getaddrinfo() returns a list of address structures.
     Try each address until we successfully connect(2).
     If socket(2) (or connect(2)) fails, we (close the socket
     and) try the next address. */

  for (rp = results; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype,
        rp->ai_protocol);
    if (sfd == -1)
      continue;

    if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
      break;                  /* Success */

    close(sfd);
  }

  if (rp == NULL) {               /* No address succeeded */
    fprintf(stderr, "Could not connect\n");
    freeaddrinfo(results);
    return EXIT_FAILURE;
  }

  freeaddrinfo(results);

  for(;;){
    ret = fgets(buffer, BUFF_SIZE, stdin);
    if(!ret)
      return EXIT_FAILURE;

    bytes = fputs(buffer, stdout);
    if(bytes < 0)
      return EXIT_FAILURE;

    if(write(sfd, buffer, strlen(buffer)) < 0)
      return EXIT_FAILURE;

    if(bytes = read(sfd, buffer, BUFF_SIZE) < 0)
      return EXIT_FAILURE;

  }


  return EXIT_SUCCESS;
}
