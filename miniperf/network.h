#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>

// Global configuration structure holding command-line parameters.
typedef struct config_t {
    int is_server;      // 1 for server mode, 0 for client mode
    char ip_addr[64];   // Server IP (for client) or local bind address (for server)
    int port;           // TCP port number for control channel
    int interval;       // Interval (seconds) to print experiment progress
    char output_file[256]; // File to store results

    // Client-specific parameters:
    int udp_packet_size;  // UDP packet size in bytes
    long bandwidth;       // Target bandwidth in bits per second
    int num_streams;      // Number of parallel UDP data streams
    int duration;         // Duration of experiment in seconds
    int measure_delay;    // Flag to measure one-way delay instead of throughput etc.
    int wait_time;        // Wait time (seconds) before starting UDP transmission
} config_t;

extern config_t config;


// Structure to pass parameters to each UDP stream thread.
typedef struct {
    int thread_id;
    config_t* config;
    // Additional fields (e.g., UDP socket, server address) can be added.
    int socket_num;
} udp_thread_arg_t;

extern udp_thread_arg_t udp_arg;


// --- TCP Control Channel Definitions ---

// Define message types for the TCP signaling channel.
typedef enum {
    MSG_START_EXPERIMENT,
    MSG_STOP_EXPERIMENT,
    MSG_REPORT_RESULTS,
    // Add additional message types as needed.
} tcp_msg_type_t;

// Constant size TCP header for control messages.
typedef struct {
    uint16_t msg_type;    // Message type (from tcp_msg_type_t)
    uint16_t msg_length;  // Length of the payload (if any)
    // You can add more fields (e.g., flags or sequence numbers) if necessary.
} tcp_header_t;

// --- UDP Experiment Packet Definitions ---

// UDP packet header to include a sequence number for packet loss measurement.
typedef struct {
    uint32_t seq_num;    // Sequence number for packet ordering/loss detection
    struct timespec sent_time;
    // time_t time_telapsed;
    // You can add additional fields if needed.
} udp_header_t;

// --- Experiment Results Structure ---

typedef struct {
    double throughput;              // Measured throughput (e.g., in Mbps)
    double goodput;                 // Measured goodput (payload only)
    double jitter_avg;              // Average jitter (inter-arrival time variance)
    double jitter_std;              // Standard deviation of jitter
    double packet_loss_percentage;  // Calculated packet loss percentage
    double one_way_delay;           // Estimated one way delay (e.g. RTT/2 or improved estimation)
} experiment_results_t;

// --- Utility Function Prototypes ---

// Returns the current time in seconds (using a monotonic clock)
double get_time_in_seconds();

#endif // NETWORK_H
