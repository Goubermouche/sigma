#pragma once
#include "thread_safe_queue.h"

namespace sigma::detail{
    using default_function_type = std::move_only_function<void()>;

    class thread_pool {
    public:
        explicit thread_pool(
            const unsigned int& number_of_threads = std::thread::hardware_concurrency()
        ) : tasks_(number_of_threads) {
            u64 current_id = 0;

            for (u64 i = 0; i < number_of_threads; ++i) {
                priority_queue_.push_back(static_cast<u64>(current_id));

                try {
                    threads_.emplace_back([&, id = current_id](const std::stop_token& stop_tok) {
                        do {
                            // wait until signaled
                            tasks_[id].signal.acquire();

                            do {
                                // invoke the task
                                while (auto task = tasks_[id].tasks.pop_front()) {
                                    try {
                                        pending_tasks_.fetch_sub(1, std::memory_order_release);
                                        std::invoke(std::move(task.value()));
                                    }
                                    catch (...) {}
                                }

                                // try to steal a task
                                for (u64 j = 1; j < tasks_.size(); ++j) {
                                    const u64 index = (id + j) % tasks_.size();
                                    if (auto task = tasks_[index].tasks.steal()) {
                                        // steal a task
                                        pending_tasks_.fetch_sub(1, std::memory_order_release);
                                        std::invoke(std::move(task.value()));
                                        // stop stealing once we have invoked a stolen task
                                        break;
                                    }
                                }

                            } while (pending_tasks_.load(std::memory_order_acquire) > 0);

                            priority_queue_.rotate_to_front(id);

                        } while (!stop_tok.stop_requested());
                    });
                    // increment the thread id
                    ++current_id;

                }
                catch (...) {
                    // remove one item from the tasks
                    tasks_.pop_back();

                    // remove our thread from the priority queue
                    std::ignore = priority_queue_.pop_back();
                }
            }
        }

        ~thread_pool() {
            // stop all threads
            for (u64 i = 0; i < threads_.size(); ++i) {
                threads_[i].request_stop();
                tasks_[i].signal.release();
                threads_[i].join();
            }
        }

        /// thread pool is non-copyable
        thread_pool(const thread_pool&) = delete;
        thread_pool& operator=(const thread_pool&) = delete;

        /**
         * @brief Enqueue a task into the thread pool that returns a result.
         * @details Note that task execution begins once the task is enqueued.
         * @tparam function An invokable type.
         * @tparam arguments Argument parameter pack
         * @tparam return_type The return type of the Function
         * @param f The callable function
         * @param args The parameters that will be passed (copied) to the function.
         * @return A std::future<ReturnType> that can be used to retrieve the returned value.
         */
        template <typename function, typename... arguments,
            typename return_type = std::invoke_result_t<function&&, arguments &&...>>
            requires std::invocable<function, arguments...>
        [[nodiscard]] std::future<return_type> enqueue(function f, arguments... args) {
            // we can do this in C++23 because we now have support for move only functions
            std::promise<return_type> promise;
            auto future = promise.get_future();
            auto task = [func = std::move(f), ... largs = std::move(args),
                promise = std::move(promise)]() mutable {
                try {
                    if constexpr (std::is_same_v<return_type, void>) {
                        func(largs...);
                        promise.set_value();
                    }
                    else {
                        promise.set_value(func(largs...));
                    }
                }
                catch (...) {
                    promise.set_exception(std::current_exception());
                }
            };

            enqueue_task(std::move(task));
            return future;
        }

        /**
         * @brief Enqueue a task to be executed in the thread pool that returns void.
         * @tparam function An invokable type.
         * @tparam arguments Argument parameter pack for Function
         * @param func The callable to be executed
         * @param args Arguments that will be passed to the function.
         */
        template <typename function, typename... arguments>
            requires std::invocable<function, arguments...>&&
        std::is_same_v<void, std::invoke_result_t<function&&, arguments &&...>>
            void enqueue_detach(function&& func, arguments &&...args) {
            enqueue_task(
                std::move([f = std::forward<function>(func),
                    ... largs = std::forward<arguments>(args)]() mutable -> decltype(auto) {
                        // suppress exceptions
                        try {
                            std::invoke(f, largs...);
                        }
                        catch (...) {
                        }
                    }));
        }

        [[nodiscard]] auto size() const { return threads_.size(); }

    private:
        template <typename function>
        void enqueue_task(function&& f) {
            auto i_opt = priority_queue_.copy_front_and_rotate_to_back();
            if (!i_opt.has_value()) {
                // would only be a problem if there are zero threads
                return;
            }
            auto i = *(i_opt);
            pending_tasks_.fetch_add(1, std::memory_order_relaxed);
            tasks_[i].tasks.push_back(std::forward<function>(f));
            tasks_[i].signal.release();
        }

        struct task_item {
            thread_safe_queue<default_function_type> tasks{};
            std::binary_semaphore signal{ 0 };
        };

        std::vector<std::jthread> threads_;
        std::deque<task_item> tasks_;
        thread_safe_queue<u64> priority_queue_;
        std::atomic_int_fast64_t pending_tasks_{};
    };
}