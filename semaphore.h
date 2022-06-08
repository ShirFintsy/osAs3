//
// Created by shir on 6/6/22.
//

#ifndef EX3_SEMAPHORE_H
#define EX3_SEMAPHORE_H


#include <mutex>
#include <condition_variable>

class semaphore {
    std::mutex mutex_;
    std::condition_variable condition_;
    unsigned long count_; // Initialized as locked.

public:
    semaphore(int size) {
        count_ = size;
    }

    void release() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        ++count_;
        condition_.notify_one();
    }

    void acquire() {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        while(!count_) // Handle spurious wake-ups.
            condition_.wait(lock);
        --count_;
    }

    bool try_acquire() {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        if(count_) {
            --count_;
            return true;
        }
        return false;
    }
};


#endif //EX3_SEMAPHORE_H
