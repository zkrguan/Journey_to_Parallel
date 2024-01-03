// Workshop 9 - Discretize the Elements of an Array
 // Chris Szalwinski
 // 2018/11/18
 // w9.serial.c


//#define SERIAL
//#define PARA

#define TEST_CONFIG

#ifdef TEST_CONFIG
//#include <iostream>
//#include <mpi.h>
//// Don't forgot to send arguments
//int main(int argc, char** argv) {
//
//    int nTasks;
//    int iTask;
//    MPI_Init(&argc, &argv);
//    MPI_Comm_size(MPI_COMM_WORLD, &nTasks);
//    MPI_Comm_rank(MPI_COMM_WORLD, &iTask);
//    std::cout << "Hello from MPI" << "Total Tasks" << nTasks << " Ranks" << iTask << std::endl;
//    MPI_Finalize();
//}
#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    // Rank is the process_number( or is it safe to say Id?) 
    // Size is how many processes it will be there// 
    int rank, size;
    // As you notice we will use MPI_COMM_WORLD
    // The global communicator, rookie like me only only this one//
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // If rank is 0, meaning the main process, or manager process// 
    // Otherwise it is the worker processes, or child process. 
    if (rank == 0) {
        // Process 0 sends a message to Process 1
        int message = 42;
        // First arg, the mem address of the message
        // Second, how many items inside this message// 
        // Third, the type of the message. So the receiver will be able to parse it.
        // Forth, destination rank code, the main is 0, so the child is 1 
        // Fifth, tag, for god sakes, this is a tester sample, so I set it to 0. 
        //          But the big program will have many tags for orgnizaing . 
        // The last is destination's communicator, as the above line, we used the MPI_COMM_WORLD.
        MPI_Send(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("Process 0 sent message %d to Process 1.\n", message);
    }
    else if (rank == 1) {
        // Declare the variable and ready to receive message.. 
        int received_message;
        // First arg, the mem address of where to store teh message
        // Second, how many items inside this message
        // Third, the type of the message. So the receiver will be able to parse it.
        // Forth, the parent process is 0, so you are receiving from 0
        // Fifth, tag, for god sakes, this is a tester sample, so I set it to 0. 
        //          But the big program will have many tags for orgnizaing . 
        // The last is destination's communicator, as the above line, we used the MPI_COMM_WORLD.
        MPI_Recv(&received_message, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process 1 received message %d from Process 0.\n", received_message);
    }
    // Clean before you exit.
    MPI_Finalize();
    return 0;
}

#endif // TEST_CONFIG


#ifdef SERIAL

#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "time.h"

// report processor time
//
void reportTime(const char* msg, double span) {
    printf("%-14s took %7.4lf seconds\n", msg, span);
}

// discretize data[n] into 0s and 1s
//
void discretize(float* data, int n) {
    for (int i = 0; i < n; i++)
        data[i] = (pow(sin(data[i]), cos(data[i])) +
            pow(cos(data[i]), sin(data[i]))) / 2.0f;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "%s : invalid number of arguments\n"
            "Usage: %s no_of_elements\n", argv[0], argv[0]);
        return 1;
    }

    // retrieve number of elements
    int n = atoi(argv[1]);
    clock_t start, end;

    // initialization
    start = clock();
    float* data = (float*)malloc(n * sizeof(float));
    for (int i = 0; i < n; i++)
        data[i] = (float)rand() / RAND_MAX;
    end = clock();
    reportTime("Initialization", (double)(end - start) / CLOCKS_PER_SEC);

    // discretization and collation
    start = clock();
    discretize(data, n);
    end = clock();
    int time = end - start;
    reportTime("Conversion", (double)time / CLOCKS_PER_SEC);
    int zeros = 0;
    for (int i = 0; i < n; i++)
        if (data[i] < 0.707f)
            zeros++;

    // report
    printf("%d = %d 0s + %d 1s\n", n, zeros, n - zeros);

    // clean up
    free(data);
}
#endif // !PARA


#ifdef PARA
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "mpi.h"

#define TIME 1

void reportTime(const char* msg, int i, double span) {
    printf("Process %d: %-12s took %7.4lf seconds\n", i, msg, span);
}

void discretize(float* data, int n) {
    for (int i = 0; i < n; i++)
        data[i] = (pow(sin(data[i]), cos(data[i])) +
            pow(cos(data[i]), sin(data[i]))) / 2.0f;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "%s : invalid number of arguments\n"
            "Usage: %s no_of_elements\n", argv[0], argv[0]);
        return 1;
    }

    MPI_Init(&argc, &argv);

    int rank, np;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    int n = atoi(argv[1]);
    int nPerProcess = n / np;
    n = nPerProcess * np;
    int zeroes = 0;
    float* data = NULL;
    float* result = NULL;

    double begin, end;

    if (rank == 0) {
        // Master process
        data = (float*)malloc(n * sizeof(float));
        result = (float*)malloc(n * sizeof(float));

        for (int i = 0; i < n; i++)
            data[i] = (float)rand() / RAND_MAX;

        begin = MPI_Wtime();  // Start timing

        discretize(data, nPerProcess);

        // Separate buffers for sending and receiving
        float* sendbuf = (float*)malloc(n * sizeof(float));

        // Scatter data to all processes
        MPI_Scatter(data, nPerProcess, MPI_FLOAT, sendbuf, nPerProcess, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // Gather converted data from all processes
        MPI_Gather(sendbuf, nPerProcess, MPI_FLOAT, result, nPerProcess, MPI_FLOAT, 0, MPI_COMM_WORLD);

        end = MPI_Wtime();  // End timing

        for (int i = 0; i < nPerProcess; i++)
            if (data[i] < 0.707f)
                zeroes++;
        for (int i = nPerProcess; i < n; i++)
            if (result[i] < 0.707f)
                zeroes++;

        // Timing statistics
        reportTime("Conversion", 0, end - begin);

        for (int i = 1; i < np; i++) {
            MPI_Recv(&begin, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&end, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            reportTime("Conversion", i, end - begin);
        }
        free(sendbuf);
        reportTime("Sum of All", 0, end - begin);
    }
    else {
        // Worker processes
        data = (float*)malloc(nPerProcess * sizeof(float));
        result = (float*)malloc(nPerProcess * sizeof(float));

        // Separate buffers for sending and receiving
        float* sendbuf = (float*)malloc(nPerProcess * sizeof(float));

        MPI_Scatter(data, nPerProcess, MPI_FLOAT, sendbuf, nPerProcess, MPI_FLOAT, 0, MPI_COMM_WORLD);

        begin = MPI_Wtime();  // Start timing

        discretize(sendbuf, nPerProcess);

        MPI_Gather(sendbuf, nPerProcess, MPI_FLOAT, result, nPerProcess, MPI_FLOAT, 0, MPI_COMM_WORLD);

        end = MPI_Wtime();  // End timing
;
        free(sendbuf);

        // Timing information
        MPI_Send(&begin, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&end, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    if (rank == 0) {
        reportTime("Conversion Time - Wall clock", 0, end - begin);
        // get MPI deallocation time start
        begin = MPI_Wtime();
        free(data);
        free(result);
        // get MPI deallocation time end
        end = MPI_Wtime();
        reportTime("Deallocation", 0, end - begin);
        printf("Result: %d = %d (0s) + %d (1s)\n", n, zeroes, n - zeroes);
    }
    MPI_Finalize();

    return 0;
}


#endif // PARA

