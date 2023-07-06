////////////////////////////////////////////////////////////////////////
// OpenTibia - an opensource roleplaying game
////////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////

#ifndef __TASKS__
#define __TASKS__
#include <condition_variable>
#include "thread_holder_base.h"
#include "enums.h"
#include "stats.h"

const int DISPATCHER_TASK_EXPIRATION = 2000;
const auto SYSTEM_TIME_ZERO = std::chrono::system_clock::time_point(std::chrono::milliseconds(0));

class Task
{
public:
	// DO NOT allocate this class on the stack
	explicit Task(std::function<void(void)>&& f, const std::string& _description, const std::string& _extraDescription) :
			description(_description), extraDescription(_extraDescription), func(std::move(f)) {}
	Task(uint32_t ms, std::function<void(void)>&& f, const std::string& _description, const std::string& _extraDescription) :
			description(_description), extraDescription(_extraDescription), expiration(std::chrono::system_clock::now() + std::chrono::milliseconds(ms)), func(std::move(f)) {}

	virtual ~Task() = default;
	void operator()() {
		func();
	}

	void setDontExpire() {
		expiration = SYSTEM_TIME_ZERO;
	}

	bool hasExpired() const {
		if (expiration == SYSTEM_TIME_ZERO) {
			return false;
		}
		return expiration < std::chrono::system_clock::now();
	}

	const std::string description;
	const std::string extraDescription;
	uint64_t executionTime = 0;

protected:
	std::chrono::system_clock::time_point expiration = SYSTEM_TIME_ZERO;

private:
	// Expiration has another meaning for scheduler tasks,
	// then it is the time the task should be added to the
	// dispatcher
	std::function<void(void)> func;
};

Task* createNewTask(std::function<void(void)> f, const std::string& description, const std::string& extraDescription);
Task* createNewTask(uint32_t expiration, std::function<void(void)> f, const std::string& description, const std::string& extraDescription);

class Dispatcher : public ThreadHolder<Dispatcher> {
public:
	Dispatcher() : ThreadHolder() {
		static int id = 0;
		dispatcherId = id;
		id += 1;
	}

	void addTask(Task* task, bool push_front = false);

	void shutdown();

	uint64_t getDispatcherCycle() const {
		return dispatcherCycle;
	}

	void threadMain();

private:
	std::thread thread;
	std::mutex taskLock;
	std::condition_variable taskSignal;

	std::list<Task*> taskList;
	uint64_t dispatcherCycle = 0;
	int dispatcherId = 0;
};

extern Dispatcher g_dispatcher;

#endif
