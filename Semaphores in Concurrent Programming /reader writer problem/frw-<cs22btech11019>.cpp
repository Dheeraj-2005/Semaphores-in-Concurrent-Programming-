#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <ctime>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h> // Include the header for usleep
#include <semaphore.h>
#include <random>
#include <thread>

using namespace std;

// Define variables and semaphores
int nw, nr, kw, kr, readcount = 0, μCS, μRem; // Number of writer threads, reader threads, writer attempts, reader attempts
sem_t resource, rmutex, serviceQueue, logfile;
ofstream rw_log, avg_time;

// Function to get current system time
string getSysTime()
{
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
    return string(ctime(&now_c));
}

random_device rd;
mt19937 gen(rd());

long long RWT[1024]; // reader waiting time
long long WWT[1024]; // writer waiting time

// Writer function
void *writer(void *arg)
{
    int id = *((int *)arg);

    exponential_distribution<double> dist_μCS(1.0 / μCS);
    exponential_distribution<double> dist_μRem(1.0 / μRem);

    long long wtime = 0;

    for (int i = 0; i < kw; i++)
    {

        int randCSTime = dist_μCS(gen);
        int randRemTime = dist_μRem(gen);
        sem_wait(&logfile);
        auto reqTime = chrono::system_clock::now();
        rw_log << i + 1 << "th CS request by Writer Thread " << id << " at " << getSysTime();
        sem_post(&logfile);

        sem_wait(&serviceQueue);
        sem_wait(&resource);
        sem_post(&serviceQueue);

        sem_wait(&logfile);
        auto enterTime = chrono::system_clock::now();
        rw_log << i + 1 << "th CS Entry by Writer Thread " << id << " at " << getSysTime();
        sem_post(&logfile);

        wtime += chrono::duration_cast<chrono::milliseconds>(enterTime - reqTime).count();

        this_thread::sleep_for(chrono::milliseconds((int)randCSTime));

        sem_post(&resource);

        sem_wait(&logfile);
        auto exitTime = chrono::system_clock::now();
        rw_log << i + 1 << "th CS Exit by Writer Thread " << id << " at " << getSysTime();
        sem_post(&logfile);

        this_thread::sleep_for(chrono::milliseconds((int)randRemTime));
    }
    WWT[id] = wtime;
    return NULL;
}

// Reader function
void *reader(void *arg)
{
    int id = *((int *)arg);

    exponential_distribution<double> dist_μCS(1.0 / μCS);
    exponential_distribution<double> dist_μRem(1.0 / μRem);

    long long wtime = 0;

    for (int i = 0; i < kr; i++)
    {

        int randCSTime = dist_μCS(gen);
        int randRemTime = dist_μRem(gen);
        sem_wait(&logfile);
        auto reqTime = chrono::system_clock::now();
        rw_log << i + 1 << "th CS request by Reader Thread " << id << " at " << getSysTime();
        sem_post(&logfile);

        sem_wait(&serviceQueue);
        sem_wait(&rmutex);
        readcount++;
        if (readcount == 1)
        {
            sem_wait(&resource);
        }
        sem_post(&serviceQueue);
        sem_post(&rmutex);

        sem_wait(&logfile);
        auto enterTime = chrono::system_clock::now();
        rw_log << i + 1 << "th CS Entry by Reader Thread " << id << " at " << getSysTime();
        sem_post(&logfile);
        wtime += chrono::duration_cast<chrono::milliseconds>(enterTime - reqTime).count();
        this_thread::sleep_for(chrono::milliseconds((int)randCSTime));

        sem_wait(&rmutex);
        readcount--;
        if (readcount == 0)
        {
            sem_post(&resource);
        }
        sem_post(&rmutex);
        auto exitTime = chrono::system_clock::now();
        sem_wait(&logfile);
        rw_log << i + 1 << "th CS Exit by Reader Thread " << id << " at " << getSysTime();
        sem_post(&logfile);

        this_thread::sleep_for(chrono::milliseconds((int)randRemTime));
    }
    RWT[id] = wtime;
    return NULL;
}

int main()
{
    // Read input parameters from file
    ifstream inputFile("inp-params.txt");
    if (!inputFile.is_open())
    {
        cout << "Error opening input file." << endl;
        return 1;
    }
    inputFile >> nw >> nr >> kw >> kr >> μCS >> μRem;
    inputFile.close();

    // Initialize semaphores
    sem_init(&resource, 0, 1);
    sem_init(&rmutex, 0, 1);
    sem_init(&serviceQueue, 0, 1);
    sem_init(&logfile, 0, 1);
    // Create log file
    rw_log.open("FairRW-log.txt");
    if (!rw_log.is_open())
    {
        cout << "Error opening log file." << endl;
        return 1;
    }

    // Create writer threads
    pthread_t writerThreads[nw];
    for (int i = 0; i < nw; i++)
    {
        pthread_create(&writerThreads[i], NULL, writer, (void *)&i);
    }

    // Create reader threads
    pthread_t readerThreads[nr];
    for (int i = 0; i < nr; i++)
    {
        pthread_create(&readerThreads[i], NULL, reader, (void *)&i);
    }

    // Wait for writer threads to finish
    for (int i = 0; i < nw; i++)
    {
        pthread_join(writerThreads[i], NULL);
    }

    // Wait for reader threads to finish
    for (int i = 0; i < nr; i++)
    {
        pthread_join(readerThreads[i], NULL);
    }

    rw_log.close(); // Close log file

    avg_time.open("Average_time_FRW.txt");
    long long avgWaitingTime = 0;
    for (int i = 0; i < nr; i++)
    {
        avgWaitingTime = avgWaitingTime + RWT[i];
    }
    avgWaitingTime = avgWaitingTime / (nr * kr);

    avg_time << "The avg Waiting time of each reader thread is " << avgWaitingTime << " milliseconds" << endl;

    avgWaitingTime = 0;
    for (int i = 0; i < nw; i++)
    {
        avgWaitingTime = avgWaitingTime + WWT[i];
    }
    avgWaitingTime = avgWaitingTime / (nw * kw);

    avg_time << "The avg Waiting time of each writer thread is " << avgWaitingTime << " milliseconds" << endl;
    avg_time.close(); // Close the averagetime file

    // Destroy semaphores
    sem_destroy(&resource);
    sem_destroy(&rmutex);
    sem_destroy(&serviceQueue);

    return 0;
}
