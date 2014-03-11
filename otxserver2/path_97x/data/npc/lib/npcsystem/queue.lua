-- Advanced NPC System (Created by Jiddo),
-- Modified by TheForgottenServer Team,
-- Modified by The OTX Server Team.

if(Queue == nil) then
	Queue = {
		customers = nil,
		handler = nil,
	}

	-- Creates a new queue, connected to the given NpcHandler handler
	function Queue:new(handler)
		local obj = {}
		obj.handler = handler
		obj.customers = {}

		setmetatable(obj, self)
		self.__index = self
		return obj
	end

	-- Assigns a new handler to this queue.
	function Queue:setHandler(newHandler)
		self.handler = newHandler
	end

	-- Pushes a new cid onto the tail of this queue.
	function Queue:push(cid)
		if(isPlayer(cid)) then
			table.insert(self.customers, cid)
		end
	end

	-- Returns true if the given cid is already in the queue.
	function Queue:isInQueue(cid)
		return (isInArray(self.customers, cid))
	end

	-- Removes and returns the first cid from the queue
	function Queue:pop()
		return table.remove(self.customers, 1)
	end

	-- Returns the first cid in the queue, but does not remove it!
	function Queue:peek()
		return self.customers[1]
	end

	-- Returns true if htis queue is empty.
	function Queue:empty()
		return(self:peek() == nil)
	end

	-- Returns the amount of players currently in the queue.
	function Queue:getSize()
		return table.maxn(self.customers)
	end

	-- Returns true if the creature with the given cid can be greeted by this npc.
	function Queue:canGreet(cid)
		if(isPlayer(cid)) then
			return self.handler:isInRange(cid)
		end

		return false
	end

	-- Greets the player with the given cid.
	function Queue:greet(cid)
		if(self.handler ~= nil) then
			self.handler:greet(cid)
		else
			error('No handler assigned to queue!')
		end
	end

	-- Makes sure the next greetable player in the queue is greeted.
	function Queue:greetNext()
		while (not self:empty()) do
			local nextPlayer = self:pop()
			if(self:canGreet(nextPlayer)) then
				if(callback == nil or callback(nextPlayer)) then
					self:greet(nextPlayer)
					return true
				end
			end
		end

		return false
	end
end
