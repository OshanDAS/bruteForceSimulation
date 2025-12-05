// bruteforce_parallel.c
// Compile with: gcc -fopenmp openmp_password_hash.c -lssl -lcrypto -o openmp_password_hash

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/md5.h>
#include <omp.h>

// Configuration
#define CHARSET "abcdefghijklmnopqrstuvwxyz"
#define CHARSET_SIZE 26
#define MAX_PASSWORD_LENGTH 10

// Convert a number to password string (base-26 conversion)
void number_to_password(unsigned long long num, char* password, int length) {
    for (int i = length - 1; i >= 0; i--) {
        password[i] = CHARSET[num % CHARSET_SIZE];
        num /= CHARSET_SIZE;
    }
    password[length] = '\0';
}

// Calculate total number of combinations for given password length
unsigned long long calculate_combinations(int length) {
    unsigned long long total = 1;
    for (int i = 0; i < length; i++) {
        total *= CHARSET_SIZE;
    }
    return total;
}

// Convert hash bytes to hexadecimal string for display
void hash_to_hex(unsigned char* hash, char* output) {
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[MD5_DIGEST_LENGTH * 2] = '\0';
}

// Generate MD5 hash of a password
void generate_hash(const char* password, unsigned char* hash) {
    MD5((unsigned char*)password, strlen(password), hash);
}

// ----------------------------------------------
// PARALLEL BRUTE FORCE USING OPENMP
// ----------------------------------------------
int crack_password_parallel(const char* target_password, int password_length) {
    unsigned long long total_combinations = calculate_combinations(password_length);
    int found = 0;
    unsigned long long found_at = 0;
    char found_password[MAX_PASSWORD_LENGTH + 1];
    unsigned long long attempts = 0;

    // Compute target hash
    unsigned char target_hash[MD5_DIGEST_LENGTH];
    generate_hash(target_password, target_hash);

    // Convert to hex for display
    char target_hash_hex[MD5_DIGEST_LENGTH * 2 + 1];
    hash_to_hex(target_hash, target_hash_hex);

    double start_time = omp_get_wtime();

    printf("\n=== Starting Parallel Brute Force Search (OpenMP) ===\n");
    printf("Target password: %s\n", target_password);
    printf("Target hash (MD5): %s\n", target_hash_hex);
    printf("Password length: %d\n", password_length);
    printf("Character set: %s\n", CHARSET);
    printf("Threads: %d\n", omp_get_max_threads());
    printf("Total combinations: %llu\n\n", total_combinations);

    // PARALLEL REGION
    #pragma omp parallel
    {
        char guess[MAX_PASSWORD_LENGTH + 1];
        unsigned char guess_hash[MD5_DIGEST_LENGTH];
        unsigned long long local_attempts = 0;

        #pragma omp for schedule(dynamic)
        for (unsigned long long i = 0; i < total_combinations; i++) {

            if (found) {
                #pragma omp cancel for
            }

            number_to_password(i, guess, password_length);
            generate_hash(guess, guess_hash);
            local_attempts++;

            if (memcmp(guess_hash, target_hash, MD5_DIGEST_LENGTH) == 0) {
                found = 1;
                found_at = i;
                strcpy(found_password, guess);
                #pragma omp cancel for
            }

            // Progress indicator (every 10000 attempts)
            if (local_attempts % 10000 == 0) {
                #pragma omp atomic
                attempts += local_attempts;
                local_attempts = 0;
                
                #pragma omp critical
                {
                    printf("Progress: %llu / %llu attempts (%.2f%%)\r", 
                           attempts, total_combinations, 
                           (attempts * 100.0) / total_combinations);
                    fflush(stdout);
                }
            }
        }
        
        // Add remaining local attempts
        #pragma omp atomic
        attempts += local_attempts;
    }

    double end_time = omp_get_wtime();
    double elapsed = end_time - start_time;

    if (found) {
        char found_hash_hex[MD5_DIGEST_LENGTH * 2 + 1];
        hash_to_hex(target_hash, found_hash_hex);

        printf("\n✓ PASSWORD FOUND!\n");
        printf("Password: %s\n", found_password);
        printf("Hash: %s\n", found_hash_hex);
        printf("Found at attempt: %llu\n", found_at + 1);
        printf("Execution time: %.3f seconds\n", elapsed);
        printf("Passwords per second: %.0f\n", attempts / elapsed);
        return 1;
    }

    printf("\n✗ Password NOT found\n");
    printf("Total attempts: %llu\n", attempts);
    printf("Execution time: %.3f seconds\n", elapsed);
    return 0;
}

int main() {
    printf("========================================\n");
    printf("Parallel Brute Force Password Cracker\n");
    printf("Using MD5 + OpenMP\n");
    printf("========================================\n");

    char password[MAX_PASSWORD_LENGTH + 1];

    printf("Enter password to crack (lowercase letters only): ");
    scanf("%s", password);

    int length = strlen(password);

    if (length > MAX_PASSWORD_LENGTH) {
        printf("Error: Password too long\n");
        return 1;
    }

    for (int i = 0; i < length; i++) {
        if (password[i] < 'a' || password[i] > 'z') {
            printf("Error: Password must contain only a-z lowercase\n");
            return 1;
        }
    }

    crack_password_parallel(password, length);

    return 0;
}
