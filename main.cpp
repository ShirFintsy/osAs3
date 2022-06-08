#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <thread>
#include <cstring>
#include <mutex>
#include <unistd.h>
#include "BoundedQueue.h"
#include "UnboundedQueue.h"

vector<BoundedQueue*> ProducersQueue;
UnboundedQueue* sportsQueue = new UnboundedQueue();
UnboundedQueue* newsQueue = new UnboundedQueue();
UnboundedQueue* weatherQueue = new UnboundedQueue();
BoundedQueue* CoEditQueue;

vector<string> split (string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

int getNumof(string type) {
    std::ifstream configFile ("config.txt");
    int counter = 0;
    //configFile.open(); //open a file to perform read operation using file object
    if (configFile.is_open()) {   //checking whether the file is open
        string line;
        while (getline(configFile, line)) { //read data from file object and put it into string.
            vector<string> v = split (line, " ");
            if (type == "producer" && v[0] == "PRODUCER") {
                counter++;
            }
            if (type == "co-edit" && v[0] == "Co-Editor") { //maybe doesn't need it
                counter = stoi(v[4]);
                break;
            }
        }
        configFile.close(); //close the file object.
    } return counter;
}

vector<int> getSizes(string type) {
    vector<int> sizes;
    std::ifstream configFile ("config.txt");

    if (configFile.is_open()) {   //checking whether the file is open
        string line;
        while (getline(configFile, line)) { //read data from file object and put it into string.
            vector<string> v = split (line, " ");
            if (type == "queue" && v[0] == "queue" && v[1] == "size") {
                sizes.push_back(stoi(v[3]));
            }else if (type == "news num" && v[0] == "PRODUCER") {
                string numOfNews;
                getline(configFile, numOfNews);
                sizes.push_back(stoi(numOfNews));
            }
        }
        configFile.close(); //close the file object.
    } return sizes;
}

void producer(int prod, int numOfNews) {
    //1. create all news
    vector<string> allNews;
    //printf("num od news for producer %d is %d\n", prod, numOfNews);
    for (int j = 0; j < numOfNews; ++j) {
        //string news = "Producer ";
        //news.append(to_string(prod));
        string news = "article number ";
        news.append(to_string(j + 1));
        news.append(" SPORTS");
        allNews.push_back(news);
        //allNews.insert(allNews.end(),news);
        //printf("producer %d create article: %s\n", prod, news.c_str());
    }
    allNews.push_back("end");
    printf("producer %d finished creating news\n",prod);

    //2. insert the news into queue
    int counter = 1;
    BoundedQueue* bq = ProducersQueue[prod - 1];
    //printf("queue: %s\n", bq->)
    for(const string& article: allNews) {
        //printf("article: %s\n", article.c_str());
        bq->insert(article);
        printf("insert article - %s - to queue number %d\n", article.c_str(), prod);
    }

    printf("producer %d finished his job\n", prod);
}

/*
 * return all news from producer queue by RR
 */
vector<string> getArticlesFromProducers(int numOfProducers) {
    vector<string> currentsNews;
    for (int i = 0; i < numOfProducers; ++i) {
//        auto begin = ProducersQueue.begin();
//        std::advance(begin, i);
//        BoundedQueue* current = *begin;
        BoundedQueue* current = ProducersQueue[i];
//        if (current->q.empty() == 1) { // if there aren't articles in current producer queue
//            printf("produce %d is empty\n", i);
//            continue;
//        }
        if(!current->q.empty()) { // not empty
            string article = current->remove();
//            if (article == "end")
//                printf("END article removed from prod %d\n", i + 1);
            printf("removed %s from produce %d\n", article.c_str(), i + 1);
            currentsNews.push_back(article);
        } else{
            printf("prod %d is empty now\n", i+1);
        }
    }
    return currentsNews;
}
void sortNews(vector<string> news) {
    unsigned long len = news.size(); // number of articles in the vector
    //printf("size of: %lu\n", len);
    for (int i = 0; i < len; ++i) {
        //const char* newsC = news[i].c_str();
//        if(news[i] == "end"){
//            printf("end in produce %d in sort function\n", i);
//            continue;
//        }
        if (news[i] != "end") {
            size_t found = news[i].find("SPORTS");
            if (found != string::npos) {
                //insert to sports queue
                sportsQueue->insert(news[i]);
                printf("insert to sports %s\n", news[i].c_str());
            }
            found = news[i].find("NEWS");
            if (found != string::npos) {
                //insert to news queue
                newsQueue->insert(news[i]);
            }
            found = news[i].find("WEATHER");
            if (found != string::npos) {
                //insert to weather queue
                weatherQueue->insert(news[i]);
            }
        }
    }
}
void dispatcher(int numOfProducers, const vector<int>& articlesForAll) {
    vector<string> newsRow;
    int counter = 1; //debug
    //int checkSum = 0;
    int ended = 0;
    while(1) { // while there are more articles
        newsRow = getArticlesFromProducers(numOfProducers);
        printf("size of vector: %zu\n", newsRow.size());
        //printf("news 1: %s, news 2: %s\n", newsRow[0].c_str(), newsRow[1].c_str());
        // check if all of them is "end":
        for (int i = 0; i < newsRow.size(); ++i) {
            if (newsRow[i] == "end") {
                printf("end article arrived from prod queue number %d\n", i);
                ended++;
            }
        }
        //checkSum += newsRow.size();

        //printf("about to enter sort for the %d time\n", counter);
        printf("about to enter sort with %zu articles\n", newsRow.size());
        sortNews(newsRow);
        if(ended == numOfProducers){
            printf("got all ends\n");
            break;
        }
        counter++; //debug
        //newsRow = getArticlesFromProducers(numOfProducers);
    }
    printf("finished\n");
}

void coEditor(const string& type) {
    //0. init the type queue
    UnboundedQueue* currentQueue;
    if(type == "sports")
        currentQueue = sportsQueue;
    else if(type == "news")
        currentQueue = newsQueue;
    else // weather
        currentQueue = weatherQueue;
    while(!currentQueue->UbQ.empty()) { // while the queue is not empty
        //1. remove article from type queue
        string article = currentQueue->remove();
        //2. block for 0.1 sec
        sleep(0.1);
        //3. insert it to coEdit queue
        CoEditQueue->insert(article);
        //4. print to screen
        if (article != "end")
            printf("FINAL: %s\n", article.c_str());
    }
}


int main() {
    int producers = getNumof("producer");
    printf("producers: %d\n", producers);
    thread allThreads[producers + 10];

    //0. init the co-editor queue:
    int coEditSize = getNumof("co-edit");
    CoEditQueue = new BoundedQueue(coEditSize);

    //1. initialize the producers queue:
    vector<int> sizesArray = getSizes("queue");
    //printf("size 1: %d, size 2: %d\n", sizesArray[0], sizesArray[1]);
    int counter = 1; //debug
    for (int size: sizesArray) {
        BoundedQueue* bq = new BoundedQueue(size);
        //printf("producer %d queue1 size is %d\n", counter, size);
        counter++;
        ProducersQueue.push_back(bq);
        //printf("created a queue in size %d\n", size);
        //ProducersQueue.insert(ProducersQueue.begin(), bq);
    }

    //2. read from file how much news each producer need to create
    vector<int> newsNumForAll = getSizes("news num");

    // for each producer:
    int i;
    for (i = 0; i < producers; ++i) {
        allThreads[i] = thread(producer, i + 1, newsNumForAll[i]);
    }
    sleep(5); // debug?
    printf("enter dis thread\n");
    allThreads[i+1] = thread(dispatcher, producers, newsNumForAll);
    printf("enter editors threads\n");
    allThreads[i+2] = thread(coEditor, "sports");
    allThreads[i+3] = thread(coEditor, "news");
    allThreads[i+4] = thread(coEditor, "weather");

    for(thread& t: allThreads){
        t.join();
    }
    return 0;
}
