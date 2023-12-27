#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFSIZE 1000

int main(int argc, char *argv[]) {
  int sd; /* Socket Descriptor */
  int connected_sd; /* Connected Socket Descriptor */
  int rc; /* Return code from recvfrom */
  struct sockaddr_in server_address;
  struct sockaddr_in from_address; 
  socklen_t fromLength;
  int portNumber;
  char serverIP[29];
  char inputFileName[20];
  char buffer[1000];
  int fileSize;
  int sizeOfFileName;
  FILE *writeToFile;
  int numberOfBytes;

  if (argc < 2){
    printf ("Usage is: server <portNumber>\n");
    exit (1);
  }

  // Get port number from command line arguments
  portNumber = atoi(argv[1]);

  // Create a socket
  sd = socket(AF_INET, SOCK_STREAM, 0);
  
  fromLength = sizeof(struct sockaddr_in);
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(portNumber);
  server_address.sin_addr.s_addr = INADDR_ANY;

  // Bind the socket to the server address
  rc = bind(sd, (struct sockaddr *)&server_address, sizeof(server_address));
  if (rc < 0){
    perror("Bind");
    exit(1);
  }

  // Listen for incoming connections
  listen(sd, 5);

  // Accept a connection
  connected_sd = accept(sd, (struct sockaddr *) &from_address, &fromLength);
  memset(buffer, 0, 100);

  // Check for successful connection
  if (connected_sd < 0){
    perror("No Connection");
  }

  while(1){
    // Initialize variables for each file transfer
    strcpy(inputFileName, "");
    fileSize = 0;
    sizeOfFileName = 0;
    numberOfBytes = 0;

    // Read the size of the file name from the client
    rc = read(connected_sd, &sizeOfFileName, sizeof(int));
    if(rc == 0){
      break;
    }

    printf("read %d bytes to get the filesize\n", rc);
    printf("the size of the filename before converting is %d bytes\n", sizeOfFileName);

    // Convert size to host byte order
    sizeOfFileName = ntohs(sizeOfFileName);
    printf("the size of the filename after converting is %d bytes\n", sizeOfFileName);

    int totalBytes = 0;
    char *ptr = buffer;

    // Read the file name in chunks
    while (totalBytes < sizeOfFileName){
      rc = read(connected_sd, ptr, sizeOfFileName - totalBytes);
      printf("in loop, read %d bytes\n", rc);
      if (rc <= 0){
        perror("read");
        exit (1);
      }
      totalBytes += rc;
      ptr += rc;
    }

    printf("received the following '%s'\n", buffer);

    // Copy the received file name
    strcpy(inputFileName, buffer);

    // Read the file size from the client
    rc = read(connected_sd, &fileSize, sizeof(int));
    fileSize = ntohs(fileSize);

    printf("size of file is %d bytes\n", fileSize);
    totalBytes += rc;

    // Open the file for writing
    writeToFile = fopen(inputFileName, "wb");

    // Read the number of bytes to be received
    rc = read(connected_sd, &numberOfBytes, sizeof(int));
    totalBytes += rc;

    strcpy(buffer, "");

    // Read the file content in chunks
    rc = read(connected_sd, ptr, numberOfBytes);
    totalBytes += rc;

    // Write the received content to the file
    rc = fwrite(buffer, 1, BUFFSIZE, writeToFile);
    totalBytes += rc;

    // Send acknowledgment about the total bytes received back to the client
    rc = write(connected_sd, &totalBytes, sizeof(int));
    printf("read %d bytes\n", totalBytes);

    // Close the file
    fclose(writeToFile);
  }

  // Close the connected socket
  close(connected_sd);
  return 0;
}
