// Fine Grained Locking using std::mutex.
#ifndef lacpp_sorted_list_hpp
#define lacpp_sorted_list_hpp lacpp_sorted_list_hpp
#include <mutex>

/* a sorted list implementation by David Klaftenegger, 2015
 * please report bugs or suggest improvements to david.klaftenegger@it.uu.se
 */


/* struct for list nodes */
template<typename T>
struct node {
    T value;
    node<T>* next;
    // Each node has its own std::mutex for fine-grained locking.
    std::mutex hold;
};

/* concurrent sorted singly-linked list with fine-grained std::mutex locking */
template<typename T>
class sorted_list {
private:
    // A dummy head node is used to simplify the logic,
    // eliminating the need for special-case handling of the `first` pointer.
    node<T>* head_node;

public:
    sorted_list() {
        head_node = new node<T>();
        head_node->next = nullptr;
    }
    
    // Defaulted constructors and operators for the class,
    // assuming they handle member `head_node` correctly if needed.
    sorted_list(const sorted_list<T>& other) = default;
    sorted_list(sorted_list<T>&& other) = default;
    sorted_list<T>& operator=(const sorted_list<T>& other) = default;
    sorted_list<T>& operator=(sorted_list<T>&& other) = default;

    // Explicitly handle destructor to avoid memory leaks.
    ~sorted_list() {
        node<T>* current = head_node->next;
        while(current != nullptr) {
            node<T>* next = current->next;
            delete current;
            current = next;
        }
        delete head_node;
    }

    /* insert v into the list */
    void insert(T v) {
        node<T>* pred = head_node;
        pred->hold.lock(); // Lock the predecessor (initially the dummy head)

        node<T>* curr = pred->next;
        if (curr) {
            curr->hold.lock(); // Lock the successor (the first real node)
        }
        
        // Hand-over-hand traversal: Lock the next node before unlocking the previous
        while (curr != nullptr && curr->value < v) {
            pred->hold.unlock();
            pred = curr;
            curr = curr->next;
            if (curr) {
                curr->hold.lock();
            }
        }
        
        node<T>* new_node = new node<T>();
        new_node->value = v;
        new_node->next = curr;
        
        // The list modification is protected by `pred`'s lock
        pred->next = new_node;
        
        // Unlock the two nodes in reverse order of acquisition to avoid deadlock
        if (curr) curr->hold.unlock();
        pred->hold.unlock();
    }

    /* remove one copy of the specified value */
    void remove(T v) {
        node<T>* pred = head_node;
        pred->hold.lock(); // Lock the predecessor (initially the dummy head)

        node<T>* curr = pred->next;
        if (curr) {
            curr->hold.lock(); // Lock the successor (the first real node)
        }
        
        while (curr != nullptr && curr->value < v) {
            pred->hold.unlock();
            pred = curr;
            curr = curr->next;
            if (curr) {
                curr->hold.lock();
            }
        }

        if (curr != nullptr && curr->value == v) {
            // Found the node to remove
            pred->next = curr->next;
            
            // Unlock `curr` and then `pred`
            curr->hold.unlock();
            pred->hold.unlock();
            
            delete curr;
        } else {
            // Value not found, unlock any held mutexes
            if (curr) curr->hold.unlock();
            pred->hold.unlock();
        }
    }

    /* count elements with value v in the list */
    std::size_t count(T v) {
        std::size_t cnt = 0;
        node<T>* pred = head_node;
        // Lock the predecessor (dummy head) to start traversal
        pred->hold.lock();
        
        node<T>* current = pred->next;
        if(current) current->hold.lock();

        while (current != nullptr && current->value < v) {
            pred->hold.unlock();
            pred = current;
            current = current->next;
            if (current) current->hold.lock();
        }

        while(current != nullptr && current->value == v) {
            cnt++;
            pred->hold.unlock();
            pred = current;
            current = current->next;
            if(current) current->hold.lock();
        }
        
        // Unlock remaining locks
        if (current) current->hold.unlock();
        pred->hold.unlock();

        return cnt;
    }
};

#endif // lacpp_sorted_list_hpp