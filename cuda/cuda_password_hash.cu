/*
 * CUDA Parallel Password Cracker
 * Uses MD5 Hash Comparison
 * 
 * ORIGINAL WORK:
 * - Parallelization strategy and work distribution
 * - Password generation from thread indices
 * - Memory management and kernel launch configuration
 * - Result synchronization mechanism
 * 
 * ADAPTED COMPONENT:
 * - MD5 hash function for GPU (see md5_device.cuh for attribution)
 * 
 * STANDARD LIBRARIES:
 * - OpenSSL MD5 for CPU hash generation (standard practice)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cuda_runtime.h>
#include <openssl/md5.h>
#include "md5_device.cuh"

// Configuration - ADJUSTABLE LIMITS
#define CHARSET "abcdefghijklmnopqrstuvwxyz"
#define CHARSET_SIZE 26
#define MAX_PASSWORD_LENGTH 10  // Can increase to 15+ if needed (memory allows up to 55)

// Warning thresholds for time estimation
#define WARN_THRESHOLD_COMBINATIONS 100000000000ULL  // 100 billion (>25 seconds)

/*
 * DEVICE FUNCTION: Convert thread index to password string
 * This is OUR ORIGINAL parallelization strategy
 * 
 * Each thread gets a unique index and converts it to a password candidate
 * Example: 0 -> "aaa", 1 -> "aab", 2 -> "aac", 26 -> "aba"
 */
__device__ void index_to_password(unsigned long long index, char* password, int length) {
    for (int i = length - 1; i >= 0; i--) {
        password[i] = CHARSET[index % CHARSET_SIZE];
        index /= CHARSET_SIZE;
    }
    password[length] = '\0';
}

/*
 * DEVICE FUNCTION: Prepare password for MD5 hashing
 * MD5 expects input in specific format (padded blocks of 512 bits)
 * 
 * This is OUR ORIGINAL code for formatting passwords for MD5
 */
__device__ void prepare_md5_input(const char* password, int length, unsigned int* buffer) {
    // Clear buffer
    for (int i = 0; i < 16; i++) {
        buffer[i] = 0;
    }
    
    // Copy password bytes into buffer (little-endian format)
    for (int i = 0; i < length; i++) {
        int word_index = i / 4;
        int byte_index = i % 4;
        buffer[word_index] |= ((unsigned int)password[i]) << (byte_index * 8);
    }
    
    // Append MD5 padding bit (0x80)
    int word_index = length / 4;
    int byte_index = length % 4;
    buffer[word_index] |= 0x80 << (byte_index * 8);
    
    // Append length in bits at the end (MD5 requires this)
    buffer[14] = length * 8;  // Length in bits (lower 32 bits)
    buffer[15] = 0;            // Length in bits (upper 32 bits, always 0 for short passwords)
}

/*
 * CUDA KERNEL: Parallel Password Cracking
 * 
 * THIS IS OUR ORIGINAL PARALLELIZATION IMPLEMENTATION
 * 
 * Strategy:
 * - Each thread checks multiple password candidates
 * - Work is evenly distributed across all threads
 * - Early exit when password is found (all threads check flag)
 * - Atomic operations ensure thread-safe result storage
 */
__global__ void crack_password_kernel(
    unsigned long long start_index,          // Starting index for this kernel launch
    unsigned long long passwords_per_thread, // How many passwords each thread checks
    int password_length,                      // Length of password to crack
    unsigned int* target_hash,                // Target MD5 hash (4 x 32-bit integers)
    int* found_flag,                          // Shared flag: set to 1 when found
    char* result_password,                    // Where to store found password
    unsigned long long* found_at_index,       // Store the index where password was found
    unsigned long long total_combinations     // Total number of combinations to check
) {
    // Calculate this thread's unique global ID
    unsigned long long thread_id = blockIdx.x * blockDim.x + threadIdx.x;
    
    // Calculate starting index for this specific thread
    unsigned long long my_start = start_index + (thread_id * passwords_per_thread);
    
    // Bounds check: skip threads that are beyond the search space
    if (my_start >= total_combinations) return;
    
    // Thread-local storage for password generation and hashing
    char password[MAX_PASSWORD_LENGTH + 1];
    unsigned int md5_input[16];   // MD5 input buffer (512 bits)
    unsigned int computed_hash[4]; // MD5 output (128 bits)
    
    // Each thread checks multiple passwords
    for (unsigned long long i = 0; i < passwords_per_thread; i++) {
        // Early exit optimization: if another thread found it, stop
        if (*found_flag) {
            return;
        }
        
        unsigned long long current_index = my_start + i;
        
        // Bounds check: don't go past total combinations
        if (current_index >= total_combinations) {
            return;
        }
        
        // ORIGINAL: Convert index to password candidate
        index_to_password(current_index, password, password_length);
        
        // ORIGINAL: Prepare password for MD5
        prepare_md5_input(password, password_length, md5_input);
        
        // ADAPTED: Compute MD5 hash (using adapted function)
        md5_cuda(md5_input, computed_hash);
        
        // ORIGINAL: Compare computed hash with target hash
        if (computed_hash[0] == target_hash[0] && 
            computed_hash[1] == target_hash[1] &&
            computed_hash[2] == target_hash[2] && 
            computed_hash[3] == target_hash[3]) {
            
            // Found it! Use atomic operation to ensure only one thread writes
            int old = atomicCAS(found_flag, 0, 1);
            
            if (old == 0) {  // This thread was first to find it
                // Store the index where we found it
                *found_at_index = current_index;
                
                // Copy password to result buffer
                for (int j = 0; j <= password_length; j++) {
                    result_password[j] = password[j];
                }
            }
            return;
        }
    }
}

