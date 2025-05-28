// async_scheduler.h

#pragma once

// Define ASYNC_SCHEDULER_LIB_EXPORTS when building this DLL
// Undefine it (or leave it undefined) when using this DLL
#ifdef SCHEDULER_LIB_EXPORTS
#define ASYNC_SCHEDULER_API __declspec(dllexport)
#else
#define ASYNC_SCHEDULER_API __declspec(dllimport)
#endif

#include <functional> // For std::function (used in public interface)
#include <chrono>     // For std::chrono::milliseconds (used in public interface)
#include <memory>     // For std::unique_ptr

// Forward declaration of the implementation class
// The actual definition will be in async_scheduler.cpp
class AsyncSchedulerImpl;

class ASYNC_SCHEDULER_API AsyncScheduler {
public:
    AsyncScheduler();
    // The destructor MUST be defined in the .cpp file where AsyncSchedulerImpl is fully defined.
    // This is crucial for std::unique_ptr with an incomplete type.
    ~AsyncScheduler();

    // Non-copyable and non-movable
    AsyncScheduler(const AsyncScheduler&) = delete;
    AsyncScheduler& operator=(const AsyncScheduler&) = delete;
    // Making it movable requires defining move constructor/assignment in .cpp
    // For simplicity, we'll keep it non-movable for now.
    // If move semantics are needed, they also need careful implementation with pImpl.
    AsyncScheduler(AsyncScheduler&&) = delete;
    AsyncScheduler& operator=(AsyncScheduler&&) = delete;

    /**
     * @brief Starts a task that repeats at the given interval.
     * If a task is already running, it will be stopped before the new one starts.
     * @param task The function to execute.
     * @param interval The time interval between task executions.
     * @return true if the task was successfully started, false otherwise.
     */
    bool startRepeatingTask(std::function<void()> task, std::chrono::milliseconds interval);

    /**
     * @brief Stops the currently active repeating task.
     * This function will block until the worker thread has finished.
     */
    void stop();

    /**
     * @brief Checks if a task is currently active or in the process of stopping.
     * @return true if a task is considered active, false otherwise.
     */
    bool isRunning() const;

private:
    // Pointer to the implementation (PIMPL)
    // This hides all the STL members that were causing C4251 warnings.
    std::unique_ptr<AsyncSchedulerImpl> pImpl;
};
