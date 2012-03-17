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
/* for remote kill */
#include <pthread.h>
#include <signal.h>

#define DEBUG 1

typedef long range_t[2];
/* bzero is legacy, so define */
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)


/*
 * Make sockfd and thread global so we can close them later
 */
long sockfd;
pthread_t thread;
//pthread_mutex_t the_mutex; // allow signal handling w/ thread paused
/*
 * For each number in range, test if perfect number
 */
void brute_perfect( long start,  long end, std::vector<long> *perfect) {

    for (long testnum = start; testnum < end; ++testnum) {
        long factor_sum = 0;
        for (long k = 1; k < testnum; ++k) {
            if (testnum % k == 0) { // is k a factor of testnum?
                factor_sum += k;
            }
        }
        // does testnum meet the definition of perf. num?
        if (factor_sum == testnum) {
            perfect->push_back(testnum);
        }
    }
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

void sig_handler(int signum) {
     //pthread_mutex_lock(&the_mutex);
#ifdef DEBUG
    printf("Caught signal %d\n",signum);
    /* Clean up, clean up, everybody everywhere. */
#endif
    close(sockfd);
    pthread_cancel(thread);
    //pthread_mutex_unlock(&the_mutex);
    exit(signum);
}

/* allow for remote kill */
void *kill(void *sockfd) {
    long sock = (long) sockfd;
    int n;
    char buffer[65536];
    // Default template parameter uses UTF8 and MemoryPoolAllocator.

    signal(SIGHUP,sig_handler); /* set function calls */
    signal(SIGINT,sig_handler);
    signal(SIGQUIT, sig_handler);

    rapidjson::Document document;	
    while (true) {
        bzero(buffer, 65536);
        n = recv(sock, buffer, 65536, MSG_PEEK);
        if (n < 0) {
            perror("ERROR reading from socket: thread");
            exit(EXIT_FAILURE);
        }

        if (document.Parse<0>(buffer).HasParseError()) {
            std::cout << "ERROR parsing json: thread\n";
            exit(EXIT_FAILURE);
        }

        // print parsed input
        std::string type = document["type"].GetString();
        if(type.compare("kill") == 0) {

#ifdef DEBUG
            rapidjson::FileStream f(stdout);
            rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
            document.Accept(writer);	// Accept() traverses the DOM and generates Handler events.
#endif
            n = recv(sock, buffer, 65536, 0);
            kill(getpid(), SIGQUIT);
        }
    }

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

    if (document.Parse<0>(buffer).HasParseError()) {
        std::cout << "ERROR parsing json\n";
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    rapidjson::FileStream f(stdout);
    rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
    document.Accept(writer);	// Accept() traverses the DOM and generates Handler events.
#endif

    // print parsed input
    std::string type = document["type"].GetString();
    if(type.compare("range") != 0) {
        std::cout << "json not type 'range'";
        exit(0);
    }

    const rapidjson::Value &data = document["data"];
    std::vector<long> range(2);
    range[0] = ( long) floor(data["lower"].GetDouble());
    range[1] = ( long) floor(data["upper"].GetDouble());
    return range;
}

void build_json_result(long upper, std::vector<long> *brute, double perform, char* buffer) {
    char brute_buff[65536];
    char temp[65536];
    bzero(temp, 65536);
    bzero(brute_buff, 65536);
    for (int i = 0; i < (int) brute->size(); ++i) {
        if (i == (int) brute->size() - 1) {
            sprintf(temp, "%ld", brute->at(i));
        } else {
            sprintf(temp, "%ld, ", brute->at(i));
        }
        strcat(brute_buff, temp);
    }
    sprintf(buffer, "{\"type\": \"result\", \"data\": {\"upper\": %ld, \"perform\": %f, \"perfect\": [%s]}}\r\n",
            upper, perform, brute_buff);
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
    long portno;
    long n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[65536];
    std::vector<long> brute;
    std::vector<long> range;

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


    //pthread_mutex_init(&the_mutex, NULL);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&thread, &attr, kill, (void *) sockfd);

    /* send initial Ack message */
    double perform;
    perform = test_speed();
    bzero(buffer,65536);
    sprintf(buffer,
            "{\"type\": \"ack\", \"data\": {\"perform\": %f}}\r\n",
            perform);
    write_to_server(sockfd, buffer);

    while(true) {
        /* read range info from server */
        bzero(buffer,65536);
        n = recv(sockfd, buffer, 65536, 0);
        if (n < 0) {
            perror("ERROR reading from socket");
            exit(EXIT_FAILURE);
        }
        range = parse_json_range(buffer);

        /* we're at max int, break and exit */ 
        if(range[0] == range[1]) {
            std::cout << "Max computation range reached!\n" << range[0];
            break;
        }
        brute.clear();
        brute_perfect(range[0], range[1], &brute);
        for(int k; k < (int) brute.size(); k++) {
            std::cout << brute.at(k) << " ";
        }
        /* run brute computation and send result */
        bzero(buffer, 65536);
        build_json_result(range[1], &brute, perform, buffer);
#ifdef DEBUG
        std::cout << std::endl << buffer;
#endif
        write_to_server(sockfd, buffer);
    }

    pthread_cancel(thread);
    close(sockfd);
    return 0;
}