/*
 * HOST FUNCTION: Compute MD5 hash on CPU using OpenSSL
 * This is a one-time operation to generate the target hash
 */
void compute_target_hash(const char* password, unsigned int* hash) {
    unsigned char hash_bytes[MD5_DIGEST_LENGTH];
    
    // Use OpenSSL MD5 (standard library, runs on CPU)
    MD5((unsigned char*)password, strlen(password), hash_bytes);
    
    // Convert byte array to 32-bit integers (little-endian format)
    // This matches the format used by our GPU MD5 implementation
    for (int i = 0; i < 4; i++) {
        hash[i] = (hash_bytes[i*4 + 0]) |
                  (hash_bytes[i*4 + 1] << 8) |
                  (hash_bytes[i*4 + 2] << 16) |
                  (hash_bytes[i*4 + 3] << 24);
    }
}

/*
 * HOST FUNCTION: Convert hash to hex string for display
 */
void hash_to_hex(unsigned int* hash, char* hex_string) {
    unsigned char* bytes = (unsigned char*)hash;
    for (int i = 0; i < 16; i++) {
        sprintf(hex_string + (i * 2), "%02x", bytes[i]);
    }
    hex_string[32] = '\0';
}

/*
 * HOST FUNCTION: Main password cracking orchestrator
 * 
 * THIS IS OUR ORIGINAL WORK: kernel launch strategy and optimization
 */
int crack_password_cuda(const char* target_password, int password_length, int threads_per_block, int num_blocks) {
    // Calculate total combinations
    unsigned long long total_combinations = 1;
    for (int i = 0; i < password_length; i++) {
        total_combinations *= CHARSET_SIZE;
    }
    
    printf("\n=== CUDA Password Cracker ===\n");
    printf("Target password: %s\n", target_password);
    printf("Password length: %d\n", password_length);
    printf("Character set: %s (%d chars)\n", CHARSET, CHARSET_SIZE);
    printf("Total combinations: %llu\n", total_combinations);
    
    // Estimate time and warn if it will take too long
    float estimated_seconds = total_combinations / 4000000000.0;  // Assuming 4B passwords/sec
    if (estimated_seconds > 60) {
        printf("\n⚠️  WARNING: This will take approximately %.1f minutes!\n", estimated_seconds / 60.0);
        if (estimated_seconds > 3600) {
            printf("⚠️  That's %.1f hours! Consider using a shorter password for testing.\n", estimated_seconds / 3600.0);
        }
        printf("\n");
    };
    
    // Generate target hash using OpenSSL (CPU)
    unsigned int target_hash[4];
    compute_target_hash(target_password, target_hash);
    
    // Display target hash in hex format
    char hex_hash[33];
    hash_to_hex(target_hash, hex_hash);
    printf("Target MD5 hash: %s\n\n", hex_hash);
    
    // Allocate device memory
    unsigned int* d_target_hash;
    int* d_found_flag;
    char* d_result_password;
    unsigned long long* d_found_at_index;
    
    cudaMalloc(&d_target_hash, 4 * sizeof(unsigned int));
    cudaMalloc(&d_found_flag, sizeof(int));
    cudaMalloc(&d_result_password, (MAX_PASSWORD_LENGTH + 1) * sizeof(char));
    cudaMalloc(&d_found_at_index, sizeof(unsigned long long));
    
    // Initialize device memory
    cudaMemcpy(d_target_hash, target_hash, 4 * sizeof(unsigned int), cudaMemcpyHostToDevice);
    int zero = 0;
    cudaMemcpy(d_found_flag, &zero, sizeof(int), cudaMemcpyHostToDevice);
    unsigned long long zero_ull = 0;
    cudaMemcpy(d_found_at_index, &zero_ull, sizeof(unsigned long long), cudaMemcpyHostToDevice);
    
    // ORIGINAL: Configure kernel launch parameters
    // This is OUR parallelization strategy

    unsigned long long passwords_per_thread = 100;  // Each thread checks 100 passwords
    
    // Calculate number of blocks
    int blocks;
    if (num_blocks > 0) {
        blocks = num_blocks;  // Use user-specified blocks
    } else {
        // Auto-calculate blocks based on total work
        unsigned long long threads_needed = (total_combinations + passwords_per_thread - 1) / passwords_per_thread;
        blocks = (threads_needed + threads_per_block - 1) / threads_per_block;
    }
    
    // Create CUDA events for timing
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    
    // Start timing
    cudaEventRecord(start);
    
    // ORIGINAL: Launch kernel with OUR parallelization strategy
    crack_password_kernel<<<blocks, threads_per_block>>>(
        0,                      // Start from index 0
        passwords_per_thread,   // Each thread checks this many
        password_length,
        d_target_hash,
        d_found_flag,
        d_result_password,
        d_found_at_index,
        total_combinations
    );
    
    // Check for kernel launch errors
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("Kernel launch error: %s\n", cudaGetErrorString(err));
        return 0;
    }
    
    // Wait for kernel to complete
    cudaDeviceSynchronize();
    
    // Stop timing
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);
    
    // Check results
    int found;
    char result_password[MAX_PASSWORD_LENGTH + 1];
    unsigned long long found_at_index;
    
    cudaMemcpy(&found, d_found_flag, sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(result_password, d_result_password, (MAX_PASSWORD_LENGTH + 1) * sizeof(char), cudaMemcpyDeviceToHost);
    cudaMemcpy(&found_at_index, d_found_at_index, sizeof(unsigned long long), cudaMemcpyDeviceToHost);
    
    // Display results in same format as serial version
    if (found) {
        // Calculate percentage
        double percentage = (found_at_index * 100.0) / total_combinations;
        
        printf("✓ PASSWORD FOUND! %llu / %llu attempts (%.2f%%)\n", 
               found_at_index + 1, total_combinations, percentage);
        printf("Password: %s\n", result_password);
        
        // Display hash
        char result_hex[33];
        unsigned int result_hash[4];
        compute_target_hash(result_password, result_hash);
        hash_to_hex(result_hash, result_hex);
        printf("Hash: %s\n", result_hex);
        
        printf("Found at attempt: %llu\n", found_at_index + 1);
        printf("Execution time: %.3f seconds\n", milliseconds / 1000.0);
        printf("Passwords per second: %.0f\n", 
               (total_combinations / (milliseconds / 1000.0)));
    } else {
        printf("✗ Password NOT found\n");
        printf("Execution time: %.3f seconds\n", milliseconds / 1000.0);
        printf("\nThis shouldn't happen - all combinations were checked.\n");
        printf("Possible issues: GPU memory error or hash mismatch.\n");
    }
    
    // Cleanup
    cudaFree(d_target_hash);
    cudaFree(d_found_flag);
    cudaFree(d_result_password);
    cudaFree(d_found_at_index);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    
    return found;
}

