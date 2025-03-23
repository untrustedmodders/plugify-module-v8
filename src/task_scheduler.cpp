#include "task_scheduler.hpp"

using namespace v8lm;

uint32_t TaskScheduler::AddTask(std::chrono::milliseconds delay, Action action, bool repeat) {
	std::lock_guard<std::mutex> lock(_mutex);

	uint32_t id = ++_nextId;
	_task.emplace(id, repeat, Clock::now() + delay, delay, std::move(action));
	return id;
}

void TaskScheduler::RemoveTask(uint32_t id) {
	std::lock_guard<std::mutex> lock(_mutex);

	auto it = std::find_if(_task.begin(), _task.end(), [id](const Task& task) {
		return task.id == id;
	});

	if (it != _task.end()) {
		_task.erase(it);
	}
}

void TaskScheduler::RescheduleTask(uint32_t id, std::chrono::milliseconds newDelay) {
	std::lock_guard<std::mutex> lock(_mutex);

	auto it = std::find_if(_task.begin(), _task.end(), [id](const Task& task) {
		return task.id == id;
	});

	if (it != _task.end()) {
		auto node = _task.extract(it);
		node.value().executeTime = Clock::now() + newDelay;
		_task.insert(std::move(node));
	}
}

void TaskScheduler::Run() {
	std::lock_guard<std::mutex> lock(_mutex);

	while (!_task.empty()) {
		auto now = Clock::now();
		auto it = _task.begin();

		if (now >= it->executeTime) {
			it->action();

			if (it->repeat) {
				auto node = _task.extract(it);
				node.value().executeTime = Clock::now() + it->interval;
				_task.insert(std::move(node));
			} else {
				_task.erase(it); // Only erase non-repeating tasks
			}
		} else {
			break;
		}
	}
}


void TaskScheduler::Reset() {
	std::lock_guard<std::mutex> lock(_mutex);

	_task.clear();
}