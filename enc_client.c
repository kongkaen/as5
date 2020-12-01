/**
* Natthaphong Kongkaew
* kongkaen@oregonstate.edu
* Assignment 5: One Time Pad
* CS 344 Fall_2020 Oregonstate University
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

#define SIZE 150000

/**
* Encryption Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Read input from file and send that input as a message to the server.
* 3. Save the message received from the server to a file and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber,
                        char* hostname){

  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname);
  if (hostInfo == NULL) {
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(0);
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr,
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {

  // Initialize variables
  int port_number = atoi(argv[3]);
  char hostname[] = "localhost";
  int socketFD, charsRead, charsWritten, i;
  FILE *fp;
  struct sockaddr_in serverAddress;

  // Initialize buffer
  char buffer[SIZE];
  char key_buffer[SIZE];
  char text_buffer[SIZE];
  char cipher_buffer[SIZE];
  char recv_buffer[SIZE];

  // Clear out the buffer array
  memset(buffer, '\0', sizeof(buffer));
  memset(key_buffer, '\0', sizeof(key_buffer));
  memset(text_buffer, '\0', sizeof(text_buffer));
  memset(cipher_buffer, '\0', sizeof(cipher_buffer));
  memset(recv_buffer, '\0', sizeof(recv_buffer));

  // Check for correct arguments
  if (argc != 4) {
     fprintf(stderr,"Usage %s input_file key_file port_number\n", argv[0]);
     exit(0);
   }

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

  // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), hostname);

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  // send authentication for server to connect to only enc_client
  char verify_client[] = "enc_client";
  charsWritten =  send(socketFD, verify_client, strlen(verify_client), 0);

  // Check if the message was sent to the server
  if (charsWritten < strlen(verify_client))
  {
      fprintf(stderr,  "CLIENT: ERROR Could not send text to the server\n");
      exit(1);
  }

  // Clear authentication buffer
  memset(verify_client, '\0', sizeof(verify_client));

  // Read message fron server
  charsRead = recv(socketFD, verify_client, sizeof(buffer) - 1, 0);

  // Check if the message was received from the server
  if (charsRead < 0)
  {
      fprintf(stderr, "CLIENT: ERROR: Could not receive text from the server\n");
      exit(1);
  }

  // Compare the message to verify with the server
  if(strcmp(verify_client, "enc_client") != 0)
  {
      fprintf(stderr, "CLIENT: ERROR only enc_client can be connected to the server\n");
      exit(2);
  }

  // Get key and key size
  int key = open(argv[2], O_RDONLY);
  int keySize = read(key, key_buffer, sizeof(key_buffer) - 1);
  // Remove the trailing \n
  key_buffer[strcspn(key_buffer, "\n")] = '\0';
  keySize--;

  // Get message and size
  int text = open(argv[1], O_RDONLY);
  int textSize = read(text, text_buffer, sizeof(text_buffer) - 1);
  // Remove the trailing \n
  text_buffer[strcspn(text_buffer, "\n")] = '\0';
  textSize--;

  // Check if the plain text has invalid characters
  for (i = 0; i < textSize - 1; i++) {
      if (isspace(text_buffer[i]) || isalpha(text_buffer[i])) {
      }
      else {
        fprintf(stderr,"input contains bad characters\n");
        exit(1);
      }

  }

  // Check if key size big enough for message size
  if (textSize > keySize) {
      fprintf(stderr, "'%s' is too short", argv[2]);
      exit(1);
  }

  // Send the plaintext to server
    textSize = send(socketFD, text_buffer, strlen(text_buffer), 0);
    if (textSize < 0)
    {
        fprintf(stderr,  "CLIENT: ERROR could not send text\n");
        exit(1);
    }


    //Receive message from server
    charsRead = recv(socketFD, recv_buffer, sizeof(recv_buffer), 0);
    if (charsRead < 0)
    {
        fprintf(stderr,  "CLIENT: ERROR could not receive text\n");
        exit(1);
    }

    // Send key to server
    keySize = send(socketFD, key_buffer, strlen(key_buffer), 0);
    if (keySize < 0)
    {
        fprintf(stderr,  "CLIENT: ERROR could not send text\n");
        exit(1);
    }

  // Receive ciphered text from the server
  //memset(cipher_buffer, '\0', sizeof(cipher_buffer));
  charsRead = recv(socketFD, cipher_buffer, sizeof(cipher_buffer) - 1, 0);

  if (charsRead < 0)
    {
      error("CLIENT: ERROR reading from socket - 1");
    }

  // Print cipher text
  printf("%s\n", cipher_buffer);

  // Close socket
  close(socketFD);

  return 0;
}
