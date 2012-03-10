/**
 *
 * http://www.linuxhowtos.org/C_C++/socket.htm
 */
#include <cstdlib>
#include <iostream>
#include <cerrno>
#include <fstream>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/* rapidjson */
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
/* for sockets */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* bzero is legacy, so define */
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
/*
 * For each number in range, test if perfect number
 */
std::vector<unsigned long> brute_perfect(unsigned long start, unsigned long end) {
    std::vector<unsigned long> perfect_nums;

    for (unsigned long testnum = start; testnum < end; ++testnum) {
        unsigned long factor_sum = 0;
        for (unsigned long k = 1; k < testnum; ++k) {
            if (testnum % k == 0) { // is k a factor of testnum?
                factor_sum += k;
            }
        }
        // does testnum meet the definition of perf. num?
        if (factor_sum == testnum) {
            perfect_nums.push_back(testnum);
        }
    }
    return perfect_nums;
}
/* print vector to console: for debugging. */
void print_vector(std::vector<unsigned long> temp) {
    std::cout << "arr: ";
    for (int i = 0; i < (int) temp.size(); ++i) {
        std::cout << temp.at(i) << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
/*    print_vector(brute_perfect(1, 9589));
    
    rapidjson::Reader reader;
	char readBuffer[65536];
    rapidjson::FileReadStream is(stdin, readBuffer, sizeof(readBuffer));
*/
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memmove((char *)&serv_addr.sin_addr.s_addr,
            (char *)server->h_addr, 
            server->h_length);

    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) { 
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }

    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
        perror("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
        perror("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}

