// license:BSD-3-Clause
// copyright-holders:BUT
/*
 *  Chack'n Pop (C) 1983 TAITO Corp.
 *  emulate video hardware
 */

#include "emu.h"
#include "chaknpop.h"

#define GFX_FLIP_X  0x01
#define GFX_FLIP_Y  0x02
#define GFX_VRAM_BANK   0x04
#define GFX_UNKNOWN1    0x08
#define GFX_TX_BANK1    0x20
#define GFX_UNKNOWN2    0x40
#define GFX_TX_BANK2    0x80

#define TX_COLOR1   0x0b
#define TX_COLOR2   0x01


/***************************************************************************
  palette decode
***************************************************************************/

void chaknpop_state::chaknpop_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 1024; i++)
	{
		int bit0, bit1, bit2;

		int const col = (color_prom[i] & 0x0f) | ((color_prom[i + 1024] & 0x0f) << 4);

		// red component
		bit0 = BIT(col, 0);
		bit1 = BIT(col, 1);
		bit2 = BIT(col, 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// green component
		bit0 = BIT(col, 3);
		bit1 = BIT(col, 4);
		bit2 = BIT(col, 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		// blue component
		bit0 = 0;
		bit1 = BIT(col, 6);
		bit2 = BIT(col, 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

/***************************************************************************
  Memory handlers
***************************************************************************/

void chaknpop_state::tx_tilemap_mark_all_dirty()
{
	m_tx_tilemap->mark_all_dirty();
	m_tx_tilemap->set_flip(m_flip_x | m_flip_y);
}

uint8_t chaknpop_state::gfxmode_r()
{
	return m_gfxmode;
}

void chaknpop_state::gfxmode_w(uint8_t data)
{
	if (m_gfxmode != data)
	{
		int all_dirty = 0;

		m_gfxmode = data;
		m_vram_bank->set_entry((m_gfxmode & GFX_VRAM_BANK) ? 1 : 0);   /* Select 2 banks of 16k */

		if (m_flip_x != (m_gfxmode & GFX_FLIP_X))
		{
			m_flip_x = m_gfxmode & GFX_FLIP_X;
			all_dirty = 1;
		}

		if (m_flip_y != (m_gfxmode & GFX_FLIP_Y))
		{
			m_flip_y = m_gfxmode & GFX_FLIP_Y;
			all_dirty = 1;
		}

		if (all_dirty)
			tx_tilemap_mark_all_dirty();
	}
}

void chaknpop_state::txram_w(offs_t offset, uint8_t data)
{
	m_tx_ram[offset] = data;
	m_tx_tilemap->mark_tile_dirty(offset);
}

void chaknpop_state::attrram_w(offs_t offset, uint8_t data)
{
	if (m_attr_ram[offset] != data)
	{
		m_attr_ram[offset] = data;

		if (offset == TX_COLOR1 || offset == TX_COLOR2)
			tx_tilemap_mark_all_dirty();
	}
}


/***************************************************************************
  Callback for the tilemap code
***************************************************************************/

/*
 *  I'm not sure how to handle attributes about color
 */

TILE_GET_INFO_MEMBER(chaknpop_state::get_tx_tile_info)
{
	int tile = m_tx_ram[tile_index];
	int tile_h_bank = (m_gfxmode & GFX_TX_BANK2) << 2;  /* 0x00-0xff -> 0x200-0x2ff */
	int color = m_attr_ram[TX_COLOR2];

	if (tile == 0x74)
		color = m_attr_ram[TX_COLOR1];

	if (m_gfxmode & GFX_TX_BANK1 && tile >= 0xc0)
		tile += 0xc0;                   /* 0xc0-0xff -> 0x180-0x1bf */

	tile |= tile_h_bank;

	tileinfo.set(1, tile, color, 0);
}


/***************************************************************************
  Initialize video hardware emulation
***************************************************************************/

void chaknpop_state::video_start()
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(chaknpop_state::get_tx_tile_info)), TILEMAP_SCAN_ROWS,   8,  8, 32, 32);

	m_gfxmode = 0;
	m_flip_x = 0;
	m_flip_y = 0;

	m_vram_bank->set_entry(0);
	tx_tilemap_mark_all_dirty();

	machine().save().register_postload(save_prepost_delegate(FUNC(chaknpop_state::tx_tilemap_mark_all_dirty), this));
}


/***************************************************************************
  Screen refresh
***************************************************************************/

void chaknpop_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	/* Draw the sprites */
	for (int offs = 0; offs < m_spr_ram.bytes(); offs += 4)
	{
		int sx = m_spr_ram[offs + 3];
		int sy = 256 - 15 - m_spr_ram[offs];
		int flipx = m_spr_ram[offs+1] & 0x40;
		int flipy = m_spr_ram[offs+1] & 0x80;
		int color = (m_spr_ram[offs + 2] & 7);
		int tile = (m_spr_ram[offs + 1] & 0x3f) | ((m_spr_ram[offs + 2] & 0x38) << 3);

		if (m_flip_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}
		if (m_flip_y)
		{
			sy = 242 - sy;
			flipy = !flipy;
		}


				m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				tile,
				color,
				flipx, flipy,
				sx, sy, 0);
	}
}

void chaknpop_state::draw_bitmap( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int dx = m_flip_x ? -1 : 1;

	for (int offs = 0; offs < 0x2000; offs++)
	{
		int x = ((offs & 0x1f) << 3) + 7;
		int y = offs >> 5;

		if (!m_flip_x)
			x = 255 - x;

		if (!m_flip_y)
			y = 255 - y;

		for (int i = 0x80; i > 0; i >>= 1, x += dx)
		{
			pen_t color = 0;

			if (m_vram[offs] & i)
				color |= 0x200; // green lower cage
			if (m_vram[0x2000 + offs] & i)
				color |= 0x080;
			if (m_vram[0x4000 + offs] & i)
				color |= 0x100; // green upper cage
			if (m_vram[0x6000 + offs] & i)
				color |= 0x040; // tx mask

			if (color)
				bitmap.pix(y, x) |= color;
		}
	}
}

uint32_t chaknpop_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	draw_bitmap(bitmap, cliprect);
	return 0;
}
