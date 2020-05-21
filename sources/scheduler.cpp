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
#include "scheduler.h"
#if defined __EXCEPTION_TRACER__
#include "exception.h"
#endif

Scheduler::SchedulerState Scheduler::m_threadState = Scheduler::STATE_TERMINATED;

Scheduler::Scheduler()
{
	m_lastEvent = 0;
	Scheduler::m_threadState = STATE_RUNNING;
	m_thread = boost::thread(boost::bind(&Scheduler::schedulerThread, this));
}

void Scheduler::schedulerThread()
{
	#if defined __EXCEPTION_TRACER__
	ExceptionHandler schedulerExceptionHandler;
	schedulerExceptionHandler.InstallHandler();
	#endif

	boost::unique_lock<boost::mutex> eventLockUnique(m_eventLock, boost::defer_lock);
	while(Scheduler::m_threadState != Scheduler::STATE_TERMINATED)
	{
		SchedulerTask* task = NULL;
		bool run = false, action = false;

		// check if there are events waiting...
		eventLockUnique.lock();
		if(m_eventList.empty()) // unlock mutex and wait for signal
			m_eventSignal.wait(eventLockUnique);
		else // unlock mutex and wait for signal or timeout
			action = m_eventSignal.timed_wait(eventLockUnique, m_eventList.top()->getCycle());

		// the mutex is locked again now...
		if(!action && Scheduler::m_threadState != Scheduler::STATE_TERMINATED)
		{
			// ok we had a timeout, so there has to be an event we have to execute...
			task = m_eventList.top();
			m_eventList.pop();

			// check if the event was stopped
			EventIds::iterator it = m_eventIds.find(task->getEventId());
			if(it != m_eventIds.end())
			{
				// was not stopped so we should run it
				run = true;
				m_eventIds.erase(it);
			}
		}

		eventLockUnique.unlock();
		// add task to dispatcher
		if(!task)
			continue;

		// if it was not stopped
		if(run)
		{
			task->unsetExpiration();
			Dispatcher::getInstance().addTask(task, true);
		}
		else
			delete task; // was stopped, have to be deleted here
	}

	#if defined __EXCEPTION_TRACER__
	schedulerExceptionHandler.RemoveHandler();
	#endif
}

uint32_t Scheduler::addEvent(SchedulerTask* task)
{
	bool signal = false;
	m_eventLock.lock();
	if(Scheduler::m_threadState == Scheduler::STATE_RUNNING)
	{
		// check if the event has a valid id
		if(!task->getEventId())
		{
			// if not generate one
			if(m_lastEvent >= 0xFFFFFFFF)
				m_lastEvent = 0;

			++m_lastEvent;
			task->setEventId(m_lastEvent);
		}

		// insert the eventid in the list of active events
		m_eventIds.insert(task->getEventId());
		// add the event to the queue
		m_eventList.push(task);

		// if the list was empty or this event is the top in the list
		// we have to signal it
		signal = (task == m_eventList.top());
	}
#ifdef __DEBUG_SCHEDULER__
	else
		std::clog << "[Error - Scheduler::addTask] Scheduler thread is terminated." << std::endl;
#endif

	m_eventLock.unlock();
	if(signal)
		m_eventSignal.notify_one();

	return task->getEventId();
}

bool Scheduler::stopEvent(uint32_t eventId)
{
	if(!eventId)
		return false;

	m_eventLock.lock();
	// search the event id...
	EventIds::iterator it = m_eventIds.find(eventId);
	if(it != m_eventIds.end())
	{
		// if it is found erase from the list
		m_eventIds.erase(it);
		m_eventLock.unlock();
		return true;
	}

	// this eventid is not valid
	m_eventLock.unlock();
	return false;
}

void Scheduler::stop()
{
	m_eventLock.lock();
	m_threadState = Scheduler::STATE_CLOSING;
	m_eventLock.unlock();
}

void Scheduler::shutdown()
{
	m_eventLock.lock();
	m_threadState = Scheduler::STATE_TERMINATED;
	//this list should already be empty
	while(!m_eventList.empty())
		m_eventList.pop();

	m_eventIds.clear();
	m_eventLock.unlock();
}
