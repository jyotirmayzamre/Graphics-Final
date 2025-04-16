#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "helper.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool {

    public:
        ThreadPool(size_t num_threads){
            for (size_t i = 0; i < num_threads; i++){

                //each thread runs a lambda function
                //this keyword is used to access members of the class
                threads_.emplace_back([this] {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(queue_mutex_);
                            cv_.wait(lock, [this] {
                                return !tasks_.empty() || stop_;
                            });

                            if (stop_ && tasks_.empty()){
                                return;
                            }
                            task = move(tasks_.front());
                            tasks_.pop();
                        }
                        task();
                    }
                });
            }
        }

        ~ThreadPool(){
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                stop_ = true;
            }
            cv_.notify_all();

            for (auto& thread : threads_){
                thread.join();
            }
        }

        void enqueue(std::function<void()> task){
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                tasks_.emplace(move(task));
                cv_.notify_one();
            }
        }


    private:
        std::vector<std::thread> threads_;
        std::queue<std::function<void()>> tasks_;
        std::mutex queue_mutex_;
        std::condition_variable cv_;
        bool stop_ = false;
};



#endif