#ifndef LOCKS_HPP
#define LOCKS_HPP

#include <atomic>
#include <thread>
#include <mutex>

// https://medium.com/developer-rants/c-threads-and-atomic-variables-oversimplified-b37bbbe3f2e6

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

class TATASLock {
private:
    std::atomic<bool> flag = ATOMIC_VAR_INIT(false);
public:
    void lock() {
        while (true) {
            while (flag.load(std::memory_order_relaxed)) {
                // Spin-wait
            }
            if (!flag.exchange(true, std::memory_order_acquire)) {
                break; // Lock acquired
            }
        }
    }

    void unlock() {
        flag.store(false, std::memory_order_release);
    }
};

// Not workinggg
class CLHNode {
public:
    std::atomic<bool> locked = ATOMIC_VAR_INIT(true);
};

class CLHLock {
private:
    std::atomic<CLHNode*> tail = ATOMIC_VAR_INIT(nullptr);

public:
    CLHNode* lock() {
        CLHNode* my_node = new CLHNode();
        CLHNode* pred = tail.exchange(my_node, std::memory_order_acquire);
        if (pred != nullptr) {
            while (pred->locked.load(std::memory_order_relaxed)) {
                // Spining
            }
        }
        return my_node;
    }

    void unlock(CLHNode* my_node) {
        CLHNode* current_tail = my_node;
        if (!tail.compare_exchange_strong(current_tail, nullptr, std::memory_order_release)) {
            my_node->locked.store(false, std::memory_order_release);
        }
        // cleanup
        delete my_node; 
    }
};

#endif // LOCKS_HPP
