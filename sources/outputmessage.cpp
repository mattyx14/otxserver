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

#include "outputmessage.h"
#include "protocol.h"
#include "scheduler.h"

#ifdef __OTSERV_ALLOCATOR__
#include "allocator.h"
#endif

const uint16_t OUTPUTMESSAGE_FREE_LIST_CAPACITY = 24768;
const std::chrono::milliseconds OUTPUTMESSAGE_AUTOSEND_DELAY {3};

void OutputMessagePool::sendAll()
{
	//dispatcher thread
	for (auto& protocol : bufferedProtocols) {
		auto& msg = protocol->getCurrentBuffer();
		if (msg) {
			protocol->send(std::move(msg));
		}
	}

	if (!bufferedProtocols.empty()) {
		scheduleSendAll();
	}
}

void OutputMessagePool::addProtocolToAutosend(Protocol_ptr protocol)
{
	//dispatcher thread
	if (bufferedProtocols.empty()) {
		scheduleSendAll();
	}
	bufferedProtocols.emplace_back(protocol);
}

void OutputMessagePool::removeProtocolFromAutosend(const Protocol_ptr& protocol)
{
	//dispatcher thread
	auto it = std::find(bufferedProtocols.begin(), bufferedProtocols.end(), protocol);
	if (it != bufferedProtocols.end()) {
		std::swap(*it, bufferedProtocols.back());
		bufferedProtocols.pop_back();
	}
}

OutputMessage_ptr OutputMessagePool::getOutputMessage()
{
#ifdef __OTSERV_ALLOCATOR__
	// Allocate OutputMessage from PoolManager; keep shared_ptr control block default.
	void* mem = PoolManager::getInstance()->allocate(sizeof(OutputMessage));
	if (!mem) {
		throw std::bad_alloc();
	}

	OutputMessage* obj = new (mem) OutputMessage();

	return OutputMessage_ptr(obj, [](OutputMessage* p) {
		if (!p) {
			return;
		}
		p->~OutputMessage();
		PoolManager::getInstance()->deallocate(static_cast<void*>(p));
	});
#else
	// Fallback when custom allocator is disabled.
	return std::make_shared<OutputMessage>();
#endif
}

void OutputMessagePool::scheduleSendAll()
{
	g_scheduler.addEvent(
		createSchedulerTask(
			static_cast<uint32_t>(OUTPUTMESSAGE_AUTOSEND_DELAY.count()),
			std::bind(&OutputMessagePool::sendAll, this)
		)
	);
}
