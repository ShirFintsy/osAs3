//
// Created by shir on 6/6/22.
//

#ifndef EX3_UNBOUNDEDQUEUE_H
#define EX3_UNBOUNDEDQUEUE_H

#include <string>
#include <list>
#include <queue>
#include "semaphore.h"

using namespace std;

class UnboundedQueue {
public:
    //list<string> UbQ;
    queue<string> UbQ;
//    int front = -1;
//    int rear = -1;
    semaphore* full;
    mutex m;

    UnboundedQueue() {
        full = new semaphore(0);
//        front = -1;
//        rear = -1;
    }

    void insert (const string& s) {
        m.lock();
//        if (front == - 1)
//            front = 0;
//        rear++;
        UbQ.push(s);
        m.unlock();
        full->release();
    }
    string remove () {
        full->acquire(); // down
        m.lock();
        string last = UbQ.back();
        UbQ.pop();
        m.unlock();
        return last;

//        if (front == -1) {
//            return nullptr; // the queue is empty
//        } else {
//            full->acquire(); //down
//            m.lock();
//            rear--; //???
//            string lastNews = UbQ.back();
//            const char* stringArray = lastNews.c_str();
//            m.unlock();
//            return const_cast<char *>(stringArray);
//        }
    }
};


#endif //EX3_UNBOUNDEDQUEUE_H
