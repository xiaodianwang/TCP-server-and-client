/*Stream server*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

//port users will connect to, try to use port # [49151 - 65535]
#define PORT "65432" 
//backlog: how many pending connections queue will hold
#define BACKLOG 10
//max number of bytes that we can get at once
#define MAXDATASIZE 1000

//Function to get socket address, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void) {
    int sockfd, new_fd; //listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; //connector's address information
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv,n,m;
    unsigned char buffer[MAXDATASIZE];
    char file_name_w[64];
    int counter = 0; //counter for separately naming files written for each new connection
    FILE *file_to_write; //file handle
    int total_bytes_read = 0;
    size_t num_from_remote;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; //use my IP address
        
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    p = servinfo;
    //point to fields of 1st record in servinfo
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { 
        perror ("unable to create socket for server");
    }
    //allows for reuse of port immediately as soon as service exits
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror ("Unable to bind socket to port");
    }
    
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    
    if (listen(sockfd, BACKLOG) == -1) {
        perror("Listen error");
        exit(1);
    }
    
    printf("server: waiting for connections...\n");
    
    while(1) { //main accept loop
        sin_size = sizeof their_addr;
        new_fd = accept (sockfd, (struct sockaddr *)&their_addr, &sin_size);
        

        
        if (new_fd > 0) {
            sprintf(file_name_w, "file_to_write_%d", counter++);
            file_to_write = fopen(file_name_w, "a");

            total_bytes_read = 0;
            
            while ((num_from_remote = recv(new_fd, buffer, MAXDATASIZE - 1, 0)) != 0) {
                buffer[num_from_remote] = '\0';
                printf("read %d from socket: %s\n", (int)num_from_remote, buffer);

                total_bytes_read += fwrite(buffer, 1, num_from_remote, file_to_write);
            }
            fflush(file_to_write);
            fclose(file_to_write);
            close(new_fd);
            printf ("Total bytes read: %d", total_bytes_read);
        } else {
            perror("Accept socket error");
        }
        
        
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);        
    }
    close(new_fd);
    return 0;
}

