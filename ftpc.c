#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFSIZE 1000

// Function to get the length of a file
int getFileLength(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    int len = (int) ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return len;
}

// Function to read a file and send its content to the server
int readfile(FILE *inputFile, int sd){
  unsigned char buffer[BUFFSIZE];
  int numberOfBytes;
  int rc;
  int totalBytesRead = 0, totalBytesWritten = 0;

  // Read the file content in chunks
  numberOfBytes = fread(buffer, 1, BUFFSIZE, inputFile);

  while(numberOfBytes > 0){
    totalBytesRead += numberOfBytes;

    // Send the number of bytes to the server
    rc = write(sd, &numberOfBytes, sizeof(int));
    // Send the file content to the server
    rc = write(sd, buffer, numberOfBytes);

    // Check for errors in writing to the server
    if(numberOfBytes != rc){
      perror("writing to file");
      exit(1);
    }

    totalBytesWritten += rc;
    numberOfBytes = fread(buffer, 1, BUFFSIZE, inputFile);
  }

  // Print the total bytes read and written
  printf("read %d bytes, and wrote %d bytes\n", totalBytesRead, totalBytesWritten);

  return (0);
}

int main(int argc, char *argv[]) {
  FILE *inputFile, *outputFile;
  int sd; // Socket Descriptor
  struct sockaddr_in server_address;
  int rc;
  int portNumber;
  char serverIP[29];
  char inputFileName[20];
  int bytesReceived;

  // Check if the correct number of command line arguments is provided
  if (argc < 3){
    printf("usage is client <ipaddr> <port>\n");
  }

  // Create a socket
  sd = socket(AF_INET, SOCK_STREAM, 0);
  
  // Extract port number and server IP from command line arguments
  portNumber = atoi(argv[2]);
  strcpy(serverIP, argv[1]);
  
  // Set up the server address
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(portNumber);
  server_address.sin_addr.s_addr = inet_addr(serverIP);

  // Connect to the server
  if(connect(sd, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) < 0){
    close(sd);
    perror("error connecting stream socket");
    exit(1);
  }
  
  // Main loop for file transfer
  while(1){
    printf("\n");
    printf("Enter the file name: ");
    scanf("%s", inputFileName);

    // Exit loop if user inputs "DONE"
    if(strcmp(inputFileName, "DONE")==0){
      break;
    }

    // Get the size of the file name
    int sizeOfFileName = strlen(inputFileName);
    int converted_sizeOfFileName = ntohs(sizeOfFileName);
    
    // Send the size of the file name to the server
    rc = write(sd, &converted_sizeOfFileName, sizeof(converted_sizeOfFileName));
    printf("wrote %d bytes to send the filename size\n", rc);
    if (rc < 0){
      perror("write");
    }

    // Send the file name to the server
    rc = write(sd, inputFileName, sizeOfFileName);
    printf("wrote %d bytes to send the filename\n", rc);
    if (rc < 0){
      perror("write");
    }

    // Open the file for reading in binary mode
    inputFile = fopen(inputFileName, "rb");
    
    // Get the size of the file
    int fileSize = getFileLength(inputFile);
    int converted_fileSize = ntohs(fileSize);

    // Send the size of the file to the server
    rc = write(sd, &converted_fileSize, sizeof(converted_fileSize));
    printf("wrote %d bytes to send the filesize\n", rc);
    if (rc < 0){
      perror("write");
    }

    // Read the file and send its content to the server
    readfile(inputFile, sd);

    // Read acknowledgment from the server about the bytes received
    rc = read(sd, &bytesReceived, sizeof(int));
    printf("Server received %d bytes", bytesReceived);
  }

  // Close the client socket
  close(sd);
  return 0;
}
