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
#include "trashholder.h"

#include "game.h"
#include "spells.h"
#include "const.h"

extern Game g_game;

void TrashHolder::__addThing(Creature* actor, int32_t, Thing* thing)
{
	if(Item* item = thing->getItem())
	{
		if(item == this || !item->isMovable())
			return;

		if(g_game.isSwimmingPool(this, getTile(), true))
		{
			if(item->getID() == ITEM_WATERBALL_SPLASH)
				return;

			if(item->getID() == ITEM_WATERBALL)
			{
				g_game.transformItem(item, ITEM_WATERBALL_SPLASH);
				return;
			}
		}

		g_game.internalRemoveItem(actor, item);
		if(effect != MAGIC_EFFECT_NONE)
			g_game.addMagicEffect(getPosition(), effect);
	}
	else if(g_game.isSwimmingPool(this, getTile(), false) && thing->getCreature())
	{
		Player* player = thing->getCreature()->getPlayer();
		if(player && player->getPosition() == player->getLastPosition())
		{
			//player has just logged in a swimming pool
			static Outfit_t outfit(SWIMMING_OUTFIT);
			Spell::CreateIllusion(player, outfit, -1);
		}
	}
}