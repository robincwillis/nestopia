////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#include "../NstMapper.hpp"
#include "NstMapper032.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Mapper32::Mapper32(Context& c)
		:
		Mapper (c,CROM_MAX_256K | (c.attribute == ATR_NMT_1K ? NMT_ZERO : NMT_DEFAULT)),
		nmt1k  (c.attribute == ATR_NMT_1K)
		{}

		void Mapper32::SubReset(const bool hard)
		{
			Map( 0x8000U, 0x8FFFU, &Mapper32::Poke_8000 );
			Map( 0x9000U, 0x9FFFU, &Mapper32::Poke_9000 );
			Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_1 );

			for (uint i=0xB000; i < 0xC000; i += 0x8)
			{
				Map( i + 0x0, CHR_SWAP_1K_0 );
				Map( i + 0x1, CHR_SWAP_1K_1 );
				Map( i + 0x2, CHR_SWAP_1K_2 );
				Map( i + 0x3, CHR_SWAP_1K_3 );
				Map( i + 0x4, CHR_SWAP_1K_4 );
				Map( i + 0x5, CHR_SWAP_1K_5 );
				Map( i + 0x6, CHR_SWAP_1K_6 );
				Map( i + 0x7, CHR_SWAP_1K_7 );
			}

			if (hard)
			{
				regs[0] = 0;
				regs[1] = 0;

				prg.SwapBanks<SIZE_8K,0x0000>(0U,~0U,~1U,~0U);
			}

			if (nmt1k)
				ppu.SetMirroring( Ppu::NMT_ZERO );
		}

		void Mapper32::SubLoad(State::Loader& state)
		{
			while (const dword chunk = state.Begin())
			{
				if (chunk == AsciiId<'B','N','K'>::V)
				{
					State::Loader::Data<2> data( state );

					regs[0] = data[0];
					regs[1] = data[1];
				}

				state.End();
			}
		}

		void Mapper32::SubSave(State::Saver& state) const
		{
			state.Begin( AsciiId<'B','N','K'>::V ).Write16( regs[0] | regs[1] << 8 ).End();
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Mapper32::UpdatePrg()
		{
			prg.SwapBank<SIZE_8K,0x0000>( (regs[1] & 0x2) ? ~1U : regs[0] );
			prg.SwapBank<SIZE_8K,0x4000>( (regs[1] & 0x2) ? regs[0] : ~1U );
		}

		NES_POKE_D(Mapper32,8000)
		{
			regs[0] = data;
			UpdatePrg();
		}

		NES_POKE_D(Mapper32,9000)
		{
			regs[1] = data;
			UpdatePrg();

			if (!nmt1k)
				ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_HORIZONTAL : Ppu::NMT_VERTICAL );
		}
	}
}
