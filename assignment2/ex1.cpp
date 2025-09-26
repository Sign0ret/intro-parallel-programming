#include <iostream>
#include <string>
#include <pthread.h>
#include <cmath>
#include <chrono>
#include <cstdlib>

using namespace std;

struct ThreadArgs {
    int start_index;
    int count;
    long double x_step;
    long double* total_area;
    pthread_mutex_t* mutex;
};
    
void usage(char *program) {
    cout << "Parallel Trapezoidal Integration using POSIX threads\n\n";
    cout << "Usage: " << program << " <number_of_threads> <number_of_trapezoids>\n";
    cout << "Example: " << program << " 4 1000\n\n";
    cout << "Options:\n";
    cout << "  -h, --help    Show this help message\n";
    exit(0);
}

long double f(long double x) {
    return 4.0L / (1.0L + x*x);
}

void* calculate_range(void* arguments) {
    ThreadArgs* args = (ThreadArgs*)arguments;
    long double local_sum = 0.0L;

    for (int i = 0; i < args->count; i++) {
        long double x1 = (args->start_index + i) * args->x_step;
        long double x2 = x1 + args->x_step;
        long double area = (args->x_step) * (f(x1) + f(x2)) / 2.0L;
        local_sum += area;
    }

    pthread_mutex_lock(args->mutex);
    *(args->total_area) += local_sum;
    pthread_mutex_unlock(args->mutex);

    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        string arg = argv[1];
        if (arg == "-h" || arg == "--help") {
            usage(argv[0]);
        }
    }

    if (argc != 3) {
        usage(argv[0]);
    }

    int threads, trapezoids;

    try {
        threads = stoi(argv[1]);
        trapezoids = stoi(argv[2]);
    } catch (...) {
        usage(argv[0]);
    }

    if (threads < 1 || trapezoids < 1) {
        usage(argv[0]);
    }

    long double x_step = 1.0L / trapezoids;
    int trapezoids_per_thread = trapezoids / threads;
    int remainder = trapezoids % threads;

    pthread_t* thread_ids = new pthread_t[threads];
    ThreadArgs* thread_args = new ThreadArgs[threads];
    pthread_mutex_t total_area_mutex;
    pthread_mutex_init(&total_area_mutex, nullptr);

    long double total_area = 0.0L;
    int current_start = 0;

    auto start_time = chrono::system_clock::now();

    for (int i = 0; i < threads; ++i) {
        int count = trapezoids_per_thread + (i < remainder ? 1 : 0);

        thread_args[i].start_index = current_start;
        thread_args[i].count = count;
        thread_args[i].x_step = x_step;
        thread_args[i].total_area = &total_area;
        thread_args[i].mutex = &total_area_mutex;

        pthread_create(&thread_ids[i], nullptr, calculate_range, (void*)&thread_args[i]);

        current_start += count;
    }

    for (int i = 0; i < threads; ++i) {
        pthread_join(thread_ids[i], nullptr);
    }

    auto duration = chrono::system_clock::now() - start_time;

    pthread_mutex_destroy(&total_area_mutex);

    cout.precision(15);
    cout << "Calculated value: " << total_area << " in " << chrono::duration<double>(duration).count() << " secs (wall clock)." << endl;

    delete[] thread_ids;
    delete[] thread_args;

    return 0;
}