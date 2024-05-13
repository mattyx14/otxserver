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
#include "otpch.h"

#include "dispatcher.h"
#include "game.h"
#include "outputmessage.h"

extern Game g_game;

Task* createNewTask(std::function<void(void)> f, const std::string& description, const std::string& extraDescription)
{
	return new Task(std::move(f), description, extraDescription);
}

Task* createNewTask(uint32_t expiration, std::function<void(void)> f, const std::string& description, const std::string& extraDescription)
{
	return new Task(expiration, std::move(f), description, extraDescription);
}

void Dispatcher::threadMain()
{
	// NOTE: second argument defer_lock is to prevent from immediate locking
	std::unique_lock<std::mutex> taskLockUnique(taskLock, std::defer_lock);
	std::chrono::high_resolution_clock::time_point time_point;

	while (getState() != THREAD_STATE_TERMINATED)
	{
		// check if there are tasks waiting
		taskLockUnique.lock();

		if(taskList.empty())
		{
			//if the list is empty wait for signal
			time_point = std::chrono::high_resolution_clock::now();
			taskSignal.wait(taskLockUnique);
			g_stats.dispatcherWaitTime(dispatcherId) += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - time_point).count();
		}

		if(!taskList.empty())
		{
			time_point = std::chrono::high_resolution_clock::now();

			// take the first task
			Task* task = taskList.front();
			taskList.pop_front();
			taskLockUnique.unlock();

			if(!task->hasExpired())
			{
				++dispatcherCycle;

				// execute it
				(*task)();

				g_game.clearSpectatorCache();
			}

			task->executionTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - time_point).count();
			g_stats.addDispatcherTask(dispatcherId, task);

		}
		else
			taskLockUnique.unlock();
	}
}

void Dispatcher::addTask(Task* task, bool push_front /*= false*/)
{
	bool do_signal = false;
	std::unique_lock<std::mutex> lock(taskLock); // lock mutex with unique_lock

	if(getState() == THREAD_STATE_RUNNING)
	{
		do_signal = taskList.empty();

		if (push_front)
			taskList.push_front(task);
		else
			taskList.push_back(task);
	}
	else
		delete task;

	lock.unlock();

	// Send a signal if the list is empty
	if(do_signal)
		taskSignal.notify_one();
}

void Dispatcher::shutdown()
{
	Task* task = createTask([this]() {
		setState(THREAD_STATE_TERMINATED);
		taskSignal.notify_one();
	});

	std::lock_guard<std::mutex> lockClass(taskLock);
	taskList.push_back(task);

	taskSignal.notify_one();
}
