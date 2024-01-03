// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//#define SIMPLE
//#define SERIAL
#define MPI_SOLUTION
//#define TEST_CONFIG

#ifdef TEST_CONFIG
#include <iostream>
#include <mpi.h>
// Don't forgot to send arguments
int main(int argc, char** argv) {

    int nTasks;
    int iTask;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nTasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &iTask);
    std::cout << "Hello from MPI" << "Total Tasks" << nTasks << " Ranks" << iTask << std::endl;
    MPI_Finalize();
}
#endif // TEST_CONFIG


#ifdef SIMPLE
// Workshop 8 - Domain Decomposition
// based on code from LLNL tutorial mpi_heat2d.c
// Master-Worker Programming Model
// Chris Szalwinski - 2018/11/13
// w8.simple.cpp
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <chrono>
using namespace std::chrono;

void initialize(int, int, float*);
void update(int, int, int, const float, const float, const float*, float*);
void output(int, int, const float*, const char*);

// report system time
//
void reportTime(const char* msg, steady_clock::duration span) {
    auto ms = duration_cast<milliseconds>(span);
    std::cout << msg << " - took - " <<
        ms.count() << " milliseconds" << std::endl;
}

// weights
const float wx = 0.1f;
const float wy = 0.1f;

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "*** Incorrect number of arguments ***\n";
        std::cerr << "Usage: " << argv[0]
            << " no-of_rows no_of_columns no_of_iterations\n";
        return 1;
    }

    // grid size
    int nRows = std::atoi(argv[1]);
    int nColumns = std::atoi(argv[2]);
    int nIterations = std::atoi(argv[3]);

    // allocate memory for decomposition
    steady_clock::time_point ts, te;
    float* data;
    try {
        // 2 * for double buffering
        data = new float[2 * nRows * nColumns];
    }
    catch (std::bad_alloc) {
        std::cerr << "*** Failed to Allocate Memory for 2 * "
            << nRows << " by " << nColumns << " grid" << std::endl;
        return 3;
    }

    // initialize first buffer data[0: nRowsTotal*nColumns)
    initialize(nRows, nColumns, data);
    // initialize second buffer data[nRowsTotal*nColumns : nRowsTotal*nColumns)
    for (int i = 0; i < nRows * nColumns; i++) data[nRows * nColumns + i] = 0.0f;

    // write original data to file
    output(nRows, nColumns, data, "original.dat");

    // decompose the data and iterate on each partition
    ts = steady_clock::now();
    int iz = 0;
    for (int i = 0; i < nIterations; i++) {
        std::cout << "Iteration " << i + 1 << std::endl;
        int rowOffset = 0;
        update(1, nRows - 2, nColumns, wx, wy,
            data + iz * nRows * nColumns, data + (1 - iz) * nRows * nColumns);
        // switch buffers
        iz = 1 - iz;
    }
    te = steady_clock::now();
    reportTime("\nw8 Domain Decomposition ", te - ts);
    std::cout << "Number of Rows       " << nRows << std::endl;
    std::cout << "Number of Columns    " << nColumns << std::endl;

    // write results to file
    output(nRows, nColumns, data, "results.dat");

    // checksum
    double sum1 = 0.0, sum2 = 0.0;
    for (int i = 0; i < nRows * nColumns; ++i) {
        sum1 += data[i];
        sum2 += data[i + nRows * nColumns];
    }
    std::cout << "Checksums = " << sum1 << ',' << sum2 << std::endl;

    // deallocate
    delete[] data;
}

// initialize for high value at above end middle row
//
void initialize(int nRowsTotal, int nColumns, float* x) {
    for (int i = 0; i < nRowsTotal; i++)
        for (int j = 0; j < nColumns; j++)
            x[i * nColumns + j] = (float)(i * (nRowsTotal - i - 1) * j
                * (nColumns - j - 1));
}

// update data using weighted neighboring values
//
void update(int startRow, int endRow, int nColumns, const float wx,
    const float wy, const float* x_old, float* x_new) {
    for (int i = startRow; i <= endRow; i++)
        for (int j = 1; j < nColumns - 1; j++)
            x_new[i * nColumns + j] = x_old[i * nColumns + j]
            + wx * (x_old[(i + 1) * nColumns + j] + x_old[(i - 1) * nColumns + j]
                - 2.0f * x_old[i * nColumns + j])
            + wy * (x_old[i * nColumns + j + 1] + x_old[i * nColumns + j - 1]
                - 2.0f * x_old[i * nColumns + j]);
}

