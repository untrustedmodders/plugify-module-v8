#include <utility>

#pragma once

namespace v8lm {
	using Action = std::function<void()>;
	using Clock = std::chrono::steady_clock;
	using TimePoint = std::chrono::time_point<Clock>;

	struct Task {
		uint32_t id;
		bool repeat;
		TimePoint executeTime;
		std::chrono::milliseconds interval;
		Action action;

		bool operator<(const Task& other) const {
			return executeTime < other.executeTime ||
				   (executeTime == other.executeTime && id < other.id);
		}
	};
	
	class TaskScheduler {
	private:
		std::set<Task> _task;
		std::mutex _mutex;
		bool _running = true;
		uint32_t _nextId = 0;
		
	public:
		uint32_t AddTask(std::chrono::milliseconds delay, Action action, bool repeat = false);
		void RemoveTask(uint32_t id);
		void RescheduleTask(uint32_t id, std::chrono::milliseconds newDelay);

		void Run();
		void Reset();
	};
}