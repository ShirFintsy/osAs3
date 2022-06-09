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

vector<string> split (string s, const string& delimiter) {
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

int getNumof(const string& type) {
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
        news.append(" SPORTS - belong to produce ");
        news.append(to_string(prod));
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
        BoundedQueue* current = ProducersQueue[i];

        if(!current->q.empty()) { // not empty
            string article = current->remove();

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
    for (int i = 0; i < len; ++i) {
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
    // sent "end" message to all types of queues:
    for (int i = 0; i < numOfProducers; ++i) {
        sportsQueue->insert("end");
        newsQueue->insert("end");
        weatherQueue->insert("end");
    }
    printf("dispatcher finished\n");
}

void coEditor(const string& type) {
    int endCounter = 0;
    //0. init the type queue
    UnboundedQueue* currentQueue;
    if(type == "sports")
        currentQueue = sportsQueue;
    else if(type == "news")
        currentQueue = newsQueue;
    else // weather
        currentQueue = weatherQueue;
    while(1) { // while the queue didn't finish
        //1. remove article from type queue
        string article = currentQueue->remove();
        //printf("in type %s article is %s\n",type.c_str(), article.c_str());
        //2. block for 0.1 sec
        sleep(0.1);
        //3. insert it to coEdit queue
        if (article != "end"){
            CoEditQueue->insert(article);
        }
        else {
            break;
//            endCounter++;
//            if(endCounter == numOfProducers)
//                break;
        }
    }
    CoEditQueue->insert("end");
    printf("co editor %s finished\n", type.c_str());
}

void screenManger() {
    int endCounter = 0;
    while(1) {
        //1. remove from co editor queue
        string article = CoEditQueue->remove();
        //printf("screen mangers just removed %s\n", article.c_str());
        //2. print it
        if (article != "end")
            printf("FINAL: %s\n", article.c_str());
        else{
            endCounter++;
            if(endCounter == 3)
                break;
        }
    }
    printf("screen manger finished\n");
}

int main() {
    int producers = getNumof("producer");
    printf("producers: %d\n", producers);

    //0. init the co-editor queue:
    int coEditSize = getNumof("co-edit");
    CoEditQueue = new BoundedQueue(coEditSize);

    //1. initialize the producers queue:
    vector<int> sizesArray = getSizes("queue");
    for (int size: sizesArray) {
        BoundedQueue* bq = new BoundedQueue(size);
        ProducersQueue.push_back(bq);
    }

    //2. read from file how much news each producer need to create
    vector<int> newsNumForAll = getSizes("news num");

    vector<thread> allThreads;
    // for each producer:
    int i;
    for (i = 0; i < producers; ++i) {
        allThreads.emplace_back(producer, i + 1, newsNumForAll[i]);
    }
    allThreads.emplace_back(dispatcher, producers, newsNumForAll);
    allThreads.emplace_back(coEditor, "sports");
    allThreads.emplace_back(coEditor, "news");
    allThreads.emplace_back(coEditor, "weather");
    allThreads.emplace_back(screenManger);

    for(thread& t: allThreads){
        t.join();
    }
    return 0;
}
