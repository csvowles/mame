// license:BSD-3-Clause
// copyright-holders:Mike Coates
/*************************************************************************

    Rainbow Islands

*************************************************************************/
#ifndef MAME_INCLUDES_RBISLAND_H
#define MAME_INCLUDES_RBISLAND_H

#pragma once


#include "taitocchip.h"

#include "pc080sn.h"
#include "pc090oj.h"
#include "machine/timer.h"
#include "emupal.h"

class rbisland_state : public driver_device
{
public:
	rbisland_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_cchip(*this, "cchip"),
		m_pc080sn(*this, "pc080sn"),
		m_pc090oj(*this, "pc090oj"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_cchip_irq_clear(*this, "cchip_irq_clear")
	{ }

	void jumping(machine_config &config);
	void rbisland(machine_config &config);
	void jumpingi(machine_config &config);

	void init_jumping();
	void init_rbisland();

protected:
	virtual void machine_start() override;

private:
	void jumping_sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t jumping_latch_r();
	void jumping_spritectrl_w(offs_t offset, uint16_t data);
	void bankswitch_w(uint8_t data);
	void counters_w(uint8_t data);
	DECLARE_VIDEO_START(jumping);
	void rbisland_colpri_cb(u32 &sprite_colbank, u32 &pri_mask, u16 sprite_ctrl);
	uint32_t screen_update_rainbow(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_jumping(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(cchip_timer);
	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(cchip_irq_clear_cb);

	void jumping_map(address_map &map);
	void jumping_sound_map(address_map &map);
	void rbisland_map(address_map &map);
	void rbisland_sound_map(address_map &map);

	/* memory pointers */
	optional_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	uint16_t      m_sprite_ctrl = 0;
	uint16_t      m_sprites_flipscreen = 0;

	/* misc */
	uint8_t       m_jumping_latch = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<taito_cchip_device> m_cchip;
	required_device<pc080sn_device> m_pc080sn;
	optional_device<pc090oj_device> m_pc090oj;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<timer_device> m_cchip_irq_clear;
};

#endif // MAME_INCLUDES_RBISLAND_H
