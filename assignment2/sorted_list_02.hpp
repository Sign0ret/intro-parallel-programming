// Fine Grained Locking using std::mutex.
// This means we are implementing a main lock for each node?
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
    std::mutex hold;
};

/* non-concurrent sorted singly-linked list */
template<typename T>
class sorted_list {
	node<T>* first = nullptr;
    std::mutex hold;

	public:
		/* default implementations:
		 * default constructor
		 * copy constructor (note: shallow copy)
		 * move constructor
		 * copy assignment operator (note: shallow copy)
		 * move assignment operator
		 *
		 * The first is required due to the others,
		 * which are explicitly listed due to the rule of five.
		 */
		sorted_list() = default;
		sorted_list(const sorted_list<T>& other) = default;
		sorted_list(sorted_list<T>&& other) = default;
		sorted_list<T>& operator=(const sorted_list<T>& other) = default;
		sorted_list<T>& operator=(sorted_list<T>&& other) = default;
		~sorted_list() {
			while(first != nullptr) {
				remove(first->value);
			}
		}
		/* insert v into the list */
		void insert(T v) {
            while (true) {
                node<T>* pred = nullptr;
                node<T>* curr = first;

                // Lock the head's mutex if needed, here assuming first node mutex is used
                if (curr) curr->hold.lock();

                // Traverse list locking next node before unlocking previous to maintain order
                while (curr != nullptr && curr->value < v) {
                    if (pred) pred->hold.unlock();
                    pred = curr;
                    curr = curr->next;
                    if (curr) curr->hold.lock();
                }

                // At this point curr is either nullptr or node with value >= v
                // pred might be nullptr if inserting at head

                // Create new node and lock it
                node<T>* new_node = new node<T>();
                new_node->value = v;
                new_node->next = curr;
                // No need to lock new_node here as it's not accessible by other threads yet

                if (pred == nullptr) {
                    // Inserting at the head
                    // Lock main list mutex to safely update head pointer or rely on other synchronization
                    // Or we assume no separate list lock and modify first directly
                    first = new_node;
                } else {
                    pred->next = new_node;
                    pred->hold.unlock();
                }

                if (curr) curr->hold.unlock();

                break;
            }
        }

        void remove(T v) {
            while (true) {
                node<T>* pred = nullptr;
                node<T>* curr = first;

                // Lock the first node mutex (if exists)
                if (curr) curr->hold.lock();

                while (curr != nullptr && curr->value < v) {
                    if (pred) pred->hold.unlock();
                    pred = curr;
                    curr = curr->next;
                    if (curr) curr->hold.lock();
                }

                if (curr == nullptr || curr->value != v) {
                    // Value not found, unlock any held mutexes
                    if (pred) pred->hold.unlock();
                    if (curr) curr->hold.unlock();
                    return;
                }

                // Now curr holds the node with value == v

                if (pred == nullptr) {
                    // Removing head node
                    first = curr->next;
                    curr->hold.unlock();
                    delete curr;
                } else {
                    pred->next = curr->next;
                    curr->hold.unlock();
                    pred->hold.unlock();
                    delete curr;
                }

                break;
            }
        }


		/* count elements with value v in the list */
		std::size_t count(T v) {
            std::lock_guard<std::mutex> lock(hold);
			std::size_t cnt = 0;
			/* first go to value v */
			node<T>* current = first;
			while(current != nullptr && current->value < v) {
				current = current->next;
			}
			/* count elements */
			while(current != nullptr && current->value == v) {
				cnt++;
				current = current->next;
			}
			return cnt;
		}
};

#endif // lacpp_sorted_list_hpp
