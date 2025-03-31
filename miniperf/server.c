#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "network.h"

#define BACKLOG 5

// Thread function to handle each incoming TCP connection.
void *handle_client(void *arg) {
    udp_thread_arg_t* udp_arg= (udp_thread_arg_t *)arg;
    // printf("PORT :%d",udp)
    // free(arg);
    // TODO: Implement TCP signaling:
    //   - Receive messages (e.g., experiment start/stop, parameters)
    //   - Setup UDP receiving for experiment data
    //   - Process UDP packets: count bytes, measure jitter, detect packet loss, etc.
    printf("Client connected. Handling TCP signaling and experiment data in a dedicated thread.\n");

    // [Placeholder for TCP message handling code]
    // Create a UDP socket.
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("UDP socket");
        pthread_exit(NULL);
    }
    
    printf("NIGGAS!\n");

    // Bind the socket to UDP_PORT on all interfaces.
    struct sockaddr_in server_udp_addr;
    memset(&server_udp_addr, 0, sizeof(server_udp_addr));
    server_udp_addr.sin_family = AF_INET;
    server_udp_addr.sin_port = htons(udp_arg->config->port);
    server_udp_addr.sin_addr.s_addr = INADDR_ANY;
    
    printf("NIGGAS!\n");

    if (bind(udp_sock, (struct sockaddr *)&server_udp_addr, sizeof(server_udp_addr)) < 0) {
        perror("UDP bind");
        close(udp_sock);
        pthread_exit(NULL);
    }
    
    // Buffer to store the incoming packet.
    char buffer[1400];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);


    struct timespec start, curr,end;


    unsigned long long int totalbytes_recv=0;

    // Start tracking time by getting the current time from CLOCK_MONOTONIC
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        perror("Failed to get start time");
        exit(EXIT_FAILURE);
    }
    
    unsigned int elapsed = 0U;
    while(elapsed <  udp_arg->config->duration ){
        ssize_t received_bytes = recvfrom(udp_sock, buffer, sizeof(buffer), 0,
                                        (struct sockaddr *)&client_addr, &addr_len);
        if (received_bytes < 0) {
            // perror("recvfrom");
            printf("RECEIVED < 0 BYTES\n");
        } else {
            //TODO: Account for header size
            totalbytes_recv +=received_bytes;
            // printf("UDP Receiver: Received %zd bytes.\n", received_bytes);
            // printf("%s\n",buffer);
        }
        // Calculate the elapsed time in seconds
        if (clock_gettime(CLOCK_MONOTONIC, &curr) != 0) {
            perror("Failed to get start time");
            exit(EXIT_FAILURE);
        }
        elapsed = (curr.tv_sec - start.tv_sec) +
              (curr.tv_nsec - start.tv_nsec) / 1e9;
    }
    printf("Total Bytes received:%llu\n",totalbytes_recv);
    close(udp_sock);
    pthread_exit(NULL);
}

// Main server function.
int server_main(const config_t *config) {
    int tcp_sock, *client_sock_ptr;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t thread_id;
    
    // Create TCP socket for control signaling.
    if ((tcp_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    // Bind to the specified IP address and port.
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config->port);
    server_addr.sin_addr.s_addr = inet_addr(config->ip_addr);
    
    if (bind(tcp_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(tcp_sock);
        exit(EXIT_FAILURE);
    }
    
    // Listen for incoming TCP connections.
    if (listen(tcp_sock, BACKLOG) < 0) {
        perror("listen");
        close(tcp_sock);
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on %s:%d\n", config->ip_addr, config->port);
    config_t *sender_config = malloc(sizeof(config_t));
    
    // Accept and handle incoming TCP connections in separate threads.
    // while (1) {
        int *client_sock = malloc(sizeof(int));
        if (!client_sock) {
            perror("malloc");
            // continue;
        }
        *client_sock = accept(tcp_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (*client_sock < 0) {
            perror("accept");
            free(client_sock);
            // continue;
        }

        //Receive initial data from TCP connection (flags)
        ssize_t bytes_received = recv(*client_sock, sender_config, sizeof(config_t), 0);
        if (bytes_received < 0) {
            perror("recv failed");
        } else {
            printf("Received %zd bytes with duration of connection: %d\n", bytes_received, sender_config->duration );
        }




        udp_thread_arg_t* udp_arg = malloc(sizeof(udp_thread_arg_t));
        udp_arg->config = (config_t *)config;
        udp_arg->socket_num = *client_sock;
        if (pthread_create(&thread_id, NULL, handle_client, udp_arg) != 0) {
            perror("pthread_create");
            close(*client_sock);
            free(client_sock);
            // continue;
        }
        // pthread_detach(thread_id);
        pthread_join(thread_id, NULL);
    // }
    
    close(tcp_sock);
    return 0;
}
