#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <cmath>
#include <chrono>

using namespace std;

void usage(char *program)
{
    std::cout << "Parallel Trapezoidal Integration\n\n";
    std::cout << "Usage: " << program << " <number_of_threads> <number_of_trapezoids>\n";
    std::cout << "Example: " << program << " 4 1000\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help    Show this help message\n";
    exit(0);
}

long double f(long double x) {
    return 4.0L / (1.0L + x*x);
}

void calculate_range(
  int start_index,
  int count,
  long double x_step,
  long double& total_area,
  mutex& ans_mutex
) {
    long double local_sum = 0.0L;
    for (int i = 0; i < count; i++) {
        long double x1 = (start_index + i) * x_step;
        long double x2 = x1 + x_step;
        long double area = (x_step) * (f(x1) + f(x2)) / 2.0L;
        local_sum += area;
    }
    lock_guard<mutex> lock(ans_mutex);
    total_area += local_sum;
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
    }
    catch (...) {
        usage(argv[0]);
    }

    if (threads < 1 || trapezoids < 1) {
        usage(argv[0]);
    }

    // *** timing begins here ***
    auto start_time = std::chrono::system_clock::now();

    long double x_step = 1.0L / trapezoids;
    int trapezoids_per_thread = trapezoids / threads;
    int remainder = trapezoids % threads;

    vector<thread> thread_pool;
    long double total_area = 0.0L;
    mutex ans_mutex;

    int current_start = 0;

    for (int i = 0; i < threads; ++i) {
        int count = trapezoids_per_thread + (i < remainder ? 1 : 0);
        thread_pool.emplace_back(
          calculate_range,
          current_start,
          count,
          x_step,
          ref(total_area),
          ref(ans_mutex)
        );
        current_start += count;
    }

    for (auto& t : thread_pool) {
        t.join();
    }

    cout.precision(15);
    std::chrono::duration<double> duration =
    (std::chrono::system_clock::now() - start_time);
    // *** timing ends here ***
    cout << "Calculted value: " << total_area << "in " << duration.count() << " secs (wall clock)." << endl;
    return 0;
}