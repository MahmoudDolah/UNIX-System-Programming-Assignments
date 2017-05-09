#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdbool.h>

char *PROG_NAME;
bool DEBUG = false;

const int MAX_CLIENTS = 1;
const int MESSAGE_BUFSIZE = 4096;
const int USERNAME_BUFSIZE = 4096;

char *SEPARATOR = ": ";
size_t SEPARATOR_LENGTH = 2;

// Sentinel value to indicate an invalid or otherwise NULL file
// descriptor.
const int FD_NULL = -1;

// The set of valid exit values.
/*
enum EXIT_T {
  EXIT_NORMAL = 0,
  EXIT_ERROR_ARGUMENT,
  EXIT_ERROR_SOCKET,
  EXIT_ERROR_MEMORY,
  EXIT_ERROR_IO,
};
*/

// The socket to the remote server/client.
static int remoteSocket;

/* --------------------------------------------------------------------
Function declarations
-------------------------------------------------------------------- */

/*
  Interprets command line arguments.
  argc and argv are the # of command line arguments, and the array of
  command line arguments respectively.

  progName is the string pointer that should hold the executable's name.
  servermode indicates that the client should operate in server mode.
  debug corresponds to whether the user is requesting debug output.

  If there is an unexpected argument in argv, this will cause the
  program to *TERMINATE*.
*/
void parseArguments(
  int argc, char **argv,
  char **progName, char *username,
  struct sockaddr_in *socketAddress, bool *servermode, bool *debug);


/*
  Waits for a client to connect to us.

  socketAddress is the address where we should listen on.

  remotesocket is the file descriptor used to communicate to/from the
  remote client.
*/
void getClientConnection(struct sockaddr_in socketAddress, int *remotesocket);

/*
  Connect to the server.

  socketAddress is the remote server to connect to.

  remotesocket is the file descriptor used to communicate to/from the
  server.
*/
void connectToServer(struct sockaddr_in *socketAddress, int *remotesocket);

/*
  Close any remote connections before exiting.
*/
void closeRemoteConnection();

/*
  Writes a message to the specified file.

  Returns 0 on success, 1 on error.

*/
int writeToFile(int file, char *message, size_t chars);

/*
  Prints out the usage string for this program.
*/
void displayUsageString();


/* --------------------------------------------------------------------
Main
-------------------------------------------------------------------- */
int main(int argc, char **argv) {
  bool servermode = false;

  remoteSocket = FD_NULL;
  // We should close the remote connection so that the remote end does
  // not end up with a socket stuck in TIME_WAIT.
  atexit(closeRemoteConnection);

  // This is where the program will connect to/bind to.
  struct sockaddr_in socketAddress;
  socketAddress.sin_family = AF_INET;

  char *username = malloc(sizeof(char) * USERNAME_BUFSIZE);
  if (username == NULL) {
    perror(PROG_NAME);
    exit(1);
  }

  parseArguments(argc, argv, &PROG_NAME, username, &socketAddress, &servermode, &DEBUG);

  if (servermode) {
    getClientConnection(socketAddress, &remoteSocket);
  } else {
    connectToServer(&socketAddress, &remoteSocket);
  }

  // Read data from remote
  int chars;
  char *message = malloc(sizeof(char) * MESSAGE_BUFSIZE);
  if (message == NULL) {
    perror(PROG_NAME);
    exit(1);
  }

  // Permit enough space to fit the username and SEPARATOR (": ")
  size_t usernameLength = strlen(username);
  char *outmessage = malloc(sizeof(char) * (MESSAGE_BUFSIZE + usernameLength + SEPARATOR_LENGTH));
  if (outmessage == NULL) {
    perror(PROG_NAME);
    exit(1);
  }

  // We only need to copy over the username/separator to the output
  // string once, since all output strings will contain this leading
  // substring.
  strncpy(outmessage, username, usernameLength);
  strncpy(outmessage + usernameLength, SEPARATOR,
    SEPARATOR_LENGTH);

  fd_set dataSourceFds;
  int dataSourceFdsCount = remoteSocket + 1;

  // The file descriptors that we need to look at.
  int inputFds[] = {0, remoteSocket, FD_NULL};
  int outputFds[] = {remoteSocket, 1, FD_NULL};
  do {
    FD_ZERO(&dataSourceFds);
    FD_SET(0, &dataSourceFds);
    FD_SET(remoteSocket, &dataSourceFds);

    int selected = select(dataSourceFdsCount, &dataSourceFds, NULL, NULL, NULL);
    if (selected < 0) {
      perror(PROG_NAME);
      exit(1);
    }

    if (DEBUG) {
      fprintf(stdout, "Received input from %d source(s).\n", selected);
    }

    for (int *in = inputFds, *out = outputFds;
      *in != -1;
      ++in, ++out) {

      if (FD_ISSET(*in, &dataSourceFds)) {
        chars = read(*in, message, MESSAGE_BUFSIZE);
        message[chars] = '\0';
        if (*out != 1) {
          // Build the message to send to the remote socket.
          strncpy(outmessage + usernameLength + SEPARATOR_LENGTH,
            message, MESSAGE_BUFSIZE);

          size_t outmessageLength = usernameLength +
            SEPARATOR_LENGTH + chars;
          // Just for good measure, null terminate the string.
          outmessage[outmessageLength] = '\0';

          if (DEBUG) {
            fprintf(stderr, "Outgoing message: '%s'\n", outmessage);
          }

          if (writeToFile(*out, outmessage, outmessageLength) != 0) {
            perror(PROG_NAME);
            exit(1);
          }
        } else {
          if (writeToFile(*out, message, chars) != 0) {
            perror(PROG_NAME);
            exit(1);
          }
          fputs("\n", stdout);
        }
      }
    }
  } while (chars > 0);

  if (DEBUG) {
    fputs("Remote end closed.\n", stdout);
  }

  free(outmessage);
  free(message);
  return 0;
}

