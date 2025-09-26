#include <iostream>
#include <cstdlib>
#include <string>
#include <pthread.h>
#include <cmath>
#include <vector>
#include <chrono> 

using namespace std;


// sequential sieve of eratosthenes up to a given limit
vector<long long> sequential_sieve(long long limit) {
    if (limit < 2) {
        return {};
    }
    
    vector<bool> is_prime(limit + 1, true);
    is_prime[0] = is_prime[1] = false;

    for (long long p = 2; p * p <= limit; ++p) {
        if (is_prime[p]) {
            for (long long i = p * p; i <= limit; i += p) {
                is_prime[i] = false;
            }
        }
    }

    vector<long long> primes;
    for (long long p = 2; p <= limit; ++p) {
        if (is_prime[p]) {
            primes.push_back(p);
        }
    }
    return primes;
}

struct ThreadArgs {
    long long start;
    long long end;
    const vector<long long>* seed_primes; // list of seed primes
    vector<bool>* is_prime_global;       // shared answer array
};

// Parallelized function to find multiples
void* thread_sieve(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);
    
    for (long long p : *args->seed_primes) {
        // Find the first multiple of p
        long long start_multiple = (args->start + p - 1) / p;
        long long start_idx = start_multiple * p;

        if (start_idx < p * p) {
            start_idx = p * p;
        }
        
        // Mark the multiples 
        for (long long j = start_idx; j <= args->end; j += p) {
            (*args->is_prime_global)[j] = false;
        }
    }

    delete args; 
    return nullptr;
}


int main(int argc, char* argv[]) {
    // Argument validation
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <Max_value> <Num_threads>" << endl;
        return 1; 
    }

    long long max_value;
    int num_threads;

    try {
        max_value = stoll(argv[1]);
        num_threads = stoi(argv[2]);
    } catch (const invalid_argument& ia) {
        cerr << "Invalid argument: " << ia.what() << endl;
        return 1;
    } catch (const out_of_range& oor) {
        cerr << "Argument out of range: " << oor.what() << endl;
        return 1;
    }
    
    if (max_value < 2 || num_threads < 1) {
        cerr << "Max value must be at least 2 and threads must be at least 1." << endl;
        return 1;
    }

    cout << "Max value: " << max_value << endl;
    cout << "Number of threads: " << num_threads << endl;

    // start timer
    auto start_time = ::chrono::high_resolution_clock::now();

    //  Sequentialy compute all primes to sqrt max
    long long sequential_limit = static_cast<long long>(sqrt(max_value));
    cout << "Computing primes up to sqrt(Max) = " << sequential_limit << endl;
    vector<long long> seed_primes = sequential_sieve(sequential_limit);
    cout << "Found " << seed_primes.size() << " seed primes." << endl;

    // Shared array for all thrds
    vector<bool> is_prime_global(max_value + 1, true);
    is_prime_global[0] = is_prime_global[1] = false;

    for (long long p : seed_primes) {
        for (long long i = p * p; i <= sequential_limit; i += p) {
            is_prime_global[i] = false;
        }
    }

    // Create thread and divide work into chunks
    vector<pthread_t> threads(num_threads);
    long long start_range = sequential_limit + 1;
    long long end_range = max_value;
    long long chunk_size = (end_range - start_range + 1) / num_threads;
    
    cout << "Parallel sieving from " << start_range << " to " << end_range << endl;

    for (int i = 0; i < num_threads; ++i) {
        long long chunk_start = start_range + i * chunk_size;
        long long chunk_end = (i == num_threads - 1) ? end_range : chunk_start + chunk_size - 1;

        ThreadArgs* args = new ThreadArgs{
            chunk_start,
            chunk_end,
            &seed_primes,
            &is_prime_global
        };
        
        // Thread creation
        int result = pthread_create(&threads[i], nullptr, thread_sieve, static_cast<void*>(args));
        if (result != 0) {
            cerr << "Error creating thread " << i << endl;
            delete args; 
            return 1;
        }
    }

    //  Wait and process answers of threads
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    
    

    long long prime_count = 0;
    for (long long i = 2; i <= max_value; ++i) {
        if (is_prime_global[i]) {
            prime_count++;
        }
    }

    // Stop timer
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;

    cout << "Total primes found: " << prime_count << endl;
    cout << "Total execution time: " << elapsed.count() << " seconds" << endl;

    return 0;
}
