/* client.c - go-back-n client implementation in C
 * by Prateek Kumar Singh <20cs01003@iitbbs.ac.in>
 *
 * This implements a go-back-n client that implements reliable data
 * transfer over TCP using the go-back-n ARQ with variable chunk size
 *
 * for debug purposes, a loss rate can also be specified in the accompanying
 * server program
 * compile with "gcc -o client client.c"
 */

#include "gbnpacket.c"
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <errno.h>     /* for errno and EINTR */
#include <netinet/in.h>
#include <signal.h>     /* for sigaction() */
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <time.h>
#include <unistd.h>    /* for close() and alarm() */
#define TIMEOUT_SECS 4 /* Seconds between retransmits */
#define MAXTRIES 100   /* Tries before giving up */

int tries = 0; /* Count of times sent - GLOBAL for signal-handler access */
int base = 0;
int windowSize = 0;
int sendflag = 1;

void DieWithError(char *errorMessage); /* Error handling function */
void CatchAlarm(int ignored);          /* Handler for SIGALRM */
int max(int a, int b); /* macros that most compilers include - used for
                          calculating a few things */
int min(int a, int b); /* I think gcc includes them but this is to be safe */
char *gen_rdm_bytestream(size_t num_bytes) {
  char *stream = malloc(num_bytes);
  size_t i;

  for (i = 0; i < num_bytes; i++) {
    stream[i] = rand();
  }

  return stream;
}
int main(int argc, char *argv[]) {
  FILE *file;
  char *fileBuffer;
  long size;
  srand(time(NULL));
  file = fopen("example.txt", "rb");
  if (!file) {
    printf("Error opening file\n");
    return 1;
  }
  fseek(file, 0, SEEK_END);
  size = ftell(file);
  rewind(file);
  fileBuffer = (char *)malloc(sizeof(char) * (size + 1));
  if (!fileBuffer) {
    printf("Error allocating memory\n");
    fclose(file);
    return 1;
  }
  fileBuffer = gen_rdm_bytestream(100);
  fileBuffer[101] = '\0';
  // fread(fileBuffer, sizeof(char), size, file);

  // Close file
  fclose(file);

  // Print text array
  int sock; /* Socket descriptor */
  int df = 0;
  struct sockaddr_in gbnServAddr; /* Echo server address */
  struct sockaddr_in fromAddr;    /* Source address of echo */
  unsigned short gbnServPort;     /* Echo server port */
  unsigned int fromSize;          /* In-out of address size for recvfrom() */
  struct sigaction myAction;      /* For setting signal handler */
  char *servIP;                   /* IP address of server */
  int respLen;                    /* Size of received datagram */
  int packet_received = -1;       /* highest ack received */
  int packet_sent = -1;           /* highest packet sent */
  extern char buffer[];           /* buffer for data */
  buffer[201] = '\0';
  // const int datasize = 8192; /* data buffer size */
  const int datasize = 201; /* data buffer size */
  int chunkSize;            /* chunk size in bytes */
  int nPackets = 0;         /* number of packets to send */
  int totalTransmission = 0;

  if (argc != 5) /* Test for correct number of arguments */
  {
    fprintf(
        stderr,
        "Usage: %s <Server IP> <Server Port No> <Chunk size> <Window Size>\n",
        argv[0]);
    exit(1);
  }

  printf("string read from file : %s\n", buffer);
  servIP = argv[1];            /* First arg:  server IP address (dotted quad) */
  chunkSize = atoi(argv[3]);   /* Third arg: string to echo */
  gbnServPort = atoi(argv[2]); /* Use given port */
  windowSize = atoi(argv[4]);
  if (chunkSize >= 512) {
    fprintf(stderr, "chunk size must be less than 512\n");
    exit(1);
  }

  nPackets = datasize / chunkSize;
  if (datasize % chunkSize)
    nPackets++; /* if it doesn't divide cleanly, need one more odd-sized packet
                 */
  // nPackets--;
  /* Create a best-effort datagram socket using TCP */
  if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    DieWithError("socket() failed");
  printf("created socket");

  /* Set signal handler for alarm signal */
  myAction.sa_handler = CatchAlarm;
  if (sigfillset(&myAction.sa_mask) < 0) /* block everything in handler */
    DieWithError("sigfillset() failed");
  myAction.sa_flags = 0;

  if (sigaction(SIGALRM, &myAction, 0) < 0)
    DieWithError("sigaction() failed for SIGALRM");

  /* Construct the server address structure */
  memset(&gbnServAddr, 0, sizeof(gbnServAddr)); /* Zero out structure */
  gbnServAddr.sin_family = AF_INET;
  gbnServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
  gbnServAddr.sin_port = htons(gbnServPort);       /* Server port */

  /* Send the string to the server */
  while ((packet_received < nPackets - 1) && (tries < MAXTRIES)) {
    // printf ("in the send loop base %d packet_sent %d packet_received %d\n",
    //      base, packet_sent, packet_received);
    if (sendflag > 0) {
      sendflag = 0;
      int ctr; /*window size counter */
      for (ctr = 0; ctr < windowSize; ctr++) {
        packet_sent = min(max(base + ctr, packet_sent),
                          nPackets - 1); /* calc highest packet sent */
        struct gbnpacket currpacket;     /* current packet we're working with */
        if ((base + ctr) < nPackets) {
          memset(&currpacket, 0, sizeof(currpacket));
          printf("sending packet %d packet_sent %d packet_received %d\n",
                 base + ctr, packet_sent, packet_received);
          totalTransmission++;

          currpacket.type = htonl(1); /*convert to network endianness */
          currpacket.seq_no = htonl(base + ctr);
          int currlength;
          if ((datasize - ((base + ctr) * chunkSize)) >=
              chunkSize) /* length chunksize except last packet */
            currlength = chunkSize;
          else
            currlength = datasize % chunkSize;
          currpacket.length = htonl(currlength);
          memcpy(currpacket.data, /*copy buffer data into packet */
                 buffer + ((base + ctr) * chunkSize), currlength);
          if (sendto(sock, &currpacket, (sizeof(int) * 3) + currlength,
                     0, /* send packet */
                     (struct sockaddr *)&gbnServAddr,
                     sizeof(gbnServAddr)) != ((sizeof(int) * 3) + currlength))
            DieWithError(
                "sendto() sent a different number of bytes than expected");
        }
      }
    }
    /* Get a response */

    fromSize = sizeof(fromAddr);
    alarm(TIMEOUT_SECS); /* Set the timeout */
    struct gbnpacket currAck;
    while ((respLen = (recvfrom(sock, &currAck, sizeof(int) * 3, 0,
                                (struct sockaddr *)&fromAddr, &fromSize))) < 0)
      if (errno == EINTR) /* Alarm went off  */
      {
        if (tries < MAXTRIES) /* incremented by signal handler */
        {
          printf("timed out, %d more tries...\n", MAXTRIES - tries);
          break;
        } else
          DieWithError("No Response");
      } else
        DieWithError("recvfrom() failed");

    /* recvfrom() got something --  cancel the timeout */
    if (respLen) {
      int acktype = ntohl(currAck.type); /* convert to host byte order */
      int ackno = ntohl(currAck.seq_no);
      if (ackno > packet_received && acktype == 2) {
        printf("received ack\n"); /* receive/handle ack */
        packet_received++;
        base = packet_received;             /* handle new ack */
        if (packet_received == packet_sent) /* all sent packets acked */
        {
          alarm(0); /* clear alarm */
          tries = 0;
          sendflag = 1;
        } else /* not all sent packets acked */
        {
          tries = 0; /* reset retry counter */
          sendflag = 0;
          alarm(TIMEOUT_SECS); /* reset alarm */
        }
      }
    }
  }
  int ctr;
  for (ctr = 0; ctr < 10;
       ctr++) /* send 10 teardown packets - don't have to necessarily send 10
                 but spec said "up to 10" */
  {
    struct gbnpacket teardown;
    teardown.type = htonl(4);
    teardown.seq_no = htonl(0);
    teardown.length = htonl(0);
    sendto(sock, &teardown, (sizeof(int) * 3), 0,
           (struct sockaddr *)&gbnServAddr, sizeof(gbnServAddr));
  }
  printf("total transmissions : %d", totalTransmission);
  close(sock); /* close socket */
  exit(0);
}

void CatchAlarm(int ignored) /* Handler for SIGALRM */
{
  tries += 1;
  sendflag = 1;
}

void DieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(1);
}

int max(int a, int b) {
  if (b > a)
    return b;
  return a;
}

int min(int a, int b) {
  if (b > a)
    return a;
  return b;
}
