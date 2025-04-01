#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include "network.h"

//TODO: MAKE SURE THAT JITTER IS WORKING CORRECTLY!!!



// // Structure to pass parameters to each UDP stream thread.
// typedef struct {
//     int thread_id;
//     config_t config;
//     // Additional fields (e.g., UDP socket, server address) can be added.
// } udp_thread_arg_t;

// UDP stream thread: sends UDP packets based on client parameters.
static void sleep_ms(long milliseconds) {
    struct timespec req;
    req.tv_sec = milliseconds / 1000;                  // seconds
    req.tv_nsec = (milliseconds % 1000) * 1000000L;      // nanoseconds
    nanosleep(&req, NULL);
}
void *udp_stream_thread(void *arg) {
    struct timespec start, curr , end;
    udp_thread_arg_t *udp_arg = (udp_thread_arg_t *)arg;
    // TODO: Create a UDP socket.
    // TODO: Implement throttling:
    //   - Calculate delay between packets based on the specified bandwidth and UDP packet size.
    // TODO: Send UDP packets including a header with sequence number.
    // TODO: For one-way delay measurement, include timestamps if needed.
    printf("UDP stream thread %d started. Sending experiment packets...\n", udp_arg->thread_id);
    
    // [Placeholder for UDP sending loop – use gettimeofday or clock_gettime for precise timing]
    
    // Create a UDP socket.
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        perror("UDP socket");
        pthread_exit(NULL);
    }

    // Prepare the server address.
    struct sockaddr_in server_udp_addr;
    memset(&server_udp_addr, 0, sizeof(server_udp_addr));
    server_udp_addr.sin_family = AF_INET;
    server_udp_addr.sin_port = htons(udp_arg->config->port);
    server_udp_addr.sin_addr.s_addr = inet_addr(udp_arg->config->ip_addr);

    // Packet size is variable (800 bytes for now)
    int packet_size = 1400;
    char packet[packet_size];
    
    // Allocate and initialize the header.
    udp_header_t *udp_header = malloc(sizeof(udp_header_t));
    if (!udp_header) {
        perror("malloc udp_header");
        close(udp_sock);
        pthread_exit(NULL);
    }
    // Optionally, zero the header initially.
    memset(udp_header, 0, sizeof(udp_header_t));

    // Fill the remainder of the packet with 'A' (starting after header).
    int header_size = sizeof(udp_header_t);
    if (header_size < packet_size) {
        memset(packet + header_size, 'A', packet_size - header_size);
    } else {
        fprintf(stderr, "Header size exceeds packet size!\n");
        free(udp_header);
        close(udp_sock);
        pthread_exit(NULL);
    }

    

    unsigned long long int totalbytes_sent=0;
    // sleep(1);
        
    // ssize_t kalo = sendto(udp_sock, packet, sizeof(packet), 0,
    //                                 (struct sockaddr *)&server_udp_addr, sizeof(server_udp_addr));

    //   int flags = fcntl(udp_sock, F_GETFL, 0);    // Get current socket flags.
    // if (flags == -1) {
    //     perror("fcntl(F_GETFL) failed");
    //     // Handle error...
    // }

    // // Set the socket to non-blocking mode.
    // if (fcntl(udp_sock, F_SETFL, flags | O_NONBLOCK) == -1) {
    //     perror("fcntl(F_SETFL) failed");
    //     // Handle error...
    // }                                   
    // Start tracking time by getting the current time from CLOCK_MONOTONIC
    sleep(1);
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        perror("Failed to get start time");
        exit(EXIT_FAILURE);
             
    }

    unsigned long long int seq = 0;
    unsigned long long int totalpackets_sent = 0;
    time_t elapsed = 0;
    int i=0;
    while((elapsed <  udp_arg->config->duration)){
        // Send the packet.
        
        if(i == 0){
            if (clock_gettime(CLOCK_MONOTONIC, &curr) != 0) {
                perror("Failed to get start time");
                exit(EXIT_FAILURE);
            }
            i++;
        }

        udp_header->seq_num = seq++;
        udp_header->sent_time = curr;
        
        memcpy(packet, udp_header, header_size);
        ssize_t sent_bytes = sendto(udp_sock, packet, sizeof(packet), 0,
                                    (struct sockaddr *)&server_udp_addr, sizeof(server_udp_addr));
                                    
        if (sent_bytes <= 0) {
         
        } else {
            totalbytes_sent += sent_bytes;
            totalpackets_sent++;
            // printf("UDP stream thread %d: Sent %zd bytes.\n", udp_arg->thread_id, sent_bytes);
        }        

        // Calculate the elapsed time in seconds
        if (clock_gettime(CLOCK_MONOTONIC, &curr) != 0) {
            perror("Failed to get start time");
            exit(EXIT_FAILURE);
        }
        sleep_ms(1);
        elapsed = (curr.tv_sec - start.tv_sec) +
              (curr.tv_nsec - start.tv_nsec) / 1e9;
    }
    printf("Total Bytes sent:%llu\n",totalbytes_sent);
    printf("Total packets sent:%llu\n",totalpackets_sent);
    
    close(udp_sock);
    

    pthread_exit(NULL);
}

// Main client function.
int client_main(const config_t *config) {
    int tcp_sock;
    struct sockaddr_in server_addr;
    
    // Create a TCP socket for control signaling.
    if ((tcp_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    // Setup server address for TCP signaling.
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config->port);
    server_addr.sin_addr.s_addr = inet_addr(config->ip_addr);
    
    // Connect to the server.
    if (connect(tcp_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(tcp_sock);
        exit(EXIT_FAILURE);
    }
    
    printf("Connected to server %s:%d via TCP.\n", config->ip_addr, config->port);
    
    sleep((unsigned int)1);

    // TODO: Exchange TCP signaling messages to:
    //   - Inform the server about experiment parameters (UDP packet size, number of streams, etc.)
    //   - Signal the start of the experiment.
    ssize_t bytes_sent = send(tcp_sock, config, sizeof(config_t), 0);
    if (bytes_sent < 0) {
        perror("send failed");
    }

    
    // Create threads for parallel UDP streams.
    pthread_t *threads = malloc(config->num_streams * sizeof(pthread_t));
    udp_thread_arg_t *thread_args = malloc(config->num_streams * sizeof(udp_thread_arg_t));
    if (!threads || !thread_args) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < config->num_streams; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].config = (config_t*)config;  // Copy global config for thread use.
        if (pthread_create(&threads[i], NULL, udp_stream_thread, &thread_args[i]) != 0) {
            perror("pthread_create");
        }
    }
    
    // Wait for all UDP threads to complete (i.e. experiment duration).
    for (int i = 0; i < config->num_streams; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // TODO: Signal the server via TCP to end the experiment and request results.
    // Optionally, receive and print the results from the server.
    
    // Cleanup.
    free(threads);
    free(thread_args);
    close(tcp_sock);
    return 0;
}