/*
 * Test function to verify MD5 implementation
 */
void test_md5_implementation() {
    printf("\n=== Testing MD5 Implementation ===\n");
    
    // Test case 1: "abc"
    const char* test1 = "abc";
    unsigned int hash1[4];
    compute_target_hash(test1, hash1);
    
    char hex1[33];
    hash_to_hex(hash1, hex1);
    
    printf("MD5(\"%s\") = %s\n", test1, hex1);
    printf("Expected:       900150983cd24fb0d6963f7d28e17f72\n");
    
    // Test case 2: "test"
    const char* test2 = "test";
    unsigned int hash2[4];
    compute_target_hash(test2, hash2);
    
    char hex2[33];
    hash_to_hex(hash2, hex2);
    
    printf("\nMD5(\"%s\") = %s\n", test2, hex2);
    printf("Expected:       098f6bcd4621d373cade4e832627b4f6\n");
    
    printf("\n");
}

int main(int argc, char* argv[]) {
    printf("========================================\n");
    printf("CUDA Parallel Password Cracker\n");
    printf("Using MD5 Hash Comparison\n");
    printf("========================================\n");
    
    char password[MAX_PASSWORD_LENGTH + 1];
    
    printf("Enter password to crack (lowercase letters only): ");
    scanf("%s", password);
    
    int length = strlen(password);
    
    if (length > MAX_PASSWORD_LENGTH) {
        printf("Error: Password too long!\n");
        return 1;
    }
    
    if (length == 0) {
        printf("Error: Password cannot be empty!\n");
        return 1;
    }
    
    // Validate password contains only lowercase letters
    for (int i = 0; i < length; i++) {
        if (password[i] < 'a' || password[i] > 'z') {
            printf("Error: Password must contain only lowercase letters (a-z)\n");
            return 1;
        }
    }
    // Get threads per block and blocks from command line or use defaults
    int threads_per_block = 256;
    int num_blocks = 0;  // 0 means auto-calculate
    
    if (argc >= 2) {
        threads_per_block = atoi(argv[1]);
    }
    if (argc >= 3) {
        num_blocks = atoi(argv[2]);
    }
    
    if (argc >= 2) {
        printf("Using %d threads per block", threads_per_block);
        if (num_blocks > 0) {
            printf(", %d blocks\n", num_blocks);
        } else {
            printf(" (auto blocks)\n");
        }
    }
    
    // Run CUDA password cracker
    crack_password_cuda(password, length, threads_per_block, num_blocks);
    
    return 0;
}