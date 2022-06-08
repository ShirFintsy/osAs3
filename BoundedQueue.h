//
// Created by shir on 6/1/22.
//

#include <string>
#include <list>
#include <queue>
#include "semaphore.h"

using namespace std;

#ifndef EX3_BOUNDEDQUEUE_H
#define EX3_BOUNDEDQUEUE_H


class BoundedQueue {
public:
    //int size;
    //list<string> BQ;
    //int front = -1;
    //int rear = -1;
    queue<string> q;
    semaphore* full;
    semaphore* empty;
    mutex m;

    BoundedQueue(int s) {
        //size = s;
        //printf("constructor of bounded queue\n");
        full = new semaphore(0);
        empty = new semaphore(s-1);
        //front = -1;
        //rear = -1;
    }

    void insert (const string& s) {
        empty->acquire(); //down
        m.lock();
        q.push(s);
        m.unlock();
        full->release();
//        if (rear == size - 1) {
//            printf("queue full\n");
//        }
//        else {
//            empty->acquire(); //down
//            m.lock();
//            if (front == - 1)
//                front = 0;
//            rear++;
//            BQ.insert(BQ.end(), s);
//            m.unlock();
//            full->release(); //up
//        }
    }
    string remove () {
        full->acquire(); // down
        m.lock();
        string last = q.front();
        q.pop();
        m.unlock();
        empty->release();
        return last;
//        if (front == -1) {
//            return nullptr; // the queue is empty
//        } else {
//            full->acquire(); // down
//            m.lock();
//            rear--;
//            string lastNews = BQ.back();
//            const char* stringArray = lastNews.c_str();
//            m.unlock();
//            empty->release();
//            return (char*) stringArray;
//        }
    }
};


#endif //EX3_BOUNDEDQUEUE_H
