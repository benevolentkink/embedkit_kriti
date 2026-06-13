#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE (8U)

// Structure representing the circular/ring buffer
typedef struct {
    uint8_t buffer[BUFFER_SIZE]; // Fixed capacity of 8 bytes
    uint32_t head;   // Index where the next write operation occurs
    uint32_t tail;   // Index where the next read operation occurs
    uint32_t count;  // Number of active bytes currently stored
} RingBuffer_t;

// Function Prototypes
void ringbuf_init(RingBuffer_t *rb);
bool ringbuf_is_full(const RingBuffer_t *rb);
bool ringbuf_is_empty(const RingBuffer_t *rb);
uint32_t ringbuf_get_count(const RingBuffer_t *rb);
bool ringbuf_write(RingBuffer_t *rb, uint8_t data);
bool ringbuf_read(RingBuffer_t *rb, uint8_t *data);

/**
 * @brief Initializes the ring buffer to an empty state.
 */
void ringbuf_init(RingBuffer_t *rb) {
    if (rb != NULL) {
        rb->head = 0;
        rb->tail = 0;
        rb->count = 0;
        for (uint32_t i = 0; i < BUFFER_SIZE; i++) {
            rb->buffer[i] = 0;
        }
    }
}

/**
 * @brief Check whether the buffer is completely full.
 */
bool ringbuf_is_full(const RingBuffer_t *rb) {
    return (rb != NULL) ? (rb->count == BUFFER_SIZE) : false;
}

/**
 * @brief Check whether the buffer is completely empty.
 */
bool ringbuf_is_empty(const RingBuffer_t *rb) {
    return (rb != NULL) ? (rb->count == 0) : true;
}

/**
 * @brief Query how many bytes are currently stored in the buffer.
 */
uint32_t ringbuf_get_count(const RingBuffer_t *rb) {
    return (rb != NULL) ? rb->count : 0;
}

/**
 * @brief Write one byte into the buffer.
 * @return true if successful, false if the buffer is full.
 */
bool ringbuf_write(RingBuffer_t *rb, uint8_t data) {
    // If the buffer is already full, fail and return an error code (never overwrite)
    if (rb == NULL || ringbuf_is_full(rb)) {
        return false;
    }

    rb->buffer[rb->head] = data;

    /* * BONUS TASK OPTIMIZATION: 
     * Replaced '% BUFFER_SIZE' with '& (BUFFER_SIZE - 1)'.
     * * Why this only works when BUFFER_SIZE is a power of 2:
     * When N is a power of 2 (e.g., 8 = 0b00001000), (N - 1) creates a perfect bitmask of 
     * all 1s below it (e.g., 7 = 0b00000111). Performing a bitwise AND with this mask 
     * keeps the lower bits intact and automatically wraps higher bits to 0 when 
     * the index reaches N. If BUFFER_SIZE is not a power of 2, the bitmask won't represent 
     * contiguous lower bits, which breaks wrapping.
     * * Why this is faster on MCUs without a hardware divider:
     * The modulo operator (%) compiles to a division instruction (e.g., DIV). On 
     * many microcontrollers lacking a hardware divider, division must be emulated in software, 
     * which consumes dozens of CPU cycles. A bitwise AND (&) is a primitive, single-cycle 
     * instruction executed directly by the ALU, vastly optimizing performance inside tight ISRs.
     */
    rb->head = (rb->head + 1) & (BUFFER_SIZE - 1);
    rb->count++;
    
    return true;
}

/**
 * @brief Read one byte from the buffer.
 * @return true if successful, false if empty (never return garbage data).
 */
bool ringbuf_read(RingBuffer_t *rb, uint8_t *data) {
    if (rb == NULL || data == NULL || ringbuf_is_empty(rb)) {
        return false;
    }

    *data = rb->buffer[rb->tail];
    
    // Bonus optimization applied to tail path wrap-around
    rb->tail = (rb->tail + 1) & (BUFFER_SIZE - 1);
    rb->count--;

    return true;
}

int main(void) {
    RingBuffer_t my_rb;
    ringbuf_init(&my_rb);
    
    uint8_t data_to_write[] = {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48};
    uint8_t read_byte = 0;

    // 1. Write 8 bytes one at a time and confirm full
    for (uint32_t i = 0; i < 8; i++) {
        if (ringbuf_write(&my_rb, data_to_write[i])) {
            printf("[WRITE] 0x%02X -> OK (count=%u)%s\n", 
                   data_to_write[i], 
                   ringbuf_get_count(&my_rb),
                   ringbuf_is_full(&my_rb) ? " FULL" : "");
        }
    }

    // 2. Attempt to write 0x99 to full buffer
    if (!ringbuf_write(&my_rb, 0x99)) {
        printf("[WRITE] 0x99 -> FAIL (buffer full)\n");
    }

    // 3. Read 3 bytes one at a time
    for (uint32_t i = 0; i < 3; i++) {
        if (ringbuf_read(&my_rb, &read_byte)) {
            printf("[READ] -> 0x%02X (count=%u)\n", read_byte, ringbuf_get_count(&my_rb));
        }
    }

    // 4. Write 3 new bytes (0x49, 0x4A, 0x4B) to reuse freed slots
    uint8_t new_data[] = {0x49, 0x4A, 0x4B};
    for (uint32_t i = 0; i < 3; i++) {
        if (ringbuf_write(&my_rb, new_data[i])) {
            printf("[WRITE] 0x%02X -> OK (count=%u)\n", new_data[i], ringbuf_get_count(&my_rb));
        }
    }

    // 5. Read all remaining bytes to empty the buffer
    while (!ringbuf_is_empty(&my_rb)) {
        if (ringbuf_read(&my_rb, &read_byte)) {
            printf("[READ] -> 0x%02X (count=%u)\n", read_byte, ringbuf_get_count(&my_rb));
        }
    }

    // 6. Attempt to read from empty buffer
    if (!ringbuf_read(&my_rb, &read_byte)) {
        printf("[READ] (empty) -> FAIL (buffer empty)\n");
    }

    return 0;
}
