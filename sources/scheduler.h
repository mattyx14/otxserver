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

#ifndef __SCHEDULER__
#define __SCHEDULER__

#include "dispatcher.h"
#include <unordered_set>
#include <queue>

#include "thread_holder_base.h"

static constexpr int32_t SCHEDULER_MINTICKS = 50;

class SchedulerTask : public Task
{
public:
	void setEventId(uint32_t id) {
		eventId = id;
	}
	uint32_t getEventId() const {
		return eventId;
	}

	std::chrono::system_clock::time_point getCycle() const {
		return expiration;
	}

private:
	SchedulerTask(uint32_t delay, std::function<void(void)>&& f, const std::string& description, const std::string& extraDescription) :
			Task(delay, std::move(f), description, extraDescription) {}

	uint32_t eventId = 0;

	friend SchedulerTask* createNewSchedulerTask(uint32_t delay, std::function<void(void)>, const std::string& description, const std::string& extraDescription);
};

SchedulerTask* createNewSchedulerTask(uint32_t delay, std::function<void(void)> f, const std::string& description, const std::string& extraDescription);

struct TaskComparator {
	bool operator()(const SchedulerTask* lhs, const SchedulerTask* rhs) const {
		return lhs->getCycle() > rhs->getCycle();
	}
};

class Scheduler : public ThreadHolder<Scheduler>
{
public:
	uint32_t addEvent(SchedulerTask* task);
	bool stopEvent(uint32_t eventId);

	void shutdown();

	void threadMain();

private:
	std::thread thread;
	std::mutex eventLock;
	std::condition_variable eventSignal;

	uint32_t lastEventId{ 0 };
	std::priority_queue<SchedulerTask*, std::deque<SchedulerTask*>, TaskComparator> eventList;
	std::unordered_set<uint32_t> eventIds;
};

extern Scheduler g_scheduler;

#endif
