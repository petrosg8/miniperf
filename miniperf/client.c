#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "network.h"

// // Structure to pass parameters to each UDP stream thread.
// typedef struct {
//     int thread_id;
//     config_t config;
//     // Additional fields (e.g., UDP socket, server address) can be added.
// } udp_thread_arg_t;

// UDP stream thread: sends UDP packets based on client parameters.
void *udp_stream_thread(void *arg) {
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

    // Prepare a 50-byte packet (filled with 'A's for demonstration).
    char packet[50];
    memset(packet, 'A', sizeof(packet));

    // (Optional) You can include a header with a sequence number if desired.
    // For now, we are just sending a raw 50-byte packet.

    // Send the packet.
    ssize_t sent_bytes = sendto(udp_sock, packet, sizeof(packet), 0,
                                (struct sockaddr *)&server_udp_addr, sizeof(server_udp_addr));
    if (sent_bytes < 0) {
        perror("sendto");
    } else {
        printf("UDP stream thread %d: Sent %zd bytes.\n", udp_arg->thread_id, sent_bytes);
    }

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
    
    // TODO: Exchange TCP signaling messages to:
    //   - Inform the server about experiment parameters (UDP packet size, number of streams, etc.)
    //   - Signal the start of the experiment.
    
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
