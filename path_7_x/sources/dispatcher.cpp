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

#include "outputmessage.h"
#if defined __EXCEPTION_TRACER__
#include "exception.h"
#endif

#include "game.h"

extern Game g_game;

Dispatcher::DispatcherState Dispatcher::m_threadState = Dispatcher::STATE_TERMINATED;

Dispatcher::Dispatcher()
{
	Dispatcher::m_threadState = Dispatcher::STATE_RUNNING;
	m_thread = boost::thread(boost::bind(&Dispatcher::dispatcherThread, this));
}

void Dispatcher::dispatcherThread()
{
	#if defined __EXCEPTION_TRACER__
	ExceptionHandler dispatcherExceptionHandler;
	dispatcherExceptionHandler.InstallHandler();
	#endif

	boost::unique_lock<boost::mutex> taskLockUnique(m_taskLock, boost::defer_lock);
	while(Dispatcher::m_threadState != Dispatcher::STATE_TERMINATED)
	{
		// check if there are tasks waiting
		taskLockUnique.lock();
		if(m_taskList.empty()) //if the list is empty wait for signal
			m_taskSignal.wait(taskLockUnique);

		if(!m_taskList.empty() && Dispatcher::m_threadState != Dispatcher::STATE_TERMINATED)
		{
			// take the first task
			Task* task = m_taskList.front();
			m_taskList.pop_front();
			taskLockUnique.unlock();

			if(!task->hasExpired())
			{
				(*task)();

				g_game.clearSpectatorCache();
			}
			delete task;
		} else {
			taskLockUnique.unlock();
		}
	}

	#if defined __EXCEPTION_TRACER__
	dispatcherExceptionHandler.RemoveHandler();
	#endif
}

void Dispatcher::addTask(Task* task, bool front/* = false*/)
{
	bool signal = false;
	m_taskLock.lock();
	if(Dispatcher::m_threadState == Dispatcher::STATE_RUNNING)
	{
		signal = m_taskList.empty();
		if(front)
			m_taskList.push_front(task);
		else
			m_taskList.push_back(task);
	}
	#ifdef __DEBUG_SCHEDULER__
	else
		std::clog << "[Error - Dispatcher::addTask] Dispatcher thread is terminated." << std::endl;
	#endif

	m_taskLock.unlock();
	// send a signal if the list was empty
	if(signal)
		m_taskSignal.notify_one();
}

void Dispatcher::flush()
{
	Task* task = NULL;
	while(!m_taskList.empty())
	{
		task = m_taskList.front();
		m_taskList.pop_front();

		(*task)();
		delete task;

		g_game.clearSpectatorCache();
	}
}

void Dispatcher::stop()
{
	m_taskLock.lock();
	m_threadState = Dispatcher::STATE_CLOSING;
	m_taskLock.unlock();
}

void Dispatcher::shutdown()
{
	m_taskLock.lock();
	m_threadState = Dispatcher::STATE_TERMINATED;

	flush();
	m_taskLock.unlock();
}
