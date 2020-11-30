#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

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

int charToInt (char c){
	if (c == ' '){
		return 26;
	}
	else {
		return (c - 'A');
	}
	return 0;
}

char intToChar(int i){
	if (i == 26){
		return ' ';
	}
	else {
		return (i + 'A');
	}
}

void enc(char message[], char key[], int message_length){

  int i, ms[]={0}, ky[]={0}, ms_ky[]={0};
  char n;
  for (i = 0; i< message_length ; i++){
      ms[i] = charToInt(message[i]);
      ky[i] = charToInt(key[i]);
      ms_ky[i] = ms[i] + ky[i];

      printf("(%c + %c) ---> (%d + %d) = %d\n", message[i], key[i], ms[i], ky[i], ms_ky[i]);

  }
  //message[i] = '\0';
  //return message;

  //printf("test");
}

int main(int argc, char *argv[])  {

  //pid_t pid, sid; //process id

  int connectionSocket, charsRead, status;
  char buffer[200001];
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

  // Associate the socket to the port
  if (bind(listenSocket,
          (struct sockaddr *)&serverAddress,
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5);

  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket,
                (struct sockaddr *)&clientAddress,
                &sizeOfClientInfo);
    if (connectionSocket < 0){
      error("ERROR on accept");
    }

    printf("SERVER: Connected to client running at host %d port %d\n",
                          ntohs(clientAddress.sin_addr.s_addr),
                          ntohs(clientAddress.sin_port));

    // Get the message from the client and display it
    memset(buffer, '\0', 200001);
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, 200001, 0);
    if (charsRead < 0){
      error("ERROR reading from socket");
    }

    // Separate key and message
    // Store key in key_buffer
    char key_buffer[100000];
    memset(key_buffer, '\0', 100000);

    // Store message in message_buffer
    char message_buffer[100000];
    memset(message_buffer, '\0', 100000);

    // "$" as the delimiter
    char * token = strtok(buffer, "$");
    strcpy(key_buffer,token);
    token = strtok(NULL, "$");
    strcpy(message_buffer,token);

    int message_length = strlen(message_buffer);

    // int array to store number converted from message
    int int_message[message_length];
    int int_key[message_length];
    int int_added[message_length];
    int int_result[message_length];

    for (int i=0;i<message_length;i++){
      int_message[i] = message_buffer[i] - 'A';
      //printf("%d(%c) + ", int_message[i], message_buffer[i]);
      int_key[i] = key_buffer[i] - 'A';
      //printf("%d(%c) ", int_key[i], key_buffer[i]);
      int_added[i] = int_message[i] + int_key[i];
      //printf(" = %d ", int_added[i]);
      if (int_added[i] >= 27){
          int_result[i] = int_added[i] % 27;
          //printf("mod 27");
      }
      else if (int_added[i] < 0 || int_added[i] > 25) {
        int_result[i] = int_key[i];
      }

      else {
        int_result[i] = int_added[i];
      }
      //printf(" = %d\n",int_result[i]);
    }

    // convert back to character array
    char char_message[message_length];
    memset(char_message, '\0', message_length);
    printf("\n");

    for (int i=0; i<message_length; i++ ){
      char_message[i] = int_result[i]+'A';
      //printf("%c",char_message[i] );
    }

    printf("%s",char_message);


    //printf("--Integer message %d ", )
/*
    int arr[message_length];
    for (int i=0; i<message_length; i++){
      arr[i]='\0';
    }

    for (int i=0; i<message_length; i++){
      arr[i] = message_buffer[i] - 'A';

      //printf("%c ", arr[i]);
    }
    for (int i=0; i< message_length; i++)
      printf("%d ", arr[i]);
*/
    //printf("\n-----SERVER: I received this from the client: \"%d\"\n", arr[7]);


    //printf("\nSERVER: I received this from the client: \"%s\"\n", key_buffer);
    //printf("SERVER: I received this from the client: \"%s\"\n", message_buffer);



    //printf("\nSERVER: I received this from the client: \"%s\"\n", buffer);

    //int test = message_buffer[4] - 'A';

    //printf("%c is %d \n", message_buffer[4], test);
    //enc(message_buffer, key_buffer,message_length);

    // Send a Success message back to the client
    charsRead = send(connectionSocket,
                    "I am the server, and I got your message", 39, 0);
    if (charsRead < 0){
      error("ERROR writing to socket");
    }
    // Close the connection socket for this client
    close(connectionSocket);
  }
  // Close the listening socket
  close(listenSocket);

  return 0;

}
