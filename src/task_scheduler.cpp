#include "task_scheduler.hpp"

using namespace v8lm;

uint32_t TaskScheduler::AddTask(std::chrono::milliseconds delay, Action action, bool repeat) {
	std::scoped_lock lock(_mutex);

	uint32_t id = ++_nextId;
	_tasks.emplace(id, repeat, false, false, Clock::now() + delay, delay, std::move(action));
	return id;
}

void TaskScheduler::RemoveTask(uint32_t id) {
	std::scoped_lock lock(_mutex);

	auto it = std::find_if(_tasks.begin(), _tasks.end(), [id](const Task& task) {
		return task.id == id;
	});

	if (it != _tasks.end()) {
		if (it->exec) {
			it->kill = true;
		} else {
			_tasks.erase(it);
		}
	}
}

void TaskScheduler::RescheduleTask(uint32_t id, std::chrono::milliseconds newDelay) {
	std::scoped_lock lock(_mutex);

	auto it = std::find_if(_tasks.begin(), _tasks.end(), [id](const Task& task) {
		return task.id == id;
	});

	if (it != _tasks.end()) {
		if (!it->exec) {
			auto node = _tasks.extract(it);
			node.value().delay = newDelay;
			node.value().executeTime = Clock::now() + newDelay;
			_tasks.insert(std::move(node));
		}
	}
}

void TaskScheduler::Run() {
	std::scoped_lock lock(_mutex);

	auto now = Clock::now();

	while (!_tasks.empty()) {
		auto it = _tasks.begin();

		if (now >= it->executeTime) {
			it->exec = true;
			it->action();
			it->exec = false;

			if (it->repeat && !it->kill) {
				auto node = _tasks.extract(it);
				node.value().executeTime = Clock::now() + node.value().delay;
				_tasks.insert(std::move(node));
				continue;
			}

			_tasks.erase(it);
		} else {
			break;
		}
	}
}

void TaskScheduler::Reset() {
	std::scoped_lock lock(_mutex);

	_tasks.clear();
}