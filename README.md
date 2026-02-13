# Measurement Network Gateway

High-performance multi-threaded sensor acquisition and TCP streaming server  
Designed for embedded Linux systems (ZedBoard / custom measurement device)

---

## Overview

This project implements a real-time measurement server that:

- Acquires sensor data from hardware or simulation
- Buffers samples using a thread-safe ring buffer
- Streams batched sensor data to a TCP client
- Allows runtime sensor rate configuration

The system automatically selects operating mode:

- **MODE_REAL** → Uses `/dev/meascdd` Linux kernel module  
- **MODE_SIM** → Generates synthetic sensor data if hardware is unavailable

---

## Architecture

```
                +-------------------+
                |   Sensor Thread   |
                |-------------------|
                | Periodic sampling |
                | Rate controlled   |
                +---------+---------+
                          |
                          v
                +-------------------+
                |   Ring Buffer     |
                |-------------------|
                | Thread-safe queue |
                +---------+---------+
                          |
                          v
                +-------------------+
                |  Network Thread   |
                |-------------------|
                | TCP server        |
                | Command parser    |
                | Batch streaming   |
                +-------------------+
```

---

## Directory Structure

```
project-root/
│
├── include/        # Header files
│   ├── mainH.h
│   ├── sensorThread.h
│   ├── networkThread.h
│   ├── ringBuffer.h
│   ├── fakeSensor.h
│   ├── utils.h
│   └── measdev.h
│
├── src/            # Source files
│   ├── main.c
│   ├── sensorThread.c
│   ├── networkThread.c
│   ├── ringBuffer.c
│   ├── fakeSensor.c
│   └── utils.c
│
├── Makefile
└── README.md
```

---

## Key Features

- Multi-threaded design using `pthread`
- Per-sensor dynamic sampling rate control
- Batch-based TCP streaming
- Length-prefixed binary framing
- Simulation fallback if hardware unavailable
- Microsecond-precision scheduling
- Lock-protected ring buffer
- TCP_NODELAY enabled for reduced latency

---

## Supported Commands

Client-to-server commands:

| Command | Description |
|----------|------------|
| START | Begin streaming sensor data |
| STOP | Pause streaming |
| CONFIGURE \<SENSOR_ID\> \<RATE_HZ\> | Set sampling rate in Hz |
| CONNECT \<IP_ADDR_IPv4\>| Connect to server using IP address |
| DISCONNECT | Close client connection |
| SHUTDOWN | Stop server completely |

### Supported Sensors

- TEMP  → Temperature sensor  
- ADC0  → ADC channel 0  
- ADC1  → ADC channel 1  
- SW    → Switch input  
- PB    → Push-button input  

---

## Build Instructions

```bash
make
```

Ensure your compiler supports:

- POSIX threads
- CLOCK_MONOTONIC
- Linux socket APIs

---

## Run

```bash
./main
```

Default server port:

```
50012
```

---

## Data Packet Format

Each TCP transmission follows this format:

```
[4-byte length prefix (network byte order)]
[Binary sensor_data_t array]
```

### sensor_data_t Structure (16 bytes)

| Field         | Size |
|---------------|------|
| sensor_id     | 4 bytes |
| sensor_value  | 4 bytes |
| timestamp     | 8 bytes |

Timestamp is in microseconds since server startup.

---

## Real-Time Scheduling

Each sensor maintains:

- `rate_hz`
- `period_us`
- `next_deadline`

Deadline-based scheduling ensures stable periodic sampling.

---

## Simulation Behavior

If `/dev/meascdd` fails to open, the system switches to simulation mode:

- Temperature → Temperature input from sensor
- ADC0 → ADC input from channel 0
- ADC1 → ADC input from channel 1
- Switches → Switch inputs from Zedboard
- Push button → Push button inputs from Zedboard

Simulation uses `CLOCK_MONOTONIC` for deterministic progression.

---

## Target Platform

- Embedded Linux
- ZedBoard
- Custom measurement hardware via kernel module

---

## Performance Characteristics

- Batch size optimized to ~1440 bytes (TCP payload friendly)
- Lock-protected producer-consumer model
- Minimal busy waiting (`usleep(50)` throttle)
- Partial send handling for robust TCP transmission

---

## Author

Haizon Helet Cruz  
2026-02-13

---

## License

Internal / Educational / Research Use
