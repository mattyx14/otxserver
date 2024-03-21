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

#ifndef __RSA__
#define __RSA__

#include "otsystem.h"
#include <gmp.h>

class RSA
{
	public:
		RSA();
		virtual ~RSA();

		void initialize(const char* p, const char* q, const char* d);
		bool initialize(const std::string& file);

		void decrypt(char* msg);

		void getPublicKey(char* buffer);

	protected:
		boost::recursive_mutex rsaLock;
		mpz_t m_p, m_q, m_u, m_d, m_dp, m_dq, m_mod;
};
#endif
