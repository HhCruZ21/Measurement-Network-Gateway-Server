/******************************************************************************
 * @file    networkThread.c
 * @brief   TCP server implementation for sensor streaming.
 *
 * Features:
 *  - Multi-command handling (START, STOP, CONFIGURE, SHUTDOWN)
 *  - Dynamic sensor rate control
 *  - Batched transmission using length-prefixed frames
 *  - Thread-safe integration with ring buffer
 *
 * Optimized for low-latency streaming using TCP_NODELAY.
 *
 * @author  Haizon Helet Cruz
 * @date    2026-02-13
 ******************************************************************************/

#include "../include/networkThread.h"
#include "../include/sensorThread.h"

#define TCP_PAYLOAD_TARGET 1440
#define MAX_BATCH (TCP_PAYLOAD_TARGET / sizeof(sensor_data_t))

extern sensor_config_t sensor_cfg[snsr_cnt];

static int sensor_from_string(const char *s)
{
    if (!strcmp(s, "TEMP"))
        return temp_sid;
    if (!strcmp(s, "ADC0"))
        return adc_zero_sid;
    if (!strcmp(s, "ADC1"))
        return adc_one_sid;
    if (!strcmp(s, "SW"))
        return sw_sid;
    if (!strcmp(s, "PB"))
        return pb_sid;
    return -1;
}