// output data
//
void output(int nRowsTotal, int nColumns, const float* x, const char* filename) {
    std::ofstream file(filename);
    file << std::fixed << std::setprecision(1);
    for (int j = nColumns - 1; j >= 0; j--) {
        for (int i = 0; i < nRowsTotal; i++)
            file << std::setw(8) << x[i * nColumns + j];
        file << std::endl;
    }
}

#endif // SIMPLE

#ifdef SERIAL
// Workshop 8 - Domain Decomposition
 // based on code from LLNL tutorial mpi_heat2d.c
 // Master-Worker Programming Model
 // Chris Szalwinski - 2018/11/13
 // w8.serial.cpp
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <chrono>
using namespace std::chrono;

void initialize(int, int, float*);
void update(int, int, int, const float, const float, const float*, float*);
void output(int, int, const float*, const char*);

// report system time
//
void reportTime(const char* msg, steady_clock::duration span) {
    auto ms = duration_cast<milliseconds>(span);
    std::cout << msg << " - took - " <<
        ms.count() << " milliseconds" << std::endl;
}

// solution constants
const int NONE = 0;
const int MINPARTITIONS = 1;
const int MAXPARTITIONS = 7;
// weights
const float wx = 0.1f;
const float wy = 0.1f;

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "*** Incorrect number of arguments ***\n";
        std::cerr << "Usage: " << argv[0]
            << " no_of_partitions no-of_rows no_of_columns no_of_iterations\n";
        return 1;
    }

    // grid size
    int nPartitions = std::atoi(argv[1]);
    if (nPartitions < MINPARTITIONS || nPartitions > MAXPARTITIONS) {
        std::cerr << "*** Number of partitions out of bounds ***\n";
        std::cerr << "Bounds: " << MINPARTITIONS - 1 << " < no_of_partitions <"
            << MAXPARTITIONS + 1 << std::endl;
        return 2;
    }
    int nRowsTotal = std::atoi(argv[2]);
    int nColumns = std::atoi(argv[3]);
    int nIterations = std::atoi(argv[4]);
    int nRowsPerWorker = (nRowsTotal + nPartitions - 1) / nPartitions;
    int nRowsLeftOver = (nPartitions) ? nRowsTotal - (nPartitions - 1)
        * nRowsPerWorker : nRowsTotal;

    // allocate memory for decomposition
    steady_clock::time_point ts, te;
    float* data;
    try {
        // 2 * for double buffering
        data = new float[2 * nRowsTotal * nColumns];
    }
    catch (std::bad_alloc) {
        std::cerr << "*** Failed to Allocate Memory for 2 * "
            << nRowsTotal << " by " << nColumns << " grid" << std::endl;
        return 3;
    }

    // initialize first buffer data[0: nRowsTotal*nColumns)
    initialize(nRowsTotal, nColumns, data);
    // initialize second buffer data[nRowsTotal*nColumns : nRowsTotal*nColumns)
    for (int i = 0; i < nRowsTotal * nColumns; i++)
        data[nRowsTotal * nColumns + i] = 0.0f;

    // write original data to file
    output(nRowsTotal, nColumns, data, "original.dat");

    // decompose the data and iterate on each partition
    ts = steady_clock::now();
    int iz = 0;
    for (int i = 0; i < nIterations; i++) {
        std::cout << "Iteration " << i + 1 << std::endl;
        int rowOffset = 0;
        for (int iWorker = 0; iWorker < nPartitions; iWorker++) {
            int nRows, above, below;
            // identify the neighboring partitions
            if (iWorker == 0)
                above = NONE;
            else
                above = iWorker - 1;
            if (iWorker == nPartitions - 1) {
                nRows = nRowsLeftOver;
                below = NONE;
            }
            else {
                nRows = nRowsPerWorker;
                below = iWorker + 1;
            }
            // set row indices for updating - do not update the first or last row 
            int startRow = rowOffset;
            int endRow = rowOffset + nRows - 1;
            if (rowOffset == 0) startRow = 1;
            if (rowOffset + nRows == nRowsTotal) endRow--;
            update(startRow, endRow, nColumns, wx, wy,
                data + iz * nRowsTotal * nColumns,
                data + (1 - iz) * nRowsTotal * nColumns);
            std::cout << " Sent to worker " << iWorker << " " << nRows
                << " rows [row rowOffset " << rowOffset << "] above "
                << above << " below " << below << std::endl;
            rowOffset += nRows;
        }
        // switch buffers
        iz = 1 - iz;
    }
    te = steady_clock::now();
    reportTime("\nw8 Domain Decomposition ", te - ts);
    std::cout << "Number of Partitions " << nPartitions << std::endl;
    std::cout << "Number of Rows       " << nRowsTotal << std::endl;
    std::cout << "Number of Columns    " << nColumns << std::endl;

    // write results to file
    output(nRowsTotal, nColumns, data, "results.dat");

    // checksum
    double sum1 = 0.0, sum2 = 0.0;
    for (int i = 0; i < nRowsTotal * nColumns; ++i) {
        sum1 += data[i];
        sum2 += data[i + nRowsTotal * nColumns];
    }
    std::cout << "Checksums = " << sum1 << ',' << sum2 << std::endl;

    // deallocate
    delete[] data;
}

