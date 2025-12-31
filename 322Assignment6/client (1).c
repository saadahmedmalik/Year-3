/*
ECE 322 Assignment 6 - XML-RPC Client
William Neel 33855470
Saad Malik 33783667
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"

// function prototypes
void parse_response(const char *response, double *values);

int main(int argc, char **argv) {
    int clientfd;
    char *num1, *num2;
    char *host, *port;
    char requestBody[MAXLINE];
    char requestHeader[MAXLINE];
    char request[MAXLINE];
    char buf[MAXLINE];
    char reply[MAXLINE] = "";
    double numbers[2];
    rio_t rio;

    if (argc != 3) {
        fprintf(stderr, "usage: %s <num1> <num2>\n", argv[0]);
        exit(1);
    }

    num1 = argv[1];
    num2 = argv[2];

    host = "localhost";
    port = "8080";

    // Build the body of the request
    snprintf(requestBody, sizeof(requestBody),
             "<?xml version=\"1.0\"?>\r\n"
             "<methodCall>\r\n"
             "    <methodName>sample.addmultiply</methodName>\r\n"
             "    <params>\r\n"
             "        <param>\r\n"
             "            <value><double>%s</double></value>\r\n"
             "        </param>\r\n"
             "        <param>\r\n"
             "            <value><double>%s</double></value>\r\n"
             "        </param>\r\n"
             "    </params>\r\n"
             "</methodCall>\r\n",
             num1, num2);

    // Build the header of the request
    snprintf(requestHeader, sizeof(requestHeader),
             "POST /RPC2 HTTP/1.1\r\n"
             "User-Agent: XML-RPC.NET\r\n"
             "Host: %s:%s\r\n"
             "Content-Length: %ld\r\n"
             "Content-Type: text/xml\r\n"
             "Connection: Keep-Alive\r\n"
             "\r\n",
             host, port, strlen(requestBody));

    // Combine header and body into the request
    snprintf(request, sizeof(request), "%s%s", requestHeader, requestBody);

    // printf("Request Header:\n%s\n", requestHeader);
    // printf("Request Body:\n%s\n", requestBody);

    // Open socket connection
    clientfd = Open_clientfd(host, port);
    if (clientfd < 0) {
        fprintf(stderr, "Error connecting to server\n");
        exit(1);
    }

    // Send the request
    Rio_writen(clientfd, request, strlen(request));

    // Initialize the rio_t structure
    Rio_readinitb(&rio, clientfd);

    int len; // Length of line read from rio buffer
    while ((len = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        // Construct reply buffer to be parsed
        strncat(reply, buf, len);
    }

    // Retrieve numbers from reply
    parse_response(reply, numbers);
    //printf("%s",reply);
    // Return values to stdout
    printf("%.1lf %.1lf\n", numbers[0], numbers[1]);

    // Close connection with server
    Close(clientfd);
    return 0;
}

// Parse the response from the server and extract the two numbers
void parse_response(const char *response, double *values) {
    const char *firstNum = strstr(response, "<double>");
    const char *secondNum;
    if (firstNum) {
        // First occurrence of float between <double></double> tags
        sscanf(firstNum, "<double>%lf</double>", &values[0]);
        secondNum = strstr(firstNum + 1, "<double>");
        if (secondNum) {
            // Second occurrence of float between <double></double> tags
            sscanf(secondNum, "<double>%lf</double>", &values[1]);
        }
    }
}