/* --------------------------------------------------------------------
Function definitions
-------------------------------------------------------------------- */
void parseArguments(
  int argc, char **argv,
  char **progName, char *username,
  struct sockaddr_in *socketAddress, bool *servermode, bool *debug) {

  *progName = *(argv++);

  // Skip the first item, since that points to the executable.
  for (--argc; argc > 0; --argc, ++argv) {
    if (strcmp(*argv, "--server") == 0) {
      *servermode = true;
    } else if (strcmp(*argv, "--debug") == 0) {
      *debug = true;
    } else {
      // This is not a valid option... maybe its an expected argument.
      break;
    }
  }

  // Did the user enter an IP address?
  if (argc == 3) {
    // User entered a pair of IP port values.
    struct in_addr ipv4Address;
    switch (inet_pton(AF_INET, *argv, &ipv4Address)) {
      case 1: // Success
        if (DEBUG) {
          fprintf(stdout, "Accepted interface: %s\n", *argv);
        }
        socketAddress->sin_addr = ipv4Address;
        break;
      case 0: // Not a valid IPv4 address
        fprintf(stderr, "%s: Not a valid address '%s'\n", *progName, *argv);
        displayUsageString();
        exit(1);
        break;
      case -1: // Socket-related error
        perror(PROG_NAME);
        exit(1);
    }

    // Consume the argument.
    --argc;
    ++argv;
  } else {
    // User only entered a port value.
    if (DEBUG) {
      fputs("Did not specify an interface. Listening on all interfaces.\n", stdout);
    }
    socketAddress->sin_addr.s_addr = INADDR_ANY;
  }

  if (*argv == NULL) {
    fputs("Expected a port number\n", stderr);
    displayUsageString();
    exit(1);
  }

  // Parse the port value.
  char *afterPort = *argv;
  int port = strtol(*argv, &afterPort, 10);

  // Make sure that we actually consumed a port number, and not a part
  // of the IP address.
  if (*afterPort != '\0') {
    fprintf(stderr, "Invalid port number: '%s'\n", *argv);
    displayUsageString();
    exit(1);
  }
  if (DEBUG) {
    fprintf(stdout, "Specified port: %d\n", port);
  }
  socketAddress->sin_port = htons(port);

  // Consume the argument.
  --argc;
  ++argv;

  if (*argv == NULL) {
    fputs("Expected username\n", stderr);
    displayUsageString();
    exit(1);
  }

  // Get the username.
  strncpy(username, *argv, USERNAME_BUFSIZE);

  // Consume the argument.
  --argc;
  ++argv;

  if (*argv != NULL) {
    fprintf(stderr, "%s: Unexpected argument '%s'", *progName, *argv);
    displayUsageString();
    exit(1);
  }

  if (DEBUG) {
    fprintf(stdout, "Username: %s\n", username);
  }

}


void getClientConnection(struct sockaddr_in socketAddress, int *remotesocket) {
  // Used only if we're in server mode. This is where we'll listen for
  // incoming connections.
  if (DEBUG) {
    fputs("Running in server mode.\n", stdout);
  }
  int serversocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serversocket < 0) {
    perror(PROG_NAME);
    exit(1);
  }

  socklen_t addrlen = sizeof(socketAddress);

  if (DEBUG) {
    fputs("Binding to socket.\n", stdout);
  }
  if (bind(serversocket, (struct sockaddr *)(&socketAddress), addrlen) != 0) {
    perror(PROG_NAME);
    exit(1);
  }

  if (listen(serversocket, MAX_CLIENTS) != 0) {
    perror(PROG_NAME);
    exit(1);
  }

  fputs("Waiting for connection from a host...\n", stdout);
  struct sockaddr remoteAddress;
  socklen_t remoteAddrLen;

  if ((*remotesocket = accept(serversocket, &remoteAddress, &remoteAddrLen)) == -1) {
    perror(PROG_NAME);
    exit(1);
  }

  if (DEBUG) {
    fputs("Connection complete.\n", stdout);
  }

  // Got a connection, exit.
  close(serversocket);
}

int writeToFile(int file, char *message, size_t chars) {
  ssize_t bytesWritten = 0;
  while ((bytesWritten = write(file, message + bytesWritten, chars - bytesWritten))) {
    if (bytesWritten < 0) {
      return 1;
    }
  }
  return 0;
}

void connectToServer(struct sockaddr_in *socketAddress, int *remotesocket) {
  // Used only if we're in client mode. Connect to the remote server.
  if (DEBUG) {
    fputs("Running in client mode.\n", stdout);
  }

  *remotesocket = socket(AF_INET, SOCK_STREAM, 0);
  if (*remotesocket < 0) {
    perror(PROG_NAME);
    exit(1);
  }

  socklen_t addrlen = sizeof(*socketAddress);

  if (DEBUG) {
    fputs("Connecting to server...\n", stdout);
  }

  if (connect(*remotesocket, (struct sockaddr *)(socketAddress), addrlen) != 0) {
    perror(PROG_NAME);
    exit(1);
  }

  if (DEBUG) {
    fputs("Server connected.\n", stdout);
  }
}

void closeRemoteConnection() {
  if (remoteSocket != FD_NULL) {
    if (close(remoteSocket) != 0) {
      perror(PROG_NAME);
    }
  }
}

void displayUsageString() {
  fputs("Usage:\n\
    client [--server] [--debug] [interface] port username\n", stdout);
}
