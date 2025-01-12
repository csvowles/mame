// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Curt Coder
/***************************************************************************

  video/lvcards.cpp

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "lvcards.h"


void lvcards_state::lvcards_palette(palette_device &palette) const//Ever so slightly different, but different enough.
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = (color_prom[0] >> 0) & 0x11;
		bit1 = (color_prom[0] >> 1) & 0x11;
		bit2 = (color_prom[0] >> 2) & 0x11;
		bit3 = (color_prom[0] >> 3) & 0x11;
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// green component
		bit0 = (color_prom[palette.entries()] >> 0) & 0x11;
		bit1 = (color_prom[palette.entries()] >> 1) & 0x11;
		bit2 = (color_prom[palette.entries()] >> 2) & 0x11;
		bit3 = (color_prom[palette.entries()] >> 3) & 0x11;
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		// blue component
		bit0 = (color_prom[2 * palette.entries()] >> 0) & 0x11;
		bit1 = (color_prom[2 * palette.entries()] >> 1) & 0x11;
		bit2 = (color_prom[2 * palette.entries()] >> 2) & 0x11;
		bit3 = (color_prom[2 * palette.entries()] >> 3) & 0x11;
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));

		color_prom++;
	}
}

void lvcards_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void lvcards_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(lvcards_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0x30) << 4) + ((attr & 0x80) << 3);
	int color = attr & 0x0f;
	int flags = (attr & 0x40) ? TILE_FLIPX : 0;

	tileinfo.set(0, code, color, flags);
}

void lvcards_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(lvcards_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);
}

uint32_t lvcards_state::screen_update_lvcards(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}
