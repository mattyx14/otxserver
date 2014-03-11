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

#ifdef __OTSERV_ALLOCATOR__
#ifndef __ALLOCATOR__
#define __ALLOCATOR__

#include "otsystem.h"
#include <boost/pool/pool.hpp>

#include <memory>
#include <cstdlib>
#include <fstream>

template<typename T>
class dummyallocator
{
	public:
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		template<class U> struct rebind
		{
			typedef dummyallocator<U> other;
		};

		template <class U>
		dummyallocator(const dummyallocator<U>&) throw() {}
		dummyallocator(const dummyallocator&) throw() {}

		dummyallocator() throw() {}
		virtual ~dummyallocator() throw() {}

		pointer address(reference x) const {return &x;}
		const_pointer address(const_reference x) const {return &x;}
		pointer allocate(size_type n, void* hint32_t = 0)
		{
			return static_cast<T*>(std::malloc(n * sizeof(T)));
		}

		void deallocate(pointer p, size_type n)
		{
			std::free(static_cast<void*>(p));
		}

		size_type max_size() const throw()
		{
			return std::numeric_limits<size_type>::max() / sizeof(T);
		}

		void construct(pointer p, const T& val)
		{
			new(static_cast<void*>(p))T(val);
		}

		void destroy(pointer p)
		{
			p->~T();
		}
};

void* operator new(size_t bytes, int32_t dummy);
void* operator new(size_t bytes);
void* operator new[](size_t bytes);
void operator delete(void* p);
void operator delete[](void* p);

#ifdef _MSC_VER
void* operator new[](size_t bytes, int32_t dummy)
void operator delete(void* p, int32_t dummy);
void operator delete[](void* p, int32_t dummy);
#endif

#ifdef __OTSERV_ALLOCATOR_STATS__
void allocatorStatsThread(void* a);
#endif

struct poolTag
{
	size_t poolbytes;
};

class PoolManager
{
	public:
		static PoolManager* getInstance()
		{
			static PoolManager instance;
			return &instance;
		}

		void* allocate(size_t size)
		{
			Pools::iterator it;
			poolLock.lock();
			for(it = pools.begin(); it != pools.end(); ++it)
			{
				if(it->first >= size + sizeof(poolTag))
				{
					poolTag* tag = reinterpret_cast<poolTag*>(it->second->malloc());
					#ifdef __OTSERV_ALLOCATOR_STATS__
					if(!tag)
						dumpStats();

					#endif
					tag->poolbytes = it->first;
					#ifdef __OTSERV_ALLOCATOR_STATS__
					poolsStats[it->first]->allocations++;
					poolsStats[it->first]->unused+= it->first - (size + sizeof(poolTag));
					#endif

					poolLock.unlock();
					return tag + 1;
				}
			}

			poolTag* tag = reinterpret_cast<poolTag*>(std::malloc(size + sizeof(poolTag)));
			#ifdef __OTSERV_ALLOCATOR_STATS__
			poolsStats[0]->allocations++;
			poolsStats[0]->unused += size;

			#endif
			tag->poolbytes = 0;
			poolLock.unlock();
			return tag + 1;
		}

		void deallocate(void* deletable)
		{
			if(!deletable)
				return;

			poolTag* const tag = reinterpret_cast<poolTag*>(deletable) - 1U;
			poolLock.lock();
			if(tag->poolbytes)
			{
				Pools::iterator it;
				it = pools.find(tag->poolbytes);

				it->second->free(tag);
				#ifdef __OTSERV_ALLOCATOR_STATS__
				poolsStats[it->first]->deallocations++;
				#endif
			}
			else
			{
				std::free(tag);
				#ifdef __OTSERV_ALLOCATOR_STATS__
				poolsStats[0]->deallocations++;
				#endif
			}

			poolLock.unlock();
		}

		#ifdef __OTSERV_ALLOCATOR_STATS__
		void dumpStats()
		{
			time_t rawtime;
			time(&rawtime);
			std::ofstream output("data/logs/memory_dump.log", std::ios_base::app);

			output << "OTServ Allocator Stats: " << std::ctime(&rawtime) << std::endl;
			for(PoolsStats::iterator it = poolsStats.begin(); it != poolsStats.end(); ++it)
			{
				output << (int32_t)(it->first) << " alloc: " << (int64_t)(it->second->allocations) << " dealloc: ";
				output << (int64_t)(it->second->deallocations) << " unused: " << (int64_t)(it->second->unused);
				if(it->second->allocations != 0 && it->first != 0)
				{
					output << " avg: " << (int64_t)((it->first) - (it->second->unused) / (it->second->allocations));
					output << " %unused: " << (int64_t)((it->second->unused) * 100 / (it->second->allocations) / (it->first));
				}

				output << " N: " << ((int64_t)(it->second->allocations) - (int64_t)(it->second->deallocations)) << std::endl;
			}

			output << std::endl;
			output.close();
		}
		#endif

		virtual ~PoolManager()
		{
			Pools::iterator it = pools.begin();
			while(it != pools.end())
			{
				std::free(it->second);
				it = pools.erase(it);
			}

			#ifdef __OTSERV_ALLOCATOR_STATS__
			for(PoolsStats::iterator sit = poolsStats.begin(); sit != poolsStats.end();)
			{
				std::free(sit->second);
				sit = poolsStats.erase(sit);
			}
			#endif
		}

	private:
		void addPool(size_t size, size_t nextSize)
		{
			pools[size] = new(0) boost::pool<boost::default_user_allocator_malloc_free>(size, nextSize);
			#ifdef __OTSERV_ALLOCATOR_STATS__

			t_PoolStats * tmp = new(0) t_PoolStats;
			tmp->unused = tmp->allocations = tmp->deallocations = 0;
			poolsStats[size] = tmp;
			#endif
		}

		PoolManager()
		{
			addPool(4 + sizeof(poolTag), 32768);
			addPool(20 + sizeof(poolTag), 32768);
			addPool(32 + sizeof(poolTag), 32768);
			addPool(48 + sizeof(poolTag), 32768);
			addPool(96 + sizeof(poolTag), 16384);
			addPool(128 + sizeof(poolTag), 1024);
			addPool(384 + sizeof(poolTag), 2048);
			addPool(512 + sizeof(poolTag), 128);
			addPool(1024 + sizeof(poolTag), 128);
			addPool(8192 + sizeof(poolTag), 128);
			addPool(16384 + sizeof(poolTag), 128);

			addPool(60 + sizeof(poolTag), 10000); //Tile class
			addPool(36 + sizeof(poolTag), 10000); //Item class

			#ifdef __OTSERV_ALLOCATOR_STATS__
			t_PoolStats * tmp = new(0) t_PoolStats;
			tmp->unused = tmp->allocations = tmp->deallocations = 0;
			poolsStats[0] = tmp;
			#endif
		}

		PoolManager(const PoolManager&);
		const PoolManager& operator=(const PoolManager&);

		typedef std::map<size_t, boost::pool<boost::default_user_allocator_malloc_free >*, std::less<size_t >,
			dummyallocator<std::pair<const size_t, boost::pool<boost::default_user_allocator_malloc_free>* > > > Pools;
		Pools pools;

		#ifdef __OTSERV_ALLOCATOR_STATS__
		struct t_PoolStats
		{
			int64_t allocations, deallocations, unused;
		};

		typedef std::map<size_t, t_PoolStats*, std::less<size_t >, dummyallocator<std::pair<const size_t, t_PoolStats* > > > PoolsStats;
		PoolsStats poolsStats;

		#endif
		boost::recursive_mutex poolLock;
};
#endif
#endif
