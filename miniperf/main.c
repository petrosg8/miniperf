#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "network.h"

// Forward declarations for server and client main functions.
extern int server_main(const struct config_t *config);
extern int client_main(const struct config_t *config);



config_t config;

// Print usage instructions.
void print_usage(const char *progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Options:\n");
    printf("  -s             Run as server\n");
    printf("  -c             Run as client\n");
    printf("  -a <ip_addr>   IP address (server: binding address, client: server IP)\n");
    printf("  -p <port>      TCP port number\n");
    printf("  -i <interval>  Interval in seconds for progress update\n");
    printf("  -f <file>      Output file to store results\n");
    // Client-specific options:
    printf("  -l <size>      UDP packet size in bytes\n");
    printf("  -b <bandwidth> Bandwidth in bits per second\n");
    printf("  -n <streams>   Number of parallel UDP streams\n");
    printf("  -t <duration>  Duration of the experiment in seconds\n");
    printf("  -d             Measure one way delay instead of throughput/jitter\n");
    printf("  -w <wait>      Wait time in seconds before starting the experiment\n");
}

// Parse command-line arguments.
int parse_arguments(int argc, char *argv[]) {
    // Set default values.
    memset(&config, 0, sizeof(config));
    config.port = 5001;
    config.interval = 1;
    config.udp_packet_size = 1024;
    config.bandwidth = 1000000; // Default: 1 Mbps.
    config.num_streams = 1;
    config.duration = 10;
    config.measure_delay = 0;
    config.wait_time = 0;
    strcpy(config.ip_addr, "0.0.0.0");
    strcpy(config.output_file, "results.txt");

    int opt;
    while ((opt = getopt(argc, argv, "sca:p:i:f:l:b:n:t:dw:")) != -1) {
        switch (opt) {
            case 's':
                config.is_server = 1;
                break;
            case 'c':
                config.is_server = 0;
                break;
            case 'a':
                strncpy(config.ip_addr, optarg, sizeof(config.ip_addr)-1);
                break;
            case 'p':
                config.port = atoi(optarg);
                break;
            case 'i':
                config.interval = atoi(optarg);
                break;
            case 'f':
                strncpy(config.output_file, optarg, sizeof(config.output_file)-1);
                break;
            case 'l':
                config.udp_packet_size = atoi(optarg);
                break;
            case 'b':
                config.bandwidth = atol(optarg);
                break;
            case 'n':
                config.num_streams = atoi(optarg);
                break;
            case 't':
                config.duration = atoi(optarg);
                break;
            case 'd':
                config.measure_delay = 1;
                break;
            case 'w':
                config.wait_time = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
                return -1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (parse_arguments(argc, argv) != 0) {
        return EXIT_FAILURE;
    }
    
    if (config.is_server) {
        printf("Running in server mode...\n");
        return server_main(&config);
    } else {
        printf("Running in client mode...\n");
        return client_main(&config);
    }
}
