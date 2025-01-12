// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

Syusse Oozumou
(c) 1984 Technos Japan (Licensed by Data East)

Driver by Takahiro Nogi (nogi@kt.rim.or.jp) 1999/10/04

***************************************************************************/

#include "emu.h"
#include "ssozumo.h"

/**************************************************************************/

void ssozumo_state::ssozumo_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0 ; i < 64 ; i++)
	{
		int bit0, bit1, bit2, bit3;

		bit0 = BIT(color_prom[0], 0);
		bit1 = BIT(color_prom[0], 1);
		bit2 = BIT(color_prom[0], 2);
		bit3 = BIT(color_prom[0], 3);
		int const r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(color_prom[0], 4);
		bit1 = BIT(color_prom[0], 5);
		bit2 = BIT(color_prom[0], 6);
		bit3 = BIT(color_prom[0], 7);
		int const g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = BIT(color_prom[64], 0);
		bit1 = BIT(color_prom[64], 1);
		bit2 = BIT(color_prom[64], 2);
		bit3 = BIT(color_prom[64], 3);
		int const b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

void ssozumo_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void ssozumo_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void ssozumo_state::videoram2_w(offs_t offset, uint8_t data)
{
	m_videoram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void ssozumo_state::colorram2_w(offs_t offset, uint8_t data)
{
	m_colorram2[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void ssozumo_state::paletteram_w(offs_t offset, uint8_t data)
{
	int bit0, bit1, bit2, bit3, val;
	int r, g, b;
	int offs2;

	m_paletteram[offset] = data;
	offs2 = offset & 0x0f;

	val = m_paletteram[offs2];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = m_paletteram[offs2 | 0x10];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = m_paletteram[offs2 | 0x20];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	m_palette->set_pen_color(offs2 + 64, rgb_t(r, g, b));
}

void ssozumo_state::scroll_w(uint8_t data)
{
	m_bg_tilemap->set_scrolly(0, data);
}

void ssozumo_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(data & 0x80);
	m_color_bank = data & 3;
	m_fg_tilemap->mark_all_dirty();
}

TILE_GET_INFO_MEMBER(ssozumo_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index] + ((m_colorram[tile_index] & 0x08) << 5);
	int color = (m_colorram[tile_index] & 0x30) >> 4;
	int flags = ((tile_index % 32) >= 16) ? TILE_FLIPY : 0;

	tileinfo.set(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(ssozumo_state::get_fg_tile_info)
{
	int code = m_videoram2[tile_index] + 256 * (m_colorram2[tile_index] & 0x07);

	tileinfo.set(0, code, m_color_bank, 0);
}

void ssozumo_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ssozumo_state::get_bg_tile_info)), TILEMAP_SCAN_COLS_FLIP_X,
			16, 16, 16, 32);

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ssozumo_state::get_fg_tile_info)), TILEMAP_SCAN_COLS_FLIP_X,
			8, 8, 32, 32);

	m_fg_tilemap->set_transparent_pen(0);

	save_item(NAME(m_color_bank));
}

void ssozumo_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		if (m_spriteram[offs] & 0x01)
		{
			int code = m_spriteram[offs + 1] + ((m_spriteram[offs] & 0xf0) << 4);
			int color = (m_spriteram[offs] & 0x08) >> 3;
			int flipx = m_spriteram[offs] & 0x04;
			int flipy = m_spriteram[offs] & 0x02;
			int sx = 239 - m_spriteram[offs + 3];
			int sy = (240 - m_spriteram[offs + 2]) & 0xff;

			if (flip_screen())
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					code, color,
					flipx, flipy,
					sx, sy, 0);
		}
	}
}

uint32_t ssozumo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
