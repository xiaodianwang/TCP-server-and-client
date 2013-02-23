/* Stream client */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

//port that client will be connecting to
#define PORT "65432"
//max number of bytes that we can get at once
#define MAXDATASIZE 1000

//Get socket address, IPv4 or IPv6 (from Beej's guide)
//If the sa_family field is AF_INET(IPv4), return the IPv4 address, else return IPv6 otherwise.
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) { 
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    //declaring variables for settint up the socket and connection
    int sockfd, numbytes;
    struct addrinfo hints, *p, *servinfo;
    int rv;
    char s[INET6_ADDRSTRLEN];

    //declaring variables for file transfer later on
    int total_bytes_sent, bytes_left, n;
    char buffer[MAXDATASIZE];
    char *src_filename; 
    FILE *file_to_read;
    size_t m;
    unsigned char *p_buf = NULL;
    
    /*The byte stream client takes 3 arguments (argc = 3): 
     *argv[0] is the function call
     *argv[1] is the IP address that you want to connect to
     *argv[2] is the file that you are sending to the server*/
    if (argc != 3) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }
    //Error check to make sure inputting the right arguments
    //printf("argv[0] %s arg[1] %s arg[2] %s argc %d\n", argv[0], argv[1], argv[2], argc);
    
    //Loading the client's info
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    /*Get the address info of the IP address of the server you want to connect to, it returns a pointer (&servinfo) to a struct addrinfo for the server. Error (which generates a value other than 0) otherwise*/
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    /*Assign pointer p to servinfo, and then create a socket from the first node of the struct addrinfo linked list.*/
    p = servinfo;
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        perror ("unable to create socket for client");
    }
    
    /*Connect the socket to the remote server. Connect() returns 0 on success, and -1 on error. */
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror("Client: unable to connect");
        exit(1);
    }
    
    if (p == NULL) {
        fprintf(stderr, "Client: no connection, no server info given");
        return 2;
    }
    
    /*Print out the IP address of the remote server that the client is connecting to*/
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);
    printf("Client: connecting to %s\n", s);
    
    //open, read and transfer the file data
    file_to_read = fopen(argv[2], "r");
    if (file_to_read == NULL) {
        printf("Open file error: %s\n", argv[2]);
        exit(1);
    }
    /*Initialize the counter (total_bytes_sent) for the amount of bytes that the client reads&sends to the server. To make sure that the */
    total_bytes_sent = 0;
    
    while ((n = fread(buffer, 1, MAXDATASIZE, file_to_read)) != 0) {
        bytes_left = (int)n;
        printf("bytes left = %d: %s\n", bytes_left, buffer);
        total_bytes_sent = 0;
        p_buf = &buffer[0];
        while (bytes_left != 0) {
            m = send(sockfd, p_buf, (size_t)bytes_left, 0);
            printf("bytes sent %d\n", (int)m);
            bytes_left -= m;
            total_bytes_sent += m;
            p_buf += total_bytes_sent;
        }
        printf("Total bytes sent = %d\n", (int) n);
    }
    fclose(file_to_read);
    close(sockfd);
    return 0;
    
}