// initialize for high value at above end middle row
//
void initialize(int nRowsTotal, int nColumns, float* x) {
    for (int i = 0; i < nRowsTotal; i++)
        for (int j = 0; j < nColumns; j++)
            x[i * nColumns + j] = (float)(i * (nRowsTotal - i - 1) * j
                * (nColumns - j - 1));
}

// update data using weighted neighboring values
//
void update(int startRow, int endRow, int nColumns, const float wx,
    const float wy, const float* x_old, float* x_new) {
    for (int i = startRow; i <= endRow; i++)
        for (int j = 1; j < nColumns - 1; j++)
            x_new[i * nColumns + j] = x_old[i * nColumns + j]
            + wx * (x_old[(i + 1) * nColumns + j] + x_old[(i - 1) * nColumns + j]
                - 2.0f * x_old[i * nColumns + j])
            + wy * (x_old[i * nColumns + j + 1] + x_old[i * nColumns + j - 1]
                - 2.0f * x_old[i * nColumns + j]);
}

// output data
//
void output(int nRowsTotal, int nColumns, const float* x, const char* filename) {
    std::ofstream file(filename);
    file << std::fixed << std::setprecision(1);
    for (int j = nColumns - 1; j >= 0; j--) {
        for (int i = 0; i < nRowsTotal; i++)
            file << std::setw(8) << x[i * nColumns + j];
        file << std::endl;
    }
}

#endif // DEBUG

#ifdef MPI_SOLUTION
// Workshop 8 - Domain Decomposition
 // based on code from LLNL tutorial mpi_heat2d.c
 // Master-Worker Programming Model
 // Chris Szalwinski - 2018/11/13
 // w8.mpi.cpp
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <chrono>
#include <mpi.h>

using namespace std::chrono;

void initialize(int, int, float*);
void update(int, int, int, const float, const float, const float*, float*);
void output(int, int, const float*, const char*);

// report system time
//
void reportTime(const char* msg, steady_clock::duration span) {
    auto ms = duration_cast<milliseconds>(span);
    std::cout << msg << " - took - " <<
        ms.count() << " milliseconds" << std::endl;
}

const int MIN_WORKERS = 1;
const int MAX_WORKERS = 7;
// message tags
const int MASTER = 0;
const int NONE = 0;
const int BEGIN = 1;
const int ABOVE = 2;
const int BELOW = 3;
const int END = 4;
// weights
const float wx = 0.1f;
const float wy = 0.1f;

