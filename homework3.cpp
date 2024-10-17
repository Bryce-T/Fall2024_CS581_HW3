/*
Bryce Taylor
bktaylor2@crimson.ua.edu
CS 581
Homework #3

To compile (Intel):
    icpx -Wall -fopenmp -O homework3.cpp -o hw3.exe

To compile (g++):
    g++ -Wall -fopenmp -O homework3.cpp -o hw3.exe

To run:
    ./hw3.exe (Size of board) (Max generations) (Number of threads) (Output file directory)
    ./hw3.exe 1000 1000 8 outputs
*/

#include <chrono>
#include <fstream>
#include <iostream>
#include <omp.h>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
    int boardSize; // Board size (N)
    int maxGenerations; // Max # of iterations
    int numThreads; // Number of threads
    string outputDir; // Directory to write output file to

    vector<vector<bool>> curBoard; // State of the current board
    vector<vector<bool>> newBoard; // State of new board (for current iteration)
    vector<bool> row; // Helper variable

    //srand(time(0)); // Seed RNG
    srand(0); // Set seed

    // Input processing and error checking
    if (argc != 5) {
        cout << "Usage: ./life.exe (Size of board) (Max number of generations) (Number of threads) (Output file dir)" << endl;
        return 1;
    }
    boardSize = atoi(argv[1]);
    maxGenerations = atoi(argv[2]);
    numThreads = atoi(argv[3]);
    outputDir = argv[4];
    cout << "Board size: " << boardSize << ", Max generations: " << maxGenerations << endl;
    cout << "Number of threads: " << numThreads << ", Output directory: " << outputDir << endl;

    // Initialize empty board
    for (int i = 0; i < boardSize + 2; i++) {
        row.push_back(0);
    }
    for (int i = 0; i < boardSize + 2; i++) {
        curBoard.push_back(row);
    }
    row.clear();

    // Initialize board (minus borders) with pseudo-random values
    for (int i = 1; i < boardSize + 1; i++) {
        for (int j = 1; j < boardSize + 1; j++) {
            curBoard[i][j] = rand() % 2;
        }
    }

    // Start timer
    auto start = chrono::high_resolution_clock::now();

    // Main algorithm loop
    #pragma omp parallel num_threads(numThreads) shared(curBoard, newBoard)
    {
    int ID = omp_get_thread_num();
    int numNeighbors; // Track number of neighbors a cell has
    for (int n = 1; n <= maxGenerations; n++) {
        if (ID == 0) {
            // Calculate new board
            newBoard = curBoard;
        }
        // Ensure board is updated before threads work on this iteration
        #pragma omp barrier
        #pragma omp for
        for (int i = 1; i < boardSize + 1; i ++) {
            for (int j = 1; j < boardSize + 1; j++) {
                // Calculate number of alive neighbors this cell has
                numNeighbors = 0;
                numNeighbors += curBoard[i-1][j-1] + curBoard[i-1][j] + curBoard[i-1][j+1] + \
                                curBoard[i+1][j-1] + curBoard[i+1][j] + curBoard[i+1][j+1] + \
                                curBoard[i][j-1] + curBoard[i][j+1];

                // If cell is dead...
                if (curBoard[i][j] == 0) {
                    // 3 neighbors means it becomes alive
                    if (numNeighbors == 3) {
                        newBoard[i][j] = 1;
                    }
                    // Otherwise cell is still dead
                    else {
                        newBoard[i][j] = 0;
                    }
                }
                // If cell is alive...
                else if (curBoard[i][j] == 1) {
                    // 0 or 1 neighbors dies
                    if (numNeighbors < 2) {
                        newBoard[i][j] = 0;
                    }
                    // 4 or more neighbors dies
                    else if (numNeighbors > 3) {
                        newBoard[i][j] = 0;
                    }
                    // 2 or 3 neighbors survives
                    else {
                        newBoard[i][j] = 1;
                    }
                }
            }
        }
        // Ensure all threads have finished iteration, then update board and break if no change
        #pragma omp barrier
        if (ID == 0) {
            // Stop if there is no change between generations
            if (newBoard == curBoard) {
                break;
            }
            // Update the current board this generation
            curBoard = newBoard;
        }
    }
    }
    
    // Write output to a file
    string outputFileName = outputDir + "/output.txt";
    //string outputFileName = outputDir;
    ofstream OutputFile(outputFileName);
    for (int i = 0; i < boardSize + 2; i++) {
        for (int j = 0; j < boardSize + 2; j++) {
            OutputFile << newBoard[i][j] << " ";
        }
        OutputFile << endl;
    }
    OutputFile.close();

    // End timer and calculate time taken
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);

    cout << "Time taken: " << duration.count() << " ms" << endl;

    return 0;
}