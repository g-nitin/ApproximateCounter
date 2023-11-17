#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <random>

const int NUM_ARGS = 7;

struct counter_t {
    // Global counter and its lock
    int global;
    pthread_mutex_t glock;

    // The local counter
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

struct workers {
    // IDs for threads
    std::vector<counter_t> workers;
};

void print_params(std::vector<int> vals, int num_args, int work_time) {
    // printf("%*d", width, value);

    printf("\nYou supplied %d number of argument(s).", num_args);
    printf("\nThe final arguments are as follows:");
    printf("\n\tn_threads                 : %5d", vals[0]);
    printf("\n\tsloppiness                : %5d", vals[1]);
    printf("\n\twork_time                 : %5d", vals[2]);
    printf("\n\twork_time chosen (random) : %5d", work_time);
    printf("\n\twork_iterations           : %5d", vals[3]);

    vals[4] == 0 ? 
    printf("\n\tcpu_bound                 : %5s", "false") : 
    printf("\n\tcpu_bound                 : %5s", "true");

    vals[5] == 0 ? 
    printf("\n\tdo_logging                : %5s", "false") : 
    printf("\n\tdo_logging                : %5s", "true");
    printf("\n");
}

void* worker_thread(void* args) {
    counter_t* c = (counter_t*) args;

    for (int i=0; i<c->work_iters; ++i) {
        
        // Sleep
        if (c->cpu_bound) {
            long increments = c->work_time * 1e2;
            // printf("Increments: %ld", increments);
            for (long j=0; j<increments; ++j)
                ;
        }

        c->local ++; // Update

        // Update
        if (c->local >= c->sloppiness) {
            pthread_mutex_lock(&c->glock); // Acquire lock
            c->global += c->local;
            pthread_mutex_unlock(&c->glock);

            // Reset
            c->local = 0;
        }
    }

    printf("\nWorker ending.");

    bool* ok = new bool;
    *ok = true;
    return ok;
}

int main(int argc, char *argv[]) {

    if (argc > NUM_ARGS) {
        printf("Number of arguments should be %d. %d provided.\nExiting", (NUM_ARGS - 2), argc);
        exit(1);
    }

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
                if ("true" == val)
                    vals.at(i-1) = 1;
                else
                    vals.at(i-1) = 0;
            }
        }
    }

    // Assigning values...
    int n_threads = vals[0];
    int sloppiness = vals[1];
    
    // https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(0.5*vals[2], 1.5*vals[2]);    
    int work_time = distrib(gen);

    int work_iterations = vals[3];
    int cpu_bound = vals[4];
    int do_logging = vals[5];

    print_params(vals, argc, work_time);

    // Initialize
    counter_t c;
    pthread_mutex_init(&c.glock, NULL);
    c.global = 0;
    c.local = 0;
    c.sloppiness = sloppiness;
    c.work_iters = work_iterations;
    c.cpu_bound = cpu_bound;
    c.work_time = work_time;

    // Create workers and run them
    // std::vector<pthread_t> workers(n_threads);
    pthread_t workers[n_threads];
    for (int i=0; i<n_threads; ++i)
        pthread_create(&workers[i], NULL, worker_thread, &c);

    // Join the workers
    bool* ret;
    bool ok = true;

    for (int i=0; i<n_threads; ++i) {
        pthread_join(workers[i], (void**) &ret);

        ok &= *ret;
        delete ret;
    }

    printf("\nok: %d", ok);
    printf("\nFinal Count = %d\n", c.global);
}