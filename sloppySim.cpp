#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <random>

const int NUM_ARGS = 7;
int global = 0;
pthread_mutex_t glock = PTHREAD_MUTEX_INITIALIZER;

struct counter {
    // std::vector<int> locals;
    int local;

    // The update frequency
    int sloppiness;

    // The work iterations
    int work_iters;

    // The work time
    int work_time;

    // CPU bound
    int cpu_bound;
};

struct count_obs {
    std::vector<counter>* c;
};

void print_params(std::vector<int> vals, int num_args) {
    // printf("%*d", width, value);

    printf("\nYou supplied %d argument(s).", num_args);
    printf("\nThe final arguments are as follows:");
    printf("\n\tn_threads        : %5d", vals[0]);
    printf("\n\tsloppiness       : %5d", vals[1]);
    printf("\n\twork_time        : %5d", vals[2]);
    printf("\n\twork_iterations  : %5d", vals[3]);

    vals[4] == 0 ? 
    printf("\n\tcpu_bound        : %5s", "false") : 
    printf("\n\tcpu_bound        : %5s", "true");

    vals[5] == 0 ? 
    printf("\n\tdo_logging       : %5s", "false") : 
    printf("\n\tdo_logging       : %5s", "true");
    printf("\n\n");
}

void* worker_thread(void* args) {
    counter* c = (counter*) args;

    // Get a random work time
    // https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(
        0.5*c->work_time, 1.5*c->work_time);
    c->work_time = distrib(gen);

    for (int i=0; i<c->work_iters; ++i) {
        
        // Sleep
        if (c->cpu_bound) {
            long increments = c->work_time * 1e6;
            for (long j=0; j<increments; ++j)
                ;
        }

        else
            usleep(c->work_time);

        c->local ++; // Update

        // Update
        if (c->local>= c->sloppiness) {
            pthread_mutex_lock(&glock); // Acquire lock
            global += c->local;
            pthread_mutex_unlock(&glock);

            // Reset
            c->local = 0;
        }
    }

    bool* ok = new bool;
    *ok = true;
    return ok;
}

int main(int argc, char *argv[]) {
    // Default values
    std::vector<int> vals = {2, 10, 10, 100, 0, 0};

    // If the num of parameters is ok
    if (argc > 1 && argc <= NUM_ARGS) {

        // For each paramter, put it's value to the vectors
        for (int i = 1; i < argc; i++) {
            
            // If the value is supposed to be an int
            if (i <= 4)
                // Convert string to int
                vals.at(i-1) = std::atoi(argv[i]);
            
            // If the value is supposed to be a boolean
            else {
                std::string val = argv[i];
                
                // Check for the boolean value (string)
                "true" == val ? vals.at(i-1) = 1 : vals.at(i-1) = 0;
            }
        }
    }

    else {
        printf("Number of arguments should be %d. %d provided.\n", (NUM_ARGS - 2), argc);
        printf("Using defaults\n");
    }

    // Assigning values...
    int n_threads = vals[0];
    int sloppiness = vals[1];
    int work_time = vals[2];
    int work_iterations = vals[3];
    int cpu_bound = vals[4];
    int do_logging = vals[5];

    print_params(vals, argc);

    // Initialize an array of counters
    std::vector<counter> counters (n_threads);
    for (int i=0; i<n_threads; ++i) {
        counter c;
        c.local = 0;
        c.sloppiness = sloppiness;
        c.work_iters = work_iterations;
        c.work_time = work_time;
        c.cpu_bound = cpu_bound;
        
        counters[i] = c;
    }

    // Initialize the count observer structure 
    // This will have a pointer to the array of structs
    count_obs count_observer;
    count_observer.c = &counters;

    // Initialize the worker threads
    pthread_t workers[n_threads];
    // Pass in each worker to the worker_thread function 
    // with an instance of a counter
    for (int i=0; i<n_threads; ++i)
        pthread_create(&workers[i],NULL, worker_thread, &counters[i]);

    if (do_logging) {
        int test_time = 10;
        int log_time = work_time * work_iterations / test_time;
        printf("Log time: %5d ms\n", log_time);

        for (int i=0; i<test_time; i++) {
            pthread_mutex_lock(&glock);
            printf("Global Counter Value: %5d", global);
            pthread_mutex_unlock(&glock);

            std::string values = "\tCounter Values: [";
            for (counter a_counter : counters)
                values += std::to_string(a_counter.local) + ", ";
            values.pop_back();  // Remove the last ','
            values.pop_back();  // Remove the last ' '
            values += "]\n";
            std::cout << values;

            // Sleep every `log_time` microseconds
            if (cpu_bound) {
                long increments = log_time * 1e6;
                for (long j=0; j<increments; ++j)
                    ;
            }
            else
                usleep(log_time);
        }
    }

    // Join the workers
    bool* ret;
    bool ok = true;
    for (int i=0; i<n_threads; ++i) {
        pthread_join(workers[i], (void**) &ret);
        ok &= *ret;
        delete ret;
    }

    printf("\nFinal Global Count = %5d\n", global);
}