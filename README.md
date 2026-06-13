# Embedkit_Kriti

**Candidate Name:** Kritika Barge  
**Assignment:** Embedded Developer Assignment (Fresher) - Ring Buffer Module  

## Module Description
* **ringbuf.c**: A lightweight, standalone circular FIFO buffer implementation handling fixed-width `uint8_t` data types with zero-overwrite protections. It includes optimized pointer wrap-around math engineered for low-cost microcontrollers lacking hardware division support.

## Build and Run Instructions
This module uses standard C libraries and has zero third-party dependencies.

### Compilation
Compile cleanly using `gcc` under C99 standards with the following command:
```bash
gcc -Wall -std=c99 ringbuf.c -o ringbuf
./ringbuf
