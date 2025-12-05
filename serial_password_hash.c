//to run this program openssl should be installed , install it using the code below
//sudo apt-get install libssl-dev




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/md5.h>

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

// Serial brute force password search using MD5 hash comparison
int crack_password_serial(const char* target_password, int password_length) {
    unsigned long long total_combinations = calculate_combinations(password_length);
    char guess[MAX_PASSWORD_LENGTH];
    unsigned long long attempts = 0;
    int found = 0;
    
    // Generate target hash
    unsigned char target_hash[MD5_DIGEST_LENGTH];
    generate_hash(target_password, target_hash);
    
    // Convert to hex for display
    char target_hash_hex[MD5_DIGEST_LENGTH * 2 + 1];
    hash_to_hex(target_hash, target_hash_hex);
    
    printf("\n=== Starting Brute Force Search ===\n");
    printf("Target password: %s\n", target_password);
    printf("Target hash (MD5): %s\n", target_hash_hex);
    printf("Password length: %d\n", password_length);
    printf("Character set: %s\n", CHARSET);
    printf("Total combinations to try: %llu\n\n", total_combinations);
    
    // Start timing
    clock_t start_time = clock();
    
    // Try every possible combination
    for (unsigned long long i = 0; i < total_combinations; i++) {
        // Generate password candidate
        number_to_password(i, guess, password_length);
        attempts++;
        
        // Hash the guess
        unsigned char guess_hash[MD5_DIGEST_LENGTH];
        generate_hash(guess, guess_hash);
        
        // Compare hashes (comparing raw bytes, not hex strings)
        if (memcmp(guess_hash, target_hash, MD5_DIGEST_LENGTH) == 0) {
            found = 1;
            
            // Stop timing
            clock_t end_time = clock();
            double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
            
            // Convert found hash to hex for display
            char found_hash_hex[MD5_DIGEST_LENGTH * 2 + 1];
            hash_to_hex(guess_hash, found_hash_hex);
            
            printf("✓ PASSWORD FOUND!\n");
            printf("Password: %s\n", guess);
            printf("Hash: %s\n", found_hash_hex);
            printf("Found at attempt: %llu\n", attempts);
            printf("Execution time: %.3f seconds\n", elapsed_time);
            printf("Passwords per second: %.0f\n", attempts / elapsed_time);
            
            return 1;
        }
        
        // Progress indicator (every 10000 attempts)
        if (attempts % 10000 == 0) {
            printf("Progress: %llu / %llu attempts (%.2f%%)\r", 
                   attempts, total_combinations, 
                   (attempts * 100.0) / total_combinations);
            fflush(stdout);
        }
    }
    
    // Stop timing
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    if (!found) {
        printf("\n✗ Password NOT found\n");
        printf("Total attempts: %llu\n", attempts);
        printf("Execution time: %.3f seconds\n", elapsed_time);
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    printf("========================================\n");
    printf("Serial Brute Force Password Cracker\n");
    printf("Using MD5 Hash Comparison\n");
    printf("========================================\n");
    
    char password[MAX_PASSWORD_LENGTH + 1];
    
    printf("Enter password to crack (lowercase letters only): ");
    scanf("%s", password);
    
    int length = strlen(password);
    
    if (length > MAX_PASSWORD_LENGTH) {
        printf("Error: Password too long (max %d characters)\n", MAX_PASSWORD_LENGTH);
        return 1;
    }
    
    // Validate password contains only lowercase letters
    for (int i = 0; i < length; i++) {
        if (password[i] < 'a' || password[i] > 'z') {
            printf("Error: Password must contain only lowercase letters (a-z)\n");
            return 1;
        }
    }
    
    crack_password_serial(password, length);
    
    return 0;
}