/**
* Natthaphong Kongkaew
* kongkaen@oregonstate.edu
* Assignment 5: One Time Pad
* CS 344 Fall_2020 Oregonstate University
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>

#define SIZE 150000
/**
* Encryption Server code
* 1. Create a socket and connect to the enc_client.
* 2. Verify enc_client.
* 3. Receive plaintext and key from enc_client
* 4. Encrypt plaintext
* 5. Send encrypted text back to enc_client.
*/

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber){

  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

// Convert character(char) to integer(int)
int charToInt (char ch){
	if (ch == ' '){
		return 26;
	}
	else {
		return (ch - 'A');
	}
	return 0;
}

// Convert integer(int) to character(char)
char intToChar(int num){
	if (num == 26){
		return ' ';
	}
	else {
		return (num + 'A');
	}
}

// Encrypt the text using One-Time-Pad
void encryptText(char plainText[], char keyBuffer[], int length) {
  int i;
  char ch;
  // Loop through plaintext
  for (i=0; i<length;i++) {
    // convert plain text char to integer
    // convert key char to integer
    // add plainText integer and key integer
    // mod by 27
    ch = (charToInt(plainText[i]) + charToInt(keyBuffer[i])) % 27;

    // convert back to char
    plainText[i] = intToChar(ch);
  }
}

int main(int argc, char *argv[])  {

  // Initialize variables
  pid_t pid;
  int connectionSocket, charsRead, charsWritten, i;
  int value = 1;
  int port = atoi(argv[1]);
  int key_size = 0;
  int text_size = 0;

  char buffer[SIZE];
  memset(buffer, '\0', sizeof(buffer));
  char key_buffer[SIZE];
  memset(key_buffer, '\0', sizeof(key_buffer));

  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) {
    fprintf(stderr,"USAGE: %s port\n", argv[0]);
    exit(1);
  }

  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));
  // Set the socket options
  setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));

  // Associate the socket to the port
  if (bind(listenSocket,
          (struct sockaddr *)&serverAddress,
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5);

  while (1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket,(struct sockaddr *)&clientAddress,
                        &sizeOfClientInfo);
    if (connectionSocket < 0){
          error("ERROR on accept");
    }

    // Fork
    pid = fork();


    // Check to make sure fork was succesful
    if (pid < 0)
    {
        printf("ERROR: Fork failed \n");
        exit(1);
    }

    // Child process created

    if (pid == 0)
    {

        // Clear the buffer
        memset(buffer, '\0', SIZE);

        // Receive verify text message from client
        int charsRead = recv(connectionSocket, buffer, 1000, 0);
        if (charsRead < 0)
        {
            fprintf(stderr,  "SERVER: ERROR could not receive 'verification message'\n");
            exit(1);
        }

        // Verify if message match the message sent from enc_client
        if (strcmp(buffer, "enc_client") != 0)
        {
            exit(2);
        }

        // If verify successful, send message back to client
        else
        {
            charsWritten = send(connectionSocket, buffer, strlen(buffer), 0);

            if (charsWritten < 0)
            {
                fprintf(stderr,  "SERVER: ERROR could not send 'verified text'\n");
                exit(1);
            }
        }

        // Clear buffer
        memset(buffer, '\0', sizeof(buffer));

        // Receive plaintext from client
        text_size = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);
        if (text_size < 0)
        {
            fprintf(stderr,  "SERVER: ERROR could not receive 'Plaintext'\n");
            exit(1);
        }

        // Send message to client
        send(connectionSocket, "Plain text received", 19, 0);

        // Receive key from client
        key_size = recv(connectionSocket, key_buffer, sizeof(key_buffer) - 1, 0);
        if (key_size < 0)
        {
            fprintf(stderr,  "SERVER: ERROR could not receive 'key'\n");
            exit(1);
        }

        // Encrypt the plaintext
        encryptText(buffer, key_buffer, text_size);

        // Send encrypted text back to the client
        int charsWritten = send(connectionSocket, buffer, strlen(buffer), 0);
        if (charsWritten < 0)
        {
            fprintf(stderr,  "SERVER: ERROR could not send 'encrypted text'\n");
            exit(1);
        }

        // Close the connection socket
        close(connectionSocket);
        // Close the listening socket
        close(listenSocket);
        return 0;
    }
    // Close the connection socket for this client
    else close(connectionSocket);

  }
    return 0;
}
