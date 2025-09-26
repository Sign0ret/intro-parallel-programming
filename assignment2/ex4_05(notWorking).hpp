#ifndef lacpp_sorted_list_hpp
#define lacpp_sorted_list_hpp lacpp_sorted_list_hpp

#include "ex4_locks.hpp" 

template<typename T>
struct node {
    T value;
    node<T>* next;
    CLHLock lock;  
    
    node(T v = T{}) : value(v), next(nullptr), lock() {}
};

template<typename T>
class sorted_list {
private:
    node<T>* head_node;

public:
    sorted_list() {
        head_node = new node<T>();  
        head_node->next = nullptr;
    }

    ~sorted_list() {
        node<T>* current = head_node;
        while (current != nullptr) {
            node<T>* next = current->next;
            delete current;
            current = next;
        }
    }

    void insert(T v) {
        node<T>* pred = head_node;
        CLHNode* pred_lock = pred->lock.lock();

        node<T>* curr = pred->next;
        CLHNode* curr_lock = nullptr;
        if (curr != nullptr) {
            curr_lock = curr->lock.lock();
        }

        while (curr != nullptr && curr->value < v) {
            pred->lock.unlock(pred_lock);
            pred = curr;
            pred_lock = curr_lock;

            curr = curr->next;
            if (curr != nullptr) {
                curr_lock = curr->lock.lock();
            } else {
                curr_lock = nullptr;
            }
        }

        node<T>* new_node = new node<T>(v);
        new_node->next = curr;
        pred->next = new_node;

        if (curr_lock) curr->lock.unlock(curr_lock);
        pred->lock.unlock(pred_lock);
    }

    void remove(T v) {
        node<T>* pred = head_node;
        CLHNode* pred_lock = pred->lock.lock();

        node<T>* curr = pred->next;
        CLHNode* curr_lock = nullptr;
        if (curr != nullptr) {
            curr_lock = curr->lock.lock();
        }

        while (curr != nullptr && curr->value < v) {
            pred->lock.unlock(pred_lock);
            pred = curr;
            pred_lock = curr_lock;

            curr = curr->next;
            if (curr != nullptr) {
                curr_lock = curr->lock.lock();
            } else {
                curr_lock = nullptr;
            }
        }

        if (curr != nullptr && curr->value == v) {
            pred->next = curr->next;
            curr->lock.unlock(curr_lock);
            delete curr;
            pred->lock.unlock(pred_lock);
        } else {
            if (curr_lock) curr->lock.unlock(curr_lock);
            pred->lock.unlock(pred_lock);
        }
    }

    std::size_t count(T v) {
        std::size_t cnt = 0;

        node<T>* curr = head_node->next;
        CLHNode* curr_lock = nullptr;

        while (curr != nullptr && curr->value < v) {
            curr_lock = curr->lock.lock();
            node<T>* next = curr->next;
            curr->lock.unlock(curr_lock);
            curr = next;
        }

        while (curr != nullptr && curr->value == v) {
            curr_lock = curr->lock.lock();
            cnt++;
            node<T>* next = curr->next;
            curr->lock.unlock(curr_lock);
            curr = next;
        }
        return cnt;
    }
};

#endif // lacpp_sorted_list_hpp
