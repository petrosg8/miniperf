#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <poll.h>
#include <math.h>
#include "network.h"

#define BACKLOG 5


// Thread function to handle each incoming TCP connection.
void *handle_client(void *arg) {
    udp_thread_arg_t* udp_arg= (udp_thread_arg_t *)arg;
    printf("IN HANDLE DURATION : %d\n", udp_arg->config->duration);
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


    struct timespec start, curr;


    unsigned long long int totalbytes_recv=0;

    // ssize_t kalo = recvfrom(udp_sock, buffer, sizeof(buffer), 0,
    //                                     (struct sockaddr *)&client_addr, &addr_len);
    int flags = fcntl(udp_sock, F_GETFL, 0);    // Get current socket flags.
    if (flags == -1) {
        perror("fcntl(F_GETFL) failed");
        // Handle error...
    }

    // Set the socket to non-blocking mode.
    if (fcntl(udp_sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL) failed");
        // Handle error...
    }



    // Start tracking time by getting the current time from CLOCK_MONOTONIC
    // if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
    //     perror("Failed to get start time");
    //     exit(EXIT_FAILURE);
    // }
    unsigned long long int totalpackets_received=0;
    int i=0;
    time_t elapsed = 0;
    unsigned long long int last_seq = 0;
    double prev_packet_one_way_delay = 0;
    double jitter = 0;
    double jitter_avg = 0;
    long int seq_diff = 0;
    long int lost_packets = 0;

    while(elapsed <  udp_arg->config->duration ){
        // printf("ELAPSED TIME: %ld\n",elapsed);
        // printf("ELAPSED TIME: %ld",elapsed);

        ssize_t received_bytes = recvfrom(udp_sock, buffer, sizeof(buffer), 0,
                                        (struct sockaddr *)&client_addr, &addr_len);
        if (received_bytes < 0) {
            // perror("recvfrom");
            // printf("RECEIVED < 0 BYTES %d\n",received_bytes);
        } 
        else {
            //TODO: Account for header size
            // printf("RECEIVED > 0 BYTES\n");  
            if(i==0){
                if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
                    perror("Failed to get start time");
                    exit(EXIT_FAILURE);
                }
                i++;
            }
            // if(i>=1){
            // Calculate the elapsed time in seconds            
            if(clock_gettime(CLOCK_MONOTONIC, &curr) !=0){
                perror("Failed to get start time");
                exit(EXIT_FAILURE);
            }
            // elapsed = (curr.tv_sec - start.tv_sec) +
            // (curr.tv_nsec - start.tv_nsec)/ 1e9;
            i++;
            
                
            // }
            
            totalbytes_recv += received_bytes;
            totalpackets_received++;
            // printf("UDP Receiver: Received %zd bytes.\n", received_bytes);

            udp_header_t *header = (udp_header_t *)buffer;

            seq_diff = header->seq_num - (last_seq + 1);
            if(seq_diff >= 1) lost_packets += seq_diff;
            
            //Calculate one way delay using current time and sender's time
            double one_way_delay_ms = ((curr.tv_sec - header->sent_time.tv_sec) * 1000.0) +
                          ((curr.tv_nsec - header->sent_time.tv_nsec) / 1e6);

            //For the first packet, dont measure jitter
            if(i > 2){
                jitter = fabs(one_way_delay_ms - prev_packet_one_way_delay);
                jitter_avg += jitter;
            }

            // // Print the header fields.
            // printf("Received packet:\n");
            // printf("  Sequence Number: %u\n", header->seq_num);
            // printf("  last_Sequence Number: %llu\n", last_seq);
            // printf("  seq_diff: %ld\n", seq_diff);
            // printf("  lost_packets %ld\n", lost_packets);
            // printf("  Sent Time: %ld seconds, %ld nanoseconds\n", 
            // header->sent_time.tv_sec , header->sent_time.tv_nsec);
            // // printf("  one-way delay %lu\n\n",one_way_delay);
            // printf("  one-way delay ms %f\n",one_way_delay_ms);
            // printf("  jitter %f\n\n",jitter);
            
            
            prev_packet_one_way_delay = one_way_delay_ms;
            last_seq = header->seq_num;
        }
        
        //TODO: MIGHT NEED TO ADD THIS FOR THE LOOP CONDITION
        if(i>=1){
            // Calculate the elapsed time in seconds            
            if(clock_gettime(CLOCK_MONOTONIC, &curr) !=0){
                perror("Failed to get start time");
                exit(EXIT_FAILURE);
            }
            elapsed = (curr.tv_sec - start.tv_sec) +
            (curr.tv_nsec - start.tv_nsec)/1e9;
        }
    }

    // ... [rest of your handle_client() function remains unchanged]

    // After the loop ends, calculate elapsed time (duration already computed in 'elapsed')
    // 'totalbytes_recv' is the sum of all payload bytes received
    // 'totalpackets_received' is the number of packets processed.
    // Assume each packet on the wire is 846 bytes (800 payload + 46 overhead bytes)
    unsigned long long total_goodput_bytes = totalbytes_recv;  // payload-only
    unsigned long long total_throughput_bytes = totalpackets_received * 1432;  // estimated on-wire bytes


    // Calculate goodput and throughput in bits per second (bps)
    double goodput_bps = (total_goodput_bytes * 8.0) / udp_arg->config->duration;
    double throughput_bps = (total_throughput_bytes * 8.0) / udp_arg->config->duration;;

    // Optionally convert to higher units (e.g. Gbps)
    double goodput_Gbps = goodput_bps / 1e9;
    double throughput_Gbps = throughput_bps / 1e9;

    double packet_loss_percentage = ((double)lost_packets / (totalpackets_received + lost_packets)) * 100.0;
    jitter_avg = jitter_avg / (totalpackets_received - 1); //Not including the first packet

    // Print results
    printf("ELAPSED TIME: %ld seconds\n", elapsed);
    printf("Total Packets Received: %llu\n", totalpackets_received);
    printf("Total Bytes received (payload/goodput): %llu bytes\n", total_goodput_bytes);
    printf("Estimated Total Bytes on Wire (throughput): %llu bytes\n", total_throughput_bytes);
    printf("Goodput: %.2f Gbps\n", goodput_Gbps);
    printf("Throughput (with headers): %.2f Gbps\n", throughput_Gbps);
    printf("Jitter (avg):  %f\n",jitter_avg);
    printf("Packet loss%%:  %f\n", packet_loss_percentage);
    printf("#Lost Packets:  %ld\n\n", lost_packets);

    close(udp_sock);
    pthread_exit(NULL);
}

// Main server function.
int server_main(const config_t *config) {
    int tcp_sock;
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
        udp_arg->config->duration = sender_config->duration;
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
