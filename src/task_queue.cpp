#include "task_queue.hpp"

using namespace v8lm;

static inline double Now() {
	using namespace std::chrono;
	using SecondsFP = std::chrono::duration<double>;
	return duration_cast<SecondsFP>(high_resolution_clock::now().time_since_epoch()).count();
}

TaskQueue::TaskQueue() = default;

TaskQueue::~TaskQueue() = default;

void TaskQueue::Post(Task task) {
	{
		std::lock_guard<std::mutex> lock(_lock);
		_tasks.emplace(std::move(task));
	}
	if (_post_empty_event) {
		_post_empty_event();
	}
}

void TaskQueue::Post(double delay_in_seconds, Task task) {
	{
		double when = Now() + delay_in_seconds;
		std::lock_guard<std::mutex> lock(_lock);
		_delayedTasks.emplace(DelayedTask{std::move(task), when});
	}
	if (_post_empty_event) {
		_post_empty_event();
	}
}

double TaskQueue::GetSecondsToNextTask() const {
	std::lock_guard<std::mutex> lock(_lock);
	if (!_tasks.empty()) {
		return 0;
	}
	if (_delayedTasks.empty()) {
		return -1;
	}
	double interval = _delayedTasks.top().when - Now();
	return interval <= 0 ? 0 : interval;
}

void TaskQueue::RunTasks() {
	for (;;) {
		Task task;

		{
			std::lock_guard<std::mutex> lock(_lock);

			// Process delayed tasks first, to prevent immediate tasks from delaying
			// these indefinitely.
			if (!_delayedTasks.empty() && Now() >= _delayedTasks.top().when) {
				const auto& top = _delayedTasks.top();
				task = std::move(const_cast<DelayedTask*>(&top)->task);
				_delayedTasks.pop();
			} else if (!_tasks.empty()) {
				task = std::move(_tasks.front());
				_tasks.pop();
			} else {
				return;
			}
		}

		task();
	}
}

void TaskQueue::ResetDropAllTasks() {
	std::queue<Task> tasks;
	std::priority_queue<DelayedTask> delayed_tasks;
	{
		std::lock_guard<std::mutex> lock(_lock);
		_tasks.swap(tasks);
		_delayedTasks.swap(delayed_tasks);
	}
	// Run destructors without the lock.
}

ThreadPoolTaskQueue::ThreadPoolTaskQueue(int num_threads) : _quit(false) {
	_threads.reserve(num_threads);
	for (int i = 0; i < num_threads; ++i) {
		_threads.emplace_back(&ThreadPoolTaskQueue::Run, this);
	}
}

ThreadPoolTaskQueue::~ThreadPoolTaskQueue() {
	{
		std::lock_guard<std::mutex> lock(_lock);
		_quit = true;
	}
	_condVar.notify_all();
	for (auto& thread : _threads) {
		thread.join();
	}
}

void ThreadPoolTaskQueue::Post(Task task) {
	{
		std::lock_guard<std::mutex> lock(_lock);
		_tasks.emplace(std::move(task));
	}
	_condVar.notify_one();
}

void ThreadPoolTaskQueue::Post(double delay_in_seconds, Task task) {
	double when = Now() + delay_in_seconds;
	{
		std::lock_guard<std::mutex> lock(_lock);
		_delayedTasks.emplace(DelayedTask{std::move(task), when});
	}
	_condVar.notify_one();
}

void ThreadPoolTaskQueue::Run() {
	for (;;) {
		Task task;

		{
			std::unique_lock<std::mutex> lock(_lock);

			for (;;) {
				double now = Now();
				if (_quit) {
					return;
				} else if (!_delayedTasks.empty() &&
						   now >= _delayedTasks.top().when) {
					const auto& top = _delayedTasks.top();
					task = std::move(const_cast<DelayedTask*>(&top)->task);
					_delayedTasks.pop();
					break;
				} else if (!_tasks.empty()) {
					task = std::move(_tasks.front());
					_tasks.pop();
					break;
				} else {
					if (_delayedTasks.empty()) {
						_condVar.wait(lock);
					} else {
						double timeout = _delayedTasks.top().when - now;
						_condVar.wait_for(lock, std::chrono::duration<double>(timeout));
					}
				}
			}
		}

		task();
	}
}

void ThreadPoolTaskQueue::ResetDropAllTasks() {
	std::queue<Task> tasks;
	std::priority_queue<DelayedTask> delayed_tasks;
	{
		std::lock_guard<std::mutex> lock(_lock);
		_tasks.swap(tasks);
		_delayedTasks.swap(delayed_tasks);
	}
	// Run destructors without the lock.
}