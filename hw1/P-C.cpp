#include<thread>
#include <semaphore>
#include<iostream>
#include<vector>
#include <chrono>
#include "cmd_console_tools.h"
#include <assert.h>
using namespace std;

class Foo
{
    int p, c;
    counting_semaphore<20> empty;
    counting_semaphore<20> full;
    counting_semaphore<1> lock;
    thread producers;
    thread consumers;
    vector<int>buffer;
    //vector<int>output;
    int fill;
    int ptr_get;
    int ptr_put;
    //int out;

    void printPut() {
        cct_gotoxy(11 + ptr_put*5, 2);
        ++ptr_put;
        cout << "\033[31m" << "\ue29e" << ' ' << (buffer[fill] < 10 ? "0" : "") << buffer[fill] << "\033[0m" << ' ' << flush;
    }

    void printGet() {
        cct_gotoxy(11 + ptr_get * 5, 3);
        ++ptr_get;
        cout << "\033[33m" << "\ue711" << ' ' << (buffer[fill] < 10 ? "0" : "") << buffer[fill] << "\033[0m" << ' ' << flush;
    }

    void put(){
        buffer[fill] = fill+1;
        this_thread::sleep_for(chrono::milliseconds(500));
        cct_gotoxy(11 + fill * 5, 1);
        cout << "\033[31m" << "\ue29e" << ' ' << (buffer[fill] < 10 ? "0" : "") << buffer[fill] << "\033[0m" << ' ' << flush;
        printPut();
        //output[out++] = buffer[fill];
        fill = (fill + 1) % 20;
    }


    void get() {
        fill = fill - 1 < 0 ? 19 : fill - 1;
        this_thread::sleep_for(chrono::milliseconds(500));
        cct_gotoxy(11 + fill * 5, 1);
        cout << "____ ";
        printGet();
        //output[out++] = -buffer[fill];
    }

public:
    void producer(int p)
    {
        for (int i = 0; i < p; ++i) {
            empty.acquire();
            lock.acquire();
            {
                put();
            }
            lock.release();
            full.release();
        }

    }

    void consumer(int c)
    {
        for (int i = 0; i < c; ++i) {
            full.acquire();
            lock.acquire();
            {
                get();
            }
            lock.release();
            empty.release();
        }
    }

    Foo(int p, int c) :p(p), c(c),empty(20),full(0),lock(1),buffer(20,0),fill(0),ptr_get(0),ptr_put(0){
       
        assert(p > c && p - c<=20);
        cout << "  buffer: ";
        for (int i = 0; i < 20; ++i) {
            cout << "____ " ;
        }
        cout<< " \n";
        cout << "producer: \n";
        cout << "consumer: \n";
        producers = thread(&Foo::producer, this, p);
        consumers = thread(&Foo::consumer, this, c);
    };

    ~Foo() {
        producers.join();
        consumers.join();
       
        cout << "\033[B";
        cout << "\033[B";
        cout << "\033[B";
    }
};

int main() {
    int p,c;
    cout<<"Please input the number of producers and consumers:";
    //cout << '\n';
    cin >> p >> c;
    
    Foo foo(p, c);
   
}