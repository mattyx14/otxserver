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
#include "otsystem.h"

#include <boost/function.hpp>
#define DISPATCHER_TASK_EXPIRATION 2000

class Task
{
	public:
		Task(const boost::function<void (void)>& f): m_expiration(
			boost::date_time::not_a_date_time), m_f(f) {}
		Task(uint32_t ms, const boost::function<void (void)>& f): m_expiration(
			boost::get_system_time() + boost::posix_time::milliseconds(ms)), m_f(f) {}

		virtual ~Task() {}
		void operator()() {m_f();}

		void unsetExpiration() {m_expiration = boost::date_time::not_a_date_time;}
		bool hasExpired() const
		{
			if(m_expiration == boost::date_time::not_a_date_time)
				return false;

			return m_expiration < boost::get_system_time();
		}

	protected:
		boost::system_time m_expiration;
		boost::function<void (void)> m_f;
};

inline Task* createTask(boost::function<void (void)> f)
{
	return new Task(f);
}
inline Task* createTask(uint32_t expiration, boost::function<void (void)> f)
{
	return new Task(expiration, f);
}

class Dispatcher
{
	public:
		virtual ~Dispatcher() {}
		static Dispatcher& getInstance()
		{
			static Dispatcher dispatcher;
			return dispatcher;
		}

		void addTask(Task* task, bool front = false);

		void stop();
		void shutdown();
		void exit() {m_thread.join();}

		void dispatcherThread();

	protected:
		void flush();

		Dispatcher();
		enum DispatcherState
		{
			STATE_RUNNING,
			STATE_CLOSING,
			STATE_TERMINATED
		};

		boost::thread m_thread;
		boost::mutex m_taskLock;
		boost::condition_variable m_taskSignal;

		std::list<Task*> m_taskList;
		static DispatcherState m_threadState;
};
#endif
