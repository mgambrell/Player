/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EP_DYNRPG_EASYRPG_H
#define EP_DYNRPG_EASYRPG_H

#include "game_dynrpg.h"
#include "game_battle.h"
#include "game_map.h"

namespace DynRpg {
	/**
	 * A DynRPG plugin that provides EasyRPG specific built-in functions.
	 * Mostly for testing.
	 */
	class EasyRpgPlugin : public DynRpgPlugin {
	public:
		EasyRpgPlugin(Game_DynRpg& instance) : DynRpgPlugin("EasyRpgPlugin", instance) {}

		bool Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) override;
		void Load(const std::vector<uint8_t>& buffer) override;
		std::vector<uint8_t> Save() override;

	private:
		bool EasyCall(dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter);
	};
}

#endif
