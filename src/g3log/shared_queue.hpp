/** ==========================================================================
* 2010 by KjellKod.cc. This is PUBLIC DOMAIN to use at your own risk and comes
* with no warranties. This code is yours to share, use and modify with no
* strings attached and no restrictions or obligations.
 *
 * For more information see g3log/LICENSE or refer refer to http://unlicense.org
* ============================================================================
*
* Example of a normal std::queue protected by a mutex for operations,
* making it safe for thread communication, using std::mutex from C++0x with
* the help from the std::thread library from JustSoftwareSolutions
* ref: http://www.stdthread.co.uk/doc/headers/mutex.html
*
* This exampel  was totally inspired by Anthony Williams lock-based data structures in
* Ref: "C++ Concurrency In Action" http://www.manning.com/williams */

#pragma once

#include <atomic>
#include <cstddef>
#include <thread>

/*

template<typename T>
class shared_queue {
 public:
   typedef char cache_line[64];
   static const std::size_t  Size = 1048576; //2^20
   enum alignas(64) { Capacity = Size + 1 };

   shared_queue() : _tail(0), _head(0) {}
   virtual ~shared_queue() {}

   void push(T item); // pushByMOve?
   bool try_push(T& item); // pushByMOve?

   bool try_and_pop(T& item);
   void wait_and_pop(T& item);
   bool pop(T& item);

   bool empty() const;

   unsigned size() const { return 1; }

 private:

   size_t increment(size_t idx) const;

   cache_line _pad_tail;
   alignas(64) std::atomic <size_t>  _tail;  // tail(input) index
   // http://en.cppreference.com/w/cpp/types/aligned_storage
   //typename std::aligned_storage<sizeof(T), alignof(T)>::type data[Capacity];
   T _array[Capacity];
//      T    _array[Capacity];
   al


*/
/** Multiple producer, multiple consumer thread safe queue
* Since 'return by reference' is used this queue won't throw */
template<typename T>
class shared_queue {
   typedef char cache_line[64];
   static const std::size_t  Size = 1048576; //2^20
   enum alignas(64) { Capacity = Size + 1 };
   shared_queue& operator=(const shared_queue&) = delete;
   shared_queue(const shared_queue& other) = delete;

   cache_line _pad_tail;
   alignas(64) std::atomic <size_t>  _tail;

   cache_line _pad_storage;
   alignas(64) T _array[Capacity];

   cache_line _pad_head;
   alignas(64) std::atomic <size_t>  _head;

   bool wait_and_push(T item);
   size_t increment(size_t idx) const {
      return (idx + 1) % Capacity;
   }

 public:
   shared_queue() : _tail(0), _head(0) {}
   void push(T item);
   bool try_and_pop(T& popped_item);
   void wait_and_pop(T& popped_item);
   bool empty() const;
   //unsigned size() const {}
};


template<typename T>
void shared_queue<T>::push(T item) {
   while (false == wait_and_push(item)) {
      std::this_thread::yield();
   }
}

template<typename T>
bool shared_queue<T>::wait_and_push(T item) {
   auto current_tail = _tail.load(std::memory_order_relaxed);
   auto next_tail = increment(current_tail);
   if (next_tail != _head.load(std::memory_order_acquire)) {
      _array[current_tail] = item;
      _tail.store(next_tail, std::memory_order_release);
      return true;
   }
   return false; // full queue
}


template<typename T>
void shared_queue<T>::wait_and_pop(T& item) {
   while (false == try_and_pop(item)) {
      std::this_thread::yield();
   }
}


template<typename T>
bool shared_queue<T>::try_and_pop(T& item) {
   const auto current_head = _head.load(std::memory_order_relaxed);
   if (current_head == _tail.load(std::memory_order_acquire))
      return false; // empty queue

   item = _array[current_head];
   _head.store(increment(current_head), std::memory_order_release);
   return true;
}



template<typename T>
bool shared_queue<T>::empty() const {
   // snapshot with acceptance of that this comparison operation is not atomic
   return (_head.load() == _tail.load());
}






