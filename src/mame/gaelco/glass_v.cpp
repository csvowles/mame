// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

  Glass Video Hardware

  Functions to emulate the video hardware of the machine

***************************************************************************/

#include "emu.h"
#include "glass.h"

/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

/*
    Tile format
    -----------

    Screen 0 & 1: (32*32, 16x16 tiles)

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | xxxxxxxx xxxxxxxx | code
      1  | -------- ---xxxxx | color (uses colors 0x200-0x3ff)
      1  | -------- --x----- | not used?
      1  | -------- -x------ | flip x
      1  | -------- x------- | flip y
      1  | xxxxxxxx -------- | not used
*/

template<int Layer>
TILE_GET_INFO_MEMBER(glass_state::get_tile_info)
{
	int data = m_videoram[(Layer * 0x1000 / 2) + (tile_index << 1)];
	int data2 = m_videoram[(Layer * 0x1000 / 2) + (tile_index << 1) + 1];
	int code = ((data & 0x03) << 14) | ((data & 0x0fffc) >> 2);

	tileinfo.set(0, code, 0x20 + (data2 & 0x1f), TILE_FLIPYX((data2 & 0xc0) >> 6));
}

/***************************************************************************

    Blitter

***************************************************************************/

/*
    The blitter is accessed writing 5 consecutive bits. The stream is: P0 P1 B2 B1 B0

    if P0 is set, the hardware selects the first half of ROM H9 (girls)
    if P1 is set, the hardware selects the second half of ROM H9 (boys)

    B2B1B0 selects the picture (there are 8 pictures in each half of the ROM)
*/

void glass_state::blitter_w(uint16_t data)
{
	m_blitter_command = ((m_blitter_command << 1) | (data & 0x01)) & 0x1f;
	m_current_bit++;

	if (m_current_bit == 5)
	{
		m_current_bit = 0;

		/* fill the screen bitmap with the current picture */
		{
			uint8_t const *gfx = m_bmap + (m_blitter_command & 0x07) * 0x10000 + (m_blitter_command & 0x08) * 0x10000 + 0x140;

			if ((m_blitter_command & 0x18) != 0)
			{
				for (int j = 0; j < 200; j++)
				{
					for (int i = 0; i < 320; i++)
					{
						int const color = *gfx;
						gfx++;
						m_screen_bitmap->pix(j, i) = color & 0xff;
					}
				}
			}
			else
				m_screen_bitmap->fill(0);
		}
	}
}

/***************************************************************************

    Memory Handlers

***************************************************************************/

void glass_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_videoram[offset]);
	m_pant[offset >> 11]->mark_tile_dirty(((offset << 1) & 0x0fff) >> 2);
}


/***************************************************************************

    Start/Stop the video hardware emulation.

***************************************************************************/

void glass_state::video_start()
{
	m_pant[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(glass_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_pant[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(glass_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_screen_bitmap = std::make_unique<bitmap_ind16>(320, 200);

	save_item(NAME(*m_screen_bitmap));

	m_pant[0]->set_transparent_pen(0);
	m_pant[1]->set_transparent_pen(0);
}


/***************************************************************************

    Sprites

***************************************************************************/

/*
    Sprite Format
    -------------

    Word | Bit(s)            | Description
    -----+-FEDCBA98-76543210-+--------------------------
      0  | -------- xxxxxxxx | y position
      0  | --xxxxxx -------- | not used?
      0  | -x------ -------- | flipx
      0  | x------- -------- | flipy
      1  | xxxxxxxx xxxxxxxx | not used?
      2  | -------x xxxxxxxx | x position
      2  | ---xxxx- -------- | sprite color (uses colors 0x100-0x1ff)
      2  | xx------ -------- | not used?
      3  | xxxxxxxx xxxxxxxx | sprite code
*/

void glass_state::draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int i;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (i = 3; i < (0x1000 - 6) / 2; i += 4)
	{
		int sx = m_spriteram[i + 2] & 0x01ff;
		int sy = (240 - (m_spriteram[i] & 0x00ff)) & 0x00ff;
		int number = m_spriteram[i + 3];
		int color = (m_spriteram[i + 2] & 0x1e00) >> 9;
		int attr = (m_spriteram[i] & 0xfe00) >> 9;

		int xflip = attr & 0x20;
		int yflip = attr & 0x40;

		number = ((number & 0x03) << 14) | ((number & 0x0fffc) >> 2);

		gfx->transpen(bitmap,cliprect,number,
				0x10 + (color & 0x0f),xflip,yflip,
				sx-0x0f,sy,0);
	}
}

/***************************************************************************

    Display Refresh

****************************************************************************/

uint32_t glass_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* set scroll registers */
	m_pant[0]->set_scrolly(0, m_vregs[0]);
	m_pant[0]->set_scrollx(0, m_vregs[1] + 0x04);
	m_pant[1]->set_scrolly(0, m_vregs[2]);
	m_pant[1]->set_scrollx(0, m_vregs[3]);

	/* draw layers + sprites */
	bitmap.fill(m_palette->black_pen(), cliprect);
	copybitmap(bitmap, *m_screen_bitmap, 0, 0, 0x18, 0x24, cliprect);
	m_pant[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_pant[0]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}
