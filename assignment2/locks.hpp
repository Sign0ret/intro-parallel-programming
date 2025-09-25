#ifndef LOCKS_HPP
#define LOCKS_HPP

#include <atomic>
#include <thread>
#include <mutex>

// A simple RAII wrapper for custom locks, similar to std::lock_guard.
template<typename T>
class lock_guard_custom {
private:
    T& m_lock;
public:
    explicit lock_guard_custom(T& lock) : m_lock(lock) {
        m_lock.lock();
    }
    ~lock_guard_custom() {
        m_lock.unlock();
    }
    lock_guard_custom(const lock_guard_custom&) = delete;
    lock_guard_custom& operator=(const lock_guard_custom&) = delete;
};

// Test-and-Test-and-Set (TATAS) Lock
// This lock reduces bus contention compared to a simple TAS lock
// by spinning on a read before attempting to acquire the lock.
class TATASLock {
private:
    std::atomic<bool> flag = ATOMIC_VAR_INIT(false);
public:
    void lock() {
        while (true) {
            // First test: Spin on a read. This reduces cache invalidations.
            while (flag.load(std::memory_order_relaxed)) {
                // Spin-wait
            }
            // Second test-and-set: Attempt to acquire the lock with an atomic exchange.
            if (!flag.exchange(true, std::memory_order_acquire)) {
                break; // Lock acquired
            }
        }
    }

    void unlock() {
        flag.store(false, std::memory_order_release);
    }
};

// CLH (Craig, Landin, and Hagersten) Queue Lock
// This version is designed to be a member of a class, for fine-grained locking.
// Each thread manages its own queue node to get in line for the lock.
class CLHNode {
public:
    std::atomic<bool> locked = ATOMIC_VAR_INIT(true);
};

class CLHLock {
private:
    std::atomic<CLHNode*> tail = ATOMIC_VAR_INIT(nullptr);

public:
    // Lock operation. Returns the predecessor node to spin on.
    CLHNode* lock() {
        CLHNode* my_node = new CLHNode();
        CLHNode* pred = tail.exchange(my_node, std::memory_order_acquire);
        if (pred != nullptr) {
            while (pred->locked.load(std::memory_order_relaxed)) {
                // Spin-wait
            }
        }
        return my_node;
    }

    // Unlock operation. Takes the current thread's node to unlock the lock.
    void unlock(CLHNode* my_node) {
        CLHNode* current_tail = my_node;
        if (!tail.compare_exchange_strong(current_tail, nullptr, std::memory_order_release)) {
            my_node->locked.store(false, std::memory_order_release);
        }
        delete my_node; // Clean up the node after unlocking
    }
};

#endif // LOCKS_HPP