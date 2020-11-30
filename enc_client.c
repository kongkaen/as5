#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

/**
* Client code
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

long find_length(char file_name[])
{
    // Open file in read mode
    FILE* fp = fopen(file_name, "r");

    // Check if the file exist
    if (fp == NULL) {
        printf("File Not Found!\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);

    // Get the size of the file
    long res = ftell(fp);

    // closing the file
    fclose(fp);

    return res;
}

char* readFile(char *file_name, long file_length){

  FILE *fp;
  fp = fopen(file_name, "r"); //open for readonly file specified in command line

  if(fp == NULL) {
    perror("Error opening file");
    exit(1);
  }

   char *buffer;
   buffer = (char*)malloc(file_length * sizeof(char));


   if( fgets (buffer, file_length, fp)!=NULL ) {
      /* writing content to stdout */
      //puts(buffer);
   }
   fclose(fp);

   //printf("buffer is %s", buffer);
   return buffer;
   free(buffer);


}

int main(int argc, char *argv[]) {

  int port_number = atoi(argv[3]);
  char hostname[] = "localhost";

  int socketFD, charsRead, charsWritten;
  FILE *fp;

  struct sockaddr_in serverAddress;

  char buffer[100000];
  // Clear out the buffer array
  memset(buffer, '\0', sizeof(buffer));

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

  // Get key length
  long key_length = find_length(argv[2]);

  //printf("key length is %ld\n", key_length);

  //  Get input file length
  long input_length = find_length(argv[1]);

  // Check if the key is long enough for the message
  if (input_length > key_length){
     fprintf(stderr, "'%s' is too short", argv[2]);
     exit(1);
  }

  // Check if the plain text has invalid characters
  int plain_text_fd = open(argv[1], 'r');
    while (read(plain_text_fd, buffer, 1) != 0) {
        if (buffer[0] != ' ' && (buffer[0] < 'A' || buffer[0] > 'Z')) {

            if (buffer[0] != '\n') {
                fprintf(stderr,"input contains bad characters");
                exit(EXIT_FAILURE);
              }
            }
    }

  // Clear out the buffer array
  memset(buffer, '\0', sizeof(buffer));


  // Copy key from key file to key buffer
  // keygen program doesnt generate new line, so +1 to the length
  char* key_buffer;
  key_buffer = (char*)malloc(key_length+1 * sizeof(char));
  key_buffer = readFile(argv[2], key_length+1);
  //printf(" buffer is %s", key_buffer);

  // Send message to server
  // Write to the server
  charsWritten = send(socketFD, key_buffer, strlen(key_buffer), 0);
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(key_buffer)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }

  // Sending a delimiter to separate between message and key
  char delim[] = "$";
  charsWritten = send(socketFD, delim, strlen(delim), 0);
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(delim)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }


  // Copy input message to buffer
  char* input_buffer;
  input_buffer = (char*)malloc(input_length * sizeof(char));
  input_buffer = readFile(argv[1], input_length);
  //printf(" buffer is %s", input_buffer);

  // Send message to server
  // Write to the server
  charsWritten = send(socketFD, input_buffer, strlen(input_buffer), 0);
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(input_buffer)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }


  // Get return message from server
  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }

  //printf("%s\n",buffer);

  // Close the socket
  close(socketFD);

  return 0;
}
