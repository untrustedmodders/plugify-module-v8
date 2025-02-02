#pragma once

namespace v8lm {

	using Task = std::function<void()>;

	struct DelayedTask {
		Task task;
		double when;

		bool operator<(const DelayedTask& t) const { return when > t.when; }
	};

	class TaskQueue {
	public:
		TaskQueue();
		~TaskQueue();

		TaskQueue(const TaskQueue&) = delete;
		TaskQueue& operator=(const TaskQueue&) = delete;
		TaskQueue(TaskQueue&&) = delete;
		TaskQueue&& operator=(TaskQueue&&) = delete;

		// If enabled then an empty event is execute after each call to Post(),
		// for example to wake up any calls blocked on glfwWaitEvents(), ect.
		void SetPostsEmptyEvents(Task post) { _post_empty_event = std::move(post); }

		void Post(Task task);
		void Post(double delay_in_seconds, Task task);

		// Returns -1 if there are no delayed tasks in the queue.
		// Returns 0 if there are tasks ready to be executed immediately.
		double GetSecondsToNextTask() const;

		// Runs all the tasks that can be executed now, and returns.
		void RunTasks();

		void ResetDropAllTasks();

	private:
		mutable std::mutex _lock;
		std::queue<Task> _tasks;
		std::priority_queue<DelayedTask> _delayedTasks;
		Task _post_empty_event;
	};

	class ThreadPoolTaskQueue {
	public:
		// Spawns "num_threads" that keep running and pumping tasks until the queue
		// is deleted.
		explicit ThreadPoolTaskQueue(int num_threads);

		// Joins on all threads before returning.
		~ThreadPoolTaskQueue();

		ThreadPoolTaskQueue(const ThreadPoolTaskQueue&) = delete;
		ThreadPoolTaskQueue& operator=(const ThreadPoolTaskQueue&) = delete;
		ThreadPoolTaskQueue(ThreadPoolTaskQueue&&) = delete;
		ThreadPoolTaskQueue&& operator=(ThreadPoolTaskQueue&&) = delete;

		void Post(Task task);
		void Post(double delay_in_seconds, Task task);

		void ResetDropAllTasks();

	private:
		void Run();

		std::mutex _lock;
		std::condition_variable _condVar;
		std::queue<Task> _tasks;
		std::priority_queue<DelayedTask> _delayedTasks;
		std::vector<std::thread> _threads;
		bool _quit;
	};

}