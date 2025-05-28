// async_scheduler.cpp
// Make sure to compile this file as part of the DLL project.

// This define should be present when building the DLL.
// For Meson, you're already doing this with cpp_args : ['/DSCHEDULER_LIB_EXPORTS']
// #define ASYNC_SCHEDULER_LIB_EXPORTS
#include "scheduler.h"

#include <thread>       // For std::thread (now in Impl)
#include <atomic>       // For std::atomic (now in Impl)
#include <mutex>        // For std::mutex (now in Impl)
#include <condition_variable> // For std::condition_variable (now in Impl)
#include <windows.h>    // For DllMain
#include <iostream>     // For potential error logging (optional)

// Definition of the implementation class
// This class now holds all the members that were previously in AsyncScheduler directly.
class AsyncSchedulerImpl {
public:
	AsyncSchedulerImpl() : stop_flag_(false), task_active_(false), current_interval_(0) {
		// Constructor for Impl
	}

	~AsyncSchedulerImpl() {
		// Ensure thread is stopped and joined in Impl's destructor
		// This will be called when AsyncScheduler's unique_ptr<AsyncSchedulerImpl> is destroyed.
		// No need to explicitly call stop() from AsyncScheduler's destructor if pImpl's destructor handles it.
		if (task_active_.load() || worker_.joinable()) {
			stop_flag_.store(true);
			cv_.notify_one();
			if (worker_.joinable()) {
				try {
					worker_.join();
				} catch (const std::system_error& /*e*/) {
					// Log error if necessary
					// std::cerr << "AsyncSchedulerImpl: Error joining worker thread in destructor: " << e.what() << std::endl;
				}
			}
		}
	}

	// Public methods of Impl, called by AsyncScheduler
	bool startRepeatingTask(std::function<void()> task, std::chrono::milliseconds interval) {
		// Stop any existing task first
		if (task_active_.load() || worker_.joinable()) {
			stop_flag_.store(true);
			cv_.notify_one();
			if (worker_.joinable()) {
				try {
					worker_.join();
				} catch (const std::system_error& /*e*/) {
					// std::cerr << "AsyncSchedulerImpl: Error joining previous worker thread: " << e.what() << std::endl;
					// Potentially problematic if join fails, but we must try to clean up.
				}
			}
		}
		// Reset state for the new task
		worker_ = std::thread(); // Reset thread object

		if (!task || interval.count() <= 0) {
			// std::cerr << "AsyncSchedulerImpl: Invalid task or interval." << std::endl;
			task_active_.store(false); // Ensure task_active is false if not starting
			return false;
		}

		current_task_ = std::move(task);
		current_interval_ = interval;
		stop_flag_.store(false); // Reset stop flag for the new task
		task_active_.store(true);  // Mark that a task is now configured

		try {
			worker_ = std::thread(&AsyncSchedulerImpl::workerThread, this);
		} catch (const std::system_error& /*e*/) {
			// std::cerr << "AsyncSchedulerImpl: Failed to start worker thread: " << e.what() << std::endl;
			task_active_.store(false); // Revert task_active_ state
			return false;
		}
		return true;
	}

	void stop() {
		if (!task_active_.load() && !worker_.joinable()) {
			return; // Not running or already stopped and joined.
		}

		stop_flag_.store(true);
		cv_.notify_one();

		if (worker_.joinable()) {
			try {
				worker_.join();
			} catch (const std::system_error& /*e*/) {
				// std::cerr << "AsyncSchedulerImpl: Error joining worker thread: " << e.what() << std::endl;
			}
		}
		task_active_.store(false);
	}

	bool isRunning() const {
		return task_active_.load();
	}

private:
	friend class AsyncScheduler; // Allow AsyncScheduler to access workerThread if needed, though it's private to Impl

	void workerThread() {
		if (!current_task_ || current_interval_.count() <= 0) {
			task_active_.store(false);
			return;
		}

		while (!stop_flag_.load()) {
			try {
				current_task_();
			} catch (const std::exception& /*e*/) {
				// std::cerr << "AsyncSchedulerImpl: Exception in scheduled task: " << e.what() << std::endl;
			} catch (...) {
				// std::cerr << "AsyncSchedulerImpl: Unknown exception in scheduled task." << std::endl;
			}

			std::unique_lock<std::mutex> lock(mtx_);
			if (cv_.wait_for(lock, current_interval_, [this] { return stop_flag_.load(); })) {
				break; // stop_flag_ was set
			}
		}
	}

	std::thread worker_;
	std::atomic<bool> stop_flag_{false};
	std::function<void()> current_task_;
	std::chrono::milliseconds current_interval_{0};
	std::mutex mtx_;
	std::condition_variable cv_;
	std::atomic<bool> task_active_{false};
};

// DllMain: Entry point for the DLL.
BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

// --- Implementation of AsyncScheduler methods ---

AsyncScheduler::AsyncScheduler() : pImpl(std::make_unique<AsyncSchedulerImpl>()) {
	// Constructor: Initializes the pImpl.
}

// Destructor definition is crucial here!
AsyncScheduler::~AsyncScheduler() {
	// The std::unique_ptr<AsyncSchedulerImpl> pImpl will automatically be destroyed,
	// calling AsyncSchedulerImpl's destructor, which handles thread cleanup.
	// No explicit call to pImpl->stop() is strictly necessary here if Impl's destructor is robust.
	// However, an explicit stop can make the intention clearer.
	// if (pImpl) {
	//     pImpl->stop();
	// }
}

bool AsyncScheduler::startRepeatingTask(std::function<void()> task, std::chrono::milliseconds interval) {
	if (!pImpl) return false; // Should not happen if constructor succeeded
	return pImpl->startRepeatingTask(std::move(task), interval);
}

void AsyncScheduler::stop() {
	if (!pImpl) return;
	pImpl->stop();
}

bool AsyncScheduler::isRunning() const {
	if (!pImpl) return false;
	return pImpl->isRunning();
}
