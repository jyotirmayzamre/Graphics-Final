#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "helper.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

/*
ThreadPool class

- unique lock  : locks the mutex and unlocks when out of scope
- emplace_back : similar to push back but it constructs the element in the structure instead of copying 


Members
- threads     : vector containing instantiated threads
- tasks       : queue containing tasks
- queue_mutex : mutex for the queue for locking
- cv          : condition variable for managing threads
- stop        : variable used by the condition variable


Methods
- constructor : lock the queue, wait until there is a task in the queue, run the task
- destructor  : lock the queue, notify all threads and join
- enqueue     : lock the queue, pass ownership of the task to the vector and notify a waiting thread

*/

class ThreadPool {
    private:
        std::vector<std::thread> threads;
        std::queue<std::function<void()>> tasks;
        std::mutex queue_mutex;
        std::condition_variable cv;
        bool stop = false;

    public:
        ThreadPool(size_t num_threads) {
            for (size_t i = 0; i < num_threads; i++){
                threads.emplace_back([this] {
                    while (true){
                        std::function<void()> task;
                        {
                            //lock on the queue mutex to make the thread sleep till one task is in the queue
                            std::unique_lock<std::mutex> lock(queue_mutex);

                            //make the thread 
                            cv.wait(lock, [this] {
                                return !tasks.empty() || stop;
                            });

                            if (stop && tasks.empty()){
                                return;
                            }

                            task = move(tasks.front());
                            tasks.pop();
                        }
                        
                        task();
                    }
                    
                });
            }

        }

        void wait() {
            
        }
        ~ThreadPool() {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                stop = true;
            }
            
            cv.notify_all();

            for(auto& thread : threads){
                thread.join();
            }

        }

        //enqueue function to push task into queue
        void enqueue(std::function<void()> task){
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                tasks.emplace(move(task));
            }
            
            cv.notify_one();
        }
};



#endif