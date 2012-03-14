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
#include <math.h>
#include <fcntl.h>
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

#define DEBUG 1

typedef long range_t[2];
/* bzero is legacy, so define */
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
/*
 * For each number in range, test if perfect number
 */
std::vector<long> brute_perfect( long start,  long end) {
    std::vector<long> perfect_nums;

    for (long testnum = start; testnum < end; ++testnum) {
        long factor_sum = 0;
        for (long k = 1; k < testnum; ++k) {
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
void print_vector(std::vector<long> temp) {
    for (int i = 0; i < (int) temp.size(); ++i) {
        std::cout << temp.at(i) << " ";
    }
    std::cout << std::endl;
}

std::vector<long> parse_json_range(char buffer[]) {
    rapidjson::Document document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.

#ifdef DEBUG
    if (document.Parse<0>(buffer).HasParseError()) {
        std::cout << "ERROR parseing json\n";
        exit(EXIT_FAILURE);
    }
#endif

    rapidjson::FileStream f(stdout);
    rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
    document.Accept(writer);	// Accept() traverses the DOM and generates Handler events.

#ifdef DEBUG
    // print parsed input
    std::string type = document["type"].GetString();
    if(type.compare("range") != 0) {
        std::cout << "json not type 'range'";
        exit(0);
    }
#endif

    const rapidjson::Value &data = document["data"];
    std::vector<long> range(2);
    range[0] = ( long) floor(data["lower"].GetDouble());
    range[1] = ( long) floor(data["upper"].GetDouble());
    return range;
}

void build_json_result(long upper, std::vector<long> brute, char* buffer) {
    char brute_buff[65536];
    char temp[65536];
    for (int i = 0; i < (int) brute.size(); ++i) {
        if (i == (int) brute.size() -1) {
            sprintf(temp, "%ld", brute.at(i));
        } else {
            sprintf(temp, "%ld, ", brute.at(i));
        }
        strcat(brute_buff, temp);
    }
    sprintf(buffer, "{\"type\": \"result\", \"data\": {\"upper\": %ld, \"perfect\": [%s]}}\r\n",
            upper, brute_buff);
}
void write_to_server(int sockfd, char* buffer) {
    int n = send(sockfd, buffer, strlen(buffer) + 1, 0);
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(EXIT_FAILURE);
    }
}
void read_from_server(int sockfd, char* buffer) {

}

int main(int argc, char* argv[]) {

    // a little ugly, but working...
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
    //fcntl(sockfd, F_SETFL, O_NONBLOCK);

    /* send result */
    double result;
    result = test_speed();
    bzero(buffer,65536);
    sprintf(buffer,
            "{\"type\": \"ack\", \"data\": {\"perform\": %f}}\r\n",
            result);

    write_to_server(sockfd, buffer);

    bzero(buffer,65536);
    n = recv(sockfd, buffer, 65536, 0);
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(EXIT_FAILURE);
    }

    std::vector<long> range = parse_json_range(buffer);
    std::vector<long> brute = brute_perfect(range[0], range[1]);
    bzero(buffer,65536);
    build_json_result(range[1], brute, buffer);
#ifdef DEBUG
    std::cout << std::endl << buffer;
#endif
    write_to_server(sockfd, buffer);

    close(sockfd);
    return 0;
}