int main(int argc, char** argv) {


    if (argc != 4) {
        std::cerr << "*** Incorrect number of arguments ***\n";
        std::cerr << "Usage: " << argv[0] <<
            " no-of_rows no_of_columns no_of_iterations\n";
        return 1;
    }

    std::cerr << "Usage: " << "****" << argv[0] << "****" << argv[1] << "****" << argv[2] << "****" << argv[3];

    // grid properties
    int nRowsTotal = std::atoi(argv[1]);
    int nColumns = std::atoi(argv[2]);
    int nIterations = std::atoi(argv[3]);

    // allocate memory for data
    steady_clock::time_point ts, te;
    float* data;
    try {
        // 2 * for double buffering
        data = new float[2 * nRowsTotal * nColumns];
    }
    catch (std::bad_alloc) {
        std::cerr << "*** Failed to Allocate Memory for 2 * "
            << nRowsTotal << " by " << nColumns << " grid" << std::endl;
        return 3;
    }
    // initialize first buffer [0: nRowsTotal*nColumns)
    initialize(nRowsTotal, nColumns, data);
    // initialize second buffer [nRowsTotal*nColumns : nRowsTotal*nColumns)
    for (int i = 0; i < nRowsTotal * nColumns; i++)
        data[nRowsTotal * nColumns + i] = 0.0f;


    // MPI startup
    int nTasks;
    int iTask;
    // TO DO: 
           // SPMD Start
    int result = MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nTasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &iTask);
    std::cerr << nTasks << "\n";
    std::cerr << iTask << "\n";


    // validate data
    int nWorkers = nTasks - 1;
    if (nWorkers < MIN_WORKERS || nWorkers > MAX_WORKERS) {
        std::cerr << nWorkers << "\n";
        std::cerr << "*** Number of workers specified is outside bounds ***\n";
        std::cerr << "Bounds: " << MIN_WORKERS << " <= no_of_WORKERS <="
            << MAX_WORKERS << std::endl;

        return 2;
    }
    // determine row partitioning
    int nRowsPerWorker = (nRowsTotal + nWorkers - 1) / nWorkers;
    int nRowsLeftOver = (nWorkers) ? nRowsTotal - (nWorkers - 1) *
        nRowsPerWorker : nRowsTotal;


    // TO DO: identify the master process
    if (iTask == MASTER) {

        std::cout << "Number of Workers " << nWorkers << std::endl;
        std::cout << "Number of Rows    " << nRowsTotal << std::endl;
        std::cout << "Number of Columns " << nColumns << std::endl;
        std::cout << "Number of Rows Per Worker " << nRowsPerWorker << std::endl;
        std::cout << "Number of Rows Last Worker " << nRowsLeftOver << std::endl;
        // write original data to file
        output(nRowsTotal, nColumns, data, "original.dat");

        // decompose - partition original data amongst the workers
        ts = steady_clock::now();
        int rowOffset = 0;
        for (int iWorker = 0; iWorker < nWorkers; iWorker++) {
            // identify the worker's neighbours
            int above = (iWorker == 0) ? NONE : iWorker;
            int below = (iWorker == nWorkers - 1) ? NONE : iWorker + 2;
            // number of rows in this worker
            int nRowsThisWorker = (nRowsPerWorker) ? nRowsPerWorker + 1
                : nRowsLeftOver;
            if (iWorker == nWorkers - 1) {
                nRowsThisWorker = (nRowsLeftOver == 0) ? nRowsPerWorker
                    : nRowsLeftOver;
            }
            else {
                nRowsThisWorker = nRowsPerWorker;
            }
            // Send startup data partition to the worker
            // - row offset
            // - number of rows for this worker
            // - process id for worker above
            // - process id for worker below
            // - data for this worker
            int destination = iWorker + 1;

            // TO DO: MPI_Send
            // send to destination all the information for this partition (worker)
            // that destination partition will need to collect that data.
            // Do so in separate messages
            //  - row offset,
            //  - number of rows for this worker,
            //  - id for worker above,
            //  - id for worker below
            // now send all the data for the partition (worker) to destination
          /*  const int MASTER = 0;
            const int NONE = 0;
            const int BEGIN = 1;
            const int ABOVE = 2;
            const int BELOW = 3;
            const int END = 4;*/
            MPI_Send(&rowOffset, 1, MPI_INT, destination, 0, MPI_COMM_WORLD);
            MPI_Send(&nRowsThisWorker, 1, MPI_INT, destination, 1, MPI_COMM_WORLD);
            MPI_Send(&above, 1, MPI_INT, destination, 2, MPI_COMM_WORLD);
            MPI_Send(&below, 1, MPI_INT, destination, 3, MPI_COMM_WORLD);
            MPI_Send(data + rowOffset * nColumns,2* nRowsThisWorker * nColumns, MPI_FLOAT, destination, 4, MPI_COMM_WORLD);

            std::cout << "Sent to process " << destination << " "
                << nRowsThisWorker << " rows data["
                << std::setw(3) << rowOffset * nColumns << "..." << std::setw(3)
                << rowOffset * nColumns + nRowsThisWorker * nColumns - 1
                << "]" << std::endl;
            rowOffset += nRowsThisWorker;
        }

        // receive the final results of the calculations from each worker
        for (int iWorker = 0; iWorker < nWorkers; iWorker++) {
            int source = iWorker + 1;
            int msgTag = END;

            int nRowsThisWorker;
            // receive from iWorker
            // - row offset to start of its data
            // - number of rows processed by iWorker
            // - data from iWorker

            // TO DO: MPI_Recv
            // receive from each source all the information that master needs to
            // collect all the data from that source
            //  - row offset
            //  - number of rows for this worker
            // collect the data using these values
            MPI_Recv(&rowOffset, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&nRowsThisWorker, 1, MPI_INT, source, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(data + rowOffset * nColumns,2 * nRowsThisWorker * nColumns, MPI_FLOAT, source, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        te = steady_clock::now();
        reportTime("\nw8 Domain Decomposition ", te - ts);

        // write results to file
        std::cout << "Writing results.dat file...\n";
        output(nRowsTotal, nColumns, data, "results.dat");
    }
    else {
        // each worker operates only on its share of the data
        int source = 0;
        int nRows;
        int above;
        int below;
        int rowOffset;

        // this worker receives information for its allocated partition
        int msgTag = BEGIN;

        // TO DO: MPI_Recv
        // receive from source all the information needed to collect the data
        // for its entire partition:
        //  - row offset,
        //  - number of rows,
        //  - id of the neighbour above,
        //  - id of the neighbour below,
        // collect the data from source using row offset, number of columns
        // and number of rows
        MPI_Recv(&rowOffset, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&nRows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&above, 1, MPI_INT, source, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&below, 1, MPI_INT, source, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        // TO DO: Allocate memory for the received data
        MPI_Recv(data + rowOffset * nColumns, 2* nRows * nColumns, MPI_FLOAT, source, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        std::cout << "Process " << iTask << " received " << nRows << " rows data["
            << std::setw(3) << rowOffset * nColumns << "..." << std::setw(3)
            << rowOffset * nColumns + nRows * nColumns - 1 << "]" << std::endl;
        // set row indices for updating - do not update the first or the last row
        int startRow = rowOffset;
        int endRow = rowOffset + nRows - 1;
        if (rowOffset == 0) startRow = 1;
        if (rowOffset + nRows == nRowsTotal) endRow--;
        // this worker iterates on its share of the data
        int iBuffer = 0;
        for (int iter = 0; iter < nIterations; iter++) {
            // sends its boundary data to neighbouring workers - above and below
            if (above != NONE) {
                // send to the neighbour above

                // TO DO: MPI_Send
                // send the data for the single row at the top of this partition
                // to above
                MPI_Send(data + rowOffset * nColumns, 2 * nColumns, MPI_FLOAT, above, 0, MPI_COMM_WORLD);
                std::cout << std::setw(3) << iter + 1 << "[" << above
                    << ":" << iTask << ":" << below << "]";
                std::cout << " sends    data[" << std::setw(3)
                    << rowOffset * nColumns
                    << "..." << std::setw(3) << rowOffset * nColumns + nColumns - 1
                    << "] to   worker " << above << " above " << std::endl;
                source = above;
                msgTag = ABOVE;
                // receive from the neighbour above

                // TO DO: MPI_Recv
                // receive data for the single row at the top of this partition
                // from above
                MPI_Recv(data + (rowOffset - 1) * nColumns, 2* nColumns, MPI_FLOAT, above, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                std::cout << std::setw(3) << iter + 1 << "[" << above << ":"
                    << iTask << ":" << below << "]";
                std::cout << " receives data[" << std::setw(3)
                    << (rowOffset - 1) * nColumns << "..." << std::setw(3)
                    << (rowOffset - 1) * nColumns + nColumns - 1 << "] from worker "
                    << above << " above " << std::endl;
            }
            if (below != NONE) {
                // send to the neighbour below

                // TO DO: MPI_Send
                // send the data for the single row at the bottom of this
                // partition to above
                MPI_Send(&data[(rowOffset + nRows - 1) * nColumns],2* nColumns, MPI_FLOAT, below, 0, MPI_COMM_WORLD);
                std::cout << std::setw(3) << iter + 1 << "[" << above << ":"
                    << iTask << ":" << below << "]";
                std::cout << " sends    data[" << std::setw(3)
                    << (rowOffset + nRows - 1) * nColumns << "..." << std::setw(3)
                    << (rowOffset + nRows - 1) * nColumns + nColumns - 1
                    << "] to   worker " << below << " below " << std::endl;
                source = below;
                msgTag = BELOW;
                // receive from the neighbour below

                // TO DO: MPI_Recv
                // receive data for the single row at the bottom of this
                // partititon from below
                MPI_Recv(&data[(rowOffset + nRows) * nColumns],2* nColumns, MPI_FLOAT, below, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                std::cout << std::setw(3) << iter + 1 << "[" << above << ":"
                    << iTask << ":" << below << "]";
                std::cout << " receives data[" << std::setw(3)
                    << (rowOffset + nRows) * nColumns << "..." << std::setw(3)
                    << (rowOffset + nRows) * nColumns + nColumns - 1
                    << "] from worker " << below << " below " << std::endl;
            }
            std::cout << "Updating rows " << startRow << " through "
                << endRow << std::endl;
            // update its share of the data
            update(startRow, endRow, nColumns, wx, wy,
                data + iBuffer * nRowsTotal * nColumns,
                data + (1 - iBuffer) * nRowsTotal * nColumns);
            // switch buffers
            iBuffer = 1 - iBuffer;
        }
        // return the final results of the iterations by this worker to the master
        msgTag = END;
        // send results for this worker
        // - row offset for this worker's data
        // - number of rows processed by this worker
        // - data processed by this worker

        // TO DO:
        // send information to collect data
        //  - row offset,
        //  - number of rows
        // now send the data to MASTER
        MPI_Send(&rowOffset, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
        MPI_Send(&nRows, 1, MPI_INT, MASTER, 1, MPI_COMM_WORLD);
        MPI_Send(&data[rowOffset * nColumns], 2 * nRows * nColumns, MPI_FLOAT, MASTER, 4, MPI_COMM_WORLD);

        std::cout << "Worker " << iTask << " sends data[" << std::setw(3)
            << rowOffset * nColumns << "..." << std::setw(3)
            << (rowOffset + nRows) * nColumns - 1 << "] to master " << std::endl;

    }
    // SPMD ends

    // close down MPI// TO DO: 
    MPI_Finalize();

    // checksum
    if (iTask == 0) {
        double sum1 = 0.0, sum2 = 0.0;

        for (int i = 0; i < nRowsTotal * nColumns; ++i) {
            sum1 += data[i];
            sum2 += data[i + nRowsTotal * nColumns];
        }
        std::cout << "Checksums = " << sum1 << ',' <<sum2 << std::endl;
        std::cout << "Checksums = " << nRowsTotal << ',' << nColumns << std::endl;
    }

    // deallocate data
    delete[] data;
}

void initialize(int nRowsTotal, int nColumns, float* x) {
    for (int i = 0; i < nRowsTotal; i++)
        for (int j = 0; j < nColumns; j++)
            x[i * nColumns + j] = (float)(i * (nRowsTotal - i - 1) * j
                * (nColumns - j - 1));
}

void update(int startRow, int endRow, int nColumns, const float wx,
    const float wy, const float* x_old, float* x_new) {
    for (int i = startRow; i <= endRow; i++)
        for (int j = 1; j < nColumns - 1; j++)
            x_new[i * nColumns + j] = x_old[i * nColumns + j]
            + wx * (x_old[(i + 1) * nColumns + j] + x_old[(i - 1) * nColumns + j]
                - 2.0f * x_old[i * nColumns + j])
            + wy * (x_old[i * nColumns + j + 1] + x_old[i * nColumns + j - 1]
                - 2.0f * x_old[i * nColumns + j]);
}

void output(int nRowsTotal, int nColumns, const float* x, const char* filename) {
    std::ofstream file(filename);
    file << std::fixed << std::setprecision(1);
    for (int j = nColumns - 1; j >= 0; j--) {
        for (int i = 0; i < nRowsTotal; i++)
            file << std::setw(8) << x[i * nColumns + j];
        file << std::endl;
    }

}
#endif // MPI_SOLUTION
