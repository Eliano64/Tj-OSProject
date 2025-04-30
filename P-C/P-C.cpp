#include<thread>
#include <semaphore>
#include<iostream>
#include<vector>
#include <chrono>
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
    vector<int>output;
    int fill;
    int out;

    void put(){
        buffer[fill] = fill+1;
        //this_thread::sleep_for(chrono::milliseconds(50));
        //cout << buffer[fill] << ' '<<flush;
        output[out++] = buffer[fill];
        fill = (fill + 1) % 20;
    }


    void get() {
        int ptr = fill - 1 < 0 ? 19 : fill - 1;
        //this_thread::sleep_for(chrono::milliseconds(50));
        //cout << "\033[31m" << buffer[ptr] << "\033[0m"<<' '<<flush;
        output[out++] = -buffer[ptr];
        fill = ptr;
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

    Foo(int p, int c) :p(p), c(c),empty(20),full(0),lock(1),buffer(20,0),fill(0),output(p+c,0),out(0){
        if (p < c) {
            throw std::invalid_argument("Foo construction fail! p should not be smaller than c!");
        }
        producers = thread(&Foo::producer, this, p);
        consumers = thread(&Foo::consumer, this, c);
    };

    ~Foo() {
        producers.join();
        consumers.join();
        cout << '\n';
        for (auto& elem : output) {
            this_thread::sleep_for(chrono::milliseconds(100));
            if (elem > 0) {
                cout << "\033[31m" << "\ue29e"<<' '<<(elem<10?"0":"")<<elem << "\033[0m" << ' ' << flush;
            }
            else {
                cout << "\b\b\b\b\b";
                cout << "     ";
                cout << "\b\b\b\b\b";
                cout << "\033[B";
                cout << "\033[33m" << "\ue711" << ' ' << (-elem < 10 ? "0" : "") << -elem << "\033[0m" << ' ' << flush;
                cout << "\b\b\b\b\b";
                cout << "\033[A";
            }
        }
        cout << "\033[B";
        cout << "\033[B";
        cout << "\033[B";
    }
};

int main() {
    int p,c;
    cout<<"Please input the number of producers and consumers:";
    cin >> p >> c;
    try {
        Foo foo(p, c);
    }
    catch (const std::exception& e) {
        cerr << e.what() << endl;
    }
}