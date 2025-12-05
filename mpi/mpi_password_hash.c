// Compile:
// mpicc -O3 mpi_password_hash.c -lssl -lcrypto -o mpi_password_hash
//
// Run:
// mpirun -np 8 ./mpi_password_hash

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

#define CHARSET "abcdefghijklmnopqrstuvwxyz"
#define CHARSET_SIZE 26
#define MAX_PASSWORD_LENGTH 10
#define TERMINATE_TAG 999
#define PROGRESS_TAG 998

// ---------------------------------------------
// Convert number → password string
// ---------------------------------------------
void number_to_password(unsigned long long num, char *password, int length) {
    for (int i = length - 1; i >= 0; i--) {
        password[i] = CHARSET[num % CHARSET_SIZE];
        num /= CHARSET_SIZE;
    }
    password[length] = '\0';
}

// ---------------------------------------------
// Count combinations = CHARSET_SIZE^length
// ---------------------------------------------
unsigned long long calculate_combinations(int length) {
    unsigned long long total = 1;
    for (int i = 0; i < length; i++)
        total *= CHARSET_SIZE;
    return total;
}

// ---------------------------------------------
// Generate MD5 hash
// ---------------------------------------------
void generate_hash(const char *password, unsigned char *hash) {
    MD5((unsigned char*)password, strlen(password), hash);
}

// ---------------------------------------------
// Brute-force search with clean termination
// ---------------------------------------------
int mpi_crack(const unsigned char *target_hash, int length,
              int rank, int world_size) {

    unsigned long long total = calculate_combinations(length);

    char guess[MAX_PASSWORD_LENGTH + 1];
    unsigned char guess_hash[MD5_DIGEST_LENGTH];

    int terminate_flag;
    MPI_Status status;
    MPI_Request progress_req;

    unsigned long long check_interval = 50000;
    unsigned long long counter = 0;
    unsigned long long local_count = 0;

    for (unsigned long long i = rank; i < total; i += world_size) {

        // ----------- CHECK FOR TERMINATION & SEND PROGRESS ----------
        if (++counter % check_interval == 0) {
            int flag = 0;
            MPI_Iprobe(MPI_ANY_SOURCE, TERMINATE_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                MPI_Recv(&terminate_flag, 1, MPI_INT, status.MPI_SOURCE,
                         TERMINATE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                return 0;
            }
            
            if (rank != 0) {
                local_count = counter;
                MPI_Isend(&local_count, 1, MPI_UNSIGNED_LONG_LONG, 0, PROGRESS_TAG, MPI_COMM_WORLD, &progress_req);
            }
        }
        
        // ----------- RANK 0: COLLECT PROGRESS ----------
        if (rank == 0 && counter % check_interval == 0) {
            unsigned long long total_progress = counter;
            int flag;
            unsigned long long worker_count;
            
            for (int p = 1; p < world_size; p++) {
                MPI_Iprobe(p, PROGRESS_TAG, MPI_COMM_WORLD, &flag, &status);
                if (flag) {
                    MPI_Recv(&worker_count, 1, MPI_UNSIGNED_LONG_LONG, p, PROGRESS_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    total_progress += worker_count;
                }
            }
            
            printf("Progress: %llu / %llu (%.2f%%)\r", total_progress, total, (total_progress * 100.0) / total);
            fflush(stdout);
        }

        // ----------- GENERATE GUESS ----------
        number_to_password(i, guess, length);
        generate_hash(guess, guess_hash);

        // ----------- CHECK MATCH ----------
        if (memcmp(guess_hash, target_hash, MD5_DIGEST_LENGTH) == 0) {
            
            printf("\nRank %d FOUND the password!\n", rank);
            printf("Password = %s\n", guess);

            // Send termination message to all other ranks
            int flag = 1;
            for (int p = 0; p < world_size; p++) {
                if (p != rank) {
                    MPI_Send(&flag, 1, MPI_INT, p, TERMINATE_TAG, MPI_COMM_WORLD);
                }
            }

            return 1;
        }
    }

    return 0;
}

// ---------------------------------------------
// MAIN
// ---------------------------------------------
int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);

    int rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    char password[MAX_PASSWORD_LENGTH + 1];

    // Only rank 0 reads input
    if (rank == 0) {
        printf("Enter password to crack: ");
        fflush(stdout);

        if (scanf("%s", password) != 1) {
            printf("Error reading password.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Broadcast password to all ranks
    MPI_Bcast(password, MAX_PASSWORD_LENGTH + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    int length = strlen(password);

    // Prepare target hash
    unsigned char target_hash[MD5_DIGEST_LENGTH];
    generate_hash(password, target_hash);

    double start = MPI_Wtime();

    mpi_crack(target_hash, length, rank, world_size);

    double end = MPI_Wtime();

    if (rank == 0) {
        printf("\nTime elapsed: %.6f seconds\n", end - start);
    }

    MPI_Finalize();
    return 0;
}