void *networkTask(void *arg)
{
    sensor_data_t batch[MAX_BATCH];
    static uint64_t batch_counter = 0;

    printf("Network thread initialized...\n");

    network_thread_arg_t *srv_arg = (network_thread_arg_t *)arg;

    char cmd_buffer[CMD_BUF_SIZE];
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    stream_t stream_state = STREAM_IDLE;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed...");
        return NULL;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Set socket for reuse failed...");
        close(server_fd);
        return NULL;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(srv_arg->server_port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Socket bind failed...");
        close(server_fd);
        return NULL;
    }

    if (listen(server_fd, SOMAXCONN) < 0)
    {
        perror("Socket listen failed...");
        close(server_fd);
        return NULL;
    }

    printf("Server listening to port %d...\n", srv_arg->server_port);

    while (system_running)
    {
        printf("Waiting for client...\n");
        addr_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);

        if (client_fd < 0)
        {
            if (!system_running)
                break;
            printf("Client connection failed to server port %d...\n", srv_arg->server_port);
            continue;
        }

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 50000; // 50 ms timeout
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        int flag = 1;
        setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

        printf("Client connected...\n");
        stream_state = STREAM_IDLE;

        while (1)
        {
            memset(cmd_buffer, 0, CMD_BUF_SIZE);
            ssize_t bytes_received = recv(client_fd, cmd_buffer, CMD_BUF_SIZE - 1, 0);

            if (bytes_received > 0)
            {
                cmd_buffer[bytes_received] = '\0';

                // strip newline
                for (char *p = cmd_buffer; *p; ++p)
                    if (*p == '\n' || *p == '\r')
                        *p = '\0';

                printf("Received command: %s\n", cmd_buffer);

                if (!strcmp(cmd_buffer, "START"))
                {
                    sensor_rate_t rates[snsr_cnt];

                    printf("[DEBUG] Sending sensor rates to client:\n");

                    for (int i = 0; i < snsr_cnt; i++)
                    {
                        pthread_mutex_lock(&sensor_cfg[i].lock);
                        rates[i].sensor_id = sensor_cfg[i].sid;
                        rates[i].rate_hz = sensor_cfg[i].rate_hz;

                        printf("  Sensor ID %u -> %u Hz\n",
                               rates[i].sensor_id,
                               rates[i].rate_hz);
                        pthread_mutex_unlock(&sensor_cfg[i].lock);
                    }

                    fflush(stdout);

                    const char hdr[] = "RATES\n";
                    ssize_t h = send(client_fd, hdr, sizeof(hdr) - 1, 0);
                    if (h < 0)
                        perror("send RATES header failed");

                    // Send rate table
                    ssize_t s = send(client_fd, rates, sizeof(rates), 0);
                    if (s < 0)
                        perror("send sensor rates failed");
                    else
                        printf("[DEBUG] %ld bytes of rate data sent\n", s);

                    stream_state = STREAM_RUNNING;
                }
                else if (!strncmp(cmd_buffer, "CONFIGURE", 9))
                {
                    char sensor[16];
                    uint32_t rate;

                    if (sscanf(cmd_buffer, "CONFIGURE %15s %u", sensor, &rate) == 2)
                    {
                        int sid = sensor_from_string(sensor);
                        if (sid >= 0 && rate > 0)
                        {
                            pthread_mutex_lock(&sensor_cfg[sid].lock);
                            sensor_cfg[sid].rate_hz = rate;
                            sensor_cfg[sid].period_us = 1000000ULL / rate;
                            sensor_cfg[sid].next_deadline = getElapsedTime() + sensor_cfg[sid].period_us;
                            pthread_mutex_unlock(&sensor_cfg[sid].lock);
                            printf("[SERVER] CONFIGURE applied\n");
                        }
                    }
                }
                else if (!strcmp(cmd_buffer, "STOP"))
                {
                    stream_state = STREAM_IDLE;
                }
                else if (!strcmp(cmd_buffer, "DISCONNECT"))
                {
                    stream_state = STREAM_DISCONNECT;
                }
                else if (!strcmp(cmd_buffer, "SHUTDOWN"))
                {
                    printf("Shutting down...\n");
                    system_running = 0;
                    stream_state = STREAM_DISCONNECT;
                }
                else
                {
                    printf("Unknown command: %s\n", cmd_buffer);
                }
            }
            else if (bytes_received == 0)
            {
                stream_state = STREAM_DISCONNECT;
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    // no command, continue normal operation
                }
                else
                {
                    perror("recv failed");
                    stream_state = STREAM_DISCONNECT;
                }
            }

            if (stream_state == STREAM_RUNNING)
            {
                unsigned int count = 0;

                printf("[SERVER] rb->count = %d\n", rb->count);
                count = ringBufferRemoveBatch(rb, batch, MAX_BATCH);

                if (count == 0)
                {
                    // Not enough data yet
                    usleep(1000);
                    continue;
                }

                size_t total_bytes = count * sizeof(sensor_data_t);

                uint32_t net_size = htonl((uint32_t)total_bytes);

                /* ---- Send length prefix first ---- */
                size_t sent = 0;
                uint8_t *size_ptr = (uint8_t *)&net_size;

                while (sent < sizeof(net_size))
                {
                    ssize_t n = send(client_fd,
                                     size_ptr + sent,
                                     sizeof(net_size) - sent,
                                     0);
                    if (n <= 0)
                    {
                        perror("send size failed");
                        stream_state = STREAM_DISCONNECT;
                        break;
                    }
                    sent += n;
                }

                if (stream_state != STREAM_RUNNING)
                    continue;

                /* ---- Now send payload ---- */

                size_t sent_bytes = 0;
                uint8_t *ptr = (uint8_t *)batch;

                printf("[SERVER] Sending batch %" PRIu64
                       " | samples=%u | bytes=%zu\n",
                       batch_counter,
                       count,
                       total_bytes);

                while (sent_bytes < total_bytes)
                {
                    ssize_t n = send(client_fd,
                                     ptr + sent_bytes,
                                     total_bytes - sent_bytes,
                                     0);
                    if (n <= 0)
                    {
                        perror("send payload failed");
                        stream_state = STREAM_DISCONNECT;
                        break;
                    }
                    sent_bytes += n;
                }

                printf("[SERVER] Batch %" PRIu64
                       " sent | actual_bytes=%zu\n",
                       batch_counter,
                       sent_bytes);

                printf("[SERVER] Batch %" PRIu64
                       " sent | actual_bytes=%zu\n",
                       batch_counter,
                       sent_bytes);

                batch_counter++;
            }

            // Handle client here1
            if (stream_state == STREAM_DISCONNECT)
            {
                printf("Closing client connection...\n");
                shutdown(client_fd, SHUT_RDWR);
                close(client_fd);
                break;
            }
        }
        if (!system_running)
            break;
    }
    close(server_fd);
    return NULL;
}