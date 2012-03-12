/**
 *
 * http://www.linuxhowtos.org/C_C++/socket.htm
 */
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cerrno>
#include <fstream>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/* for benchmark */
#include <time.h>
#include <unistd.h>
/* rapidjson */
#include "rapidjson/document.h"		// rapidjson's DOM-style API
#include "rapidjson/prettywriter.h"	// for stringify JSON
#include "rapidjson/filestream.h"	// wrapper of C stream for prettywriter as output
#include "rapidjson/filereadstream.h"
#include "rapidjson/reader.h"
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

double test_speed() {
    time_t start;
    time_t theend;
    int sum;
    start = clock();
    for(int i = 0; i < INT_MAX; ++i) {
        sum += i;
    }
    theend = clock();
    return (((double) (theend - start)) / (double) CLOCKS_PER_SEC);
}

/* print vector to console: for debugging. */
void print_vector(std::vector<unsigned long> temp) {
    std::cout << "arr: ";
    for (int i = 0; i < (int) temp.size(); ++i) {
        std::cout << temp.at(i) << " ";
    }
    std::cout << std::endl;
}

class SendRecv { 
    std::string host;
    int port;
    SendRecv (std::string thehost, int theport) {
        host = thehost;
        port = theport;
    }
};

int main(int argc, char* argv[]) {
    /*    print_vector(brute_perfect(1, 9589));

          rapidjson::Reader reader;
          char readBuffer[65536];
          rapidjson::FileReadStream is(stdin, readBuffer, sizeof(readBuffer));
          */
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[65536];

    if (argc < 3) {
        exit(0);
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
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
    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }
    /* send result */
    double result;
    char perf_json[256];
    result = test_speed();
    sprintf(perf_json,
            "{\"type\": \"ack\", \"data\": {\"result\": %f}}\r\n",
            result);
    n = write(sockfd, perf_json, strlen(perf_json));
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }

    bzero(buffer,65536);
    n = read(sockfd, buffer, 65536);
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }

    //rapidjson::Reader reader;
	//char readBuffer[65536];
    //rapidjson::FileReadStream is(sockfd, readBuffer, sizeof(readBuffer));

    rapidjson::Document document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.

    std::cout << "start output";
    printf("%s\n",buffer);

	if (document.Parse<0>(buffer).HasParseError()) {
        std::cout << "ERROR parseing json";
        exit(EXIT_FAILURE);
    }

    std::string type = document["type"].GetString();
    std::cout << type;
    if(type.compare("range") == 0) {
        std::cout << "parsed!!";
    }

	printf("\nModified JSON with reformatting:\n");
    rapidjson::FileStream f(stdout);
	rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
	document.Accept(writer);	// Accept() traverses the DOM and generates Handler events.

    //printf("%s\n",buffer);
    close(sockfd);
    return 0;
}

