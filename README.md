# Approximate (Sloppy) Counter

The program simulates a sloppy counter in C++.
Call `make sloppy` which should create a `sloppySim` executable.
The program takes the following command line arguments in the following order:

```
sloppySim n_threads sloppiness work_time work_iterations cpu_bound do_logging
```

A brief explaination of the arguments is:

- `n_threads`: The number of threads to use. Defaults to 2 threads.

- `sloppiness`: The number of counts before updating the global counter. Defaults to 10.

- `work_time`: The average work time in milliseconds. Defaults to 10.

- `work_iterations`: The number of iterations per thread since the code will know when stop the simulation. Defaults to 100.

- `cpu_bound`: true or false

    - If true, each thread is “CPU bound”, meaning works for approximately work_time on a CPU.
    - If false, then each thread simulates I/O-bound work.
    - Defaults to false.

- `do_logging`: true or false 
    - If true, then writes out some logging information and the final global counter variable in `work_time*work_iterations / 10` millisecond intervals.
    - Defaults to false

At the end, the global count should be `n_threads*work_iterations`.

The `testing.txt` files includes some sample tests (and explainations) on a linux system.
