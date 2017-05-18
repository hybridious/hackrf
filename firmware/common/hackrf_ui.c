/*
 * Copyright 2016 Dominic Spill <dominicgs@gmail.com>
 *
 * This file is part of HackRF.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "hackrf_core.h"
#include "hackrf_ui.h"
#include "gpio_lpc.h"
#include <libopencm3/lpc43xx/scu.h>

// Constants from https://github.com/CaptainStouf/raspberry_lcd4x20_I2C
// Which is released under the GPL2 license

// LCD Address
#define ADDRESS 0x27

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0b00000100 // Enable bit
#define Rw 0b00000010 // Read/Write bit
#define Rs 0b00000001 // Register select bit

uint8_t backlight_state = LCD_BACKLIGHT;

static struct gpio_t gpio_lcd_d[] = {
	GPIO(3, 8),
	GPIO(3, 9),
	GPIO(3, 10),
	GPIO(3, 11),
	GPIO(3, 12),
	GPIO(3, 13),
	GPIO(3, 14),
	GPIO(3, 15)
};

// static struct gpio_t gpio_lcd_d0 = GPIO(3, 8);
// static struct gpio_t gpio_lcd_d1 = GPIO(3, 9);
// static struct gpio_t gpio_lcd_d2 = GPIO(3, 10);
// static struct gpio_t gpio_lcd_d3 = GPIO(3, 11);
// static struct gpio_t gpio_lcd_d4 = GPIO(3, 12);
// static struct gpio_t gpio_lcd_d5 = GPIO(3, 13);
// static struct gpio_t gpio_lcd_d6 = GPIO(3, 14);
// static struct gpio_t gpio_lcd_d7 = GPIO(3, 15);

static struct gpio_t gpio_lcd_rs = GPIO(5, 7);
static struct gpio_t gpio_lcd_rw = GPIO(1, 10);
static struct gpio_t gpio_lcd_en = GPIO(1, 13);

void write_bits(uint8_t byte, bool rs) {
	int i;
	for(i=0; i<8; i++) {
		if(byte&0x1) {
			gpio_set(&gpio_lcd_d[i]);
		} else {
			gpio_clear(&gpio_lcd_d[i]);
		}
		byte >>=1;
	}
	if(rs) {
		gpio_set(&gpio_lcd_rs);
	} else {
		gpio_clear(&gpio_lcd_rs);
	}
	gpio_set(&gpio_lcd_en);
	delay(102000);
	gpio_clear(&gpio_lcd_en);
	delay(20400);
}

void lcd_write(uint8_t byte) {
	write_bits(byte, false);
}

void lcd_write_char(uint8_t byte) {
	write_bits(byte, true);
}

void lcd_clear(void) {
	lcd_write(LCD_CLEARDISPLAY);
	lcd_write(LCD_RETURNHOME);
}

void hackrf_ui_init(void) {
	int i;
	scu_pinmux(SCU_PINMUX_GPIO3_8, SCU_GPIO_PDN | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO3_9, SCU_GPIO_PDN | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO3_10, SCU_GPIO_PDN | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO3_11, SCU_GPIO_PDN | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO3_12, SCU_GPIO_PDN | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO3_13, SCU_GPIO_PDN | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO3_14, SCU_GPIO_PDN | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO3_15, SCU_GPIO_PDN | SCU_CONF_FUNCTION0);

	scu_pinmux(SCU_PINMUX_GPIO5_7, SCU_GPIO_PDN | SCU_CONF_FUNCTION4);
	scu_pinmux(SCU_PINMUX_GPIO1_10, SCU_GPIO_PDN | SCU_CONF_FUNCTION0);
	scu_pinmux(SCU_PINMUX_GPIO1_13, SCU_GPIO_PDN | SCU_CONF_FUNCTION0);

	for(i=0; i<8; i++) {
		gpio_output(&gpio_lcd_d[i]);
		gpio_clear(&gpio_lcd_d[i]);
	}

	gpio_output(&gpio_lcd_rs);
	gpio_output(&gpio_lcd_rw);
	gpio_output(&gpio_lcd_en);

	gpio_clear(&gpio_lcd_rs);
	gpio_clear(&gpio_lcd_rw);
	gpio_clear(&gpio_lcd_en);
	const uint8_t data[] = {
		LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_8BITMODE,
		LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_BLINKON,
		LCD_CLEARDISPLAY,
		LCD_ENTRYMODESET | LCD_ENTRYLEFT
	};
	for(i=0; i<4; i++)
		lcd_write(data[i]);

	lcd_clear();
	lcd_write(0x80);
	char freq_string[] = "Frequency:";
	// // uint8_t test[] = {0x46, 0x72, 0x65, 0x71, 0x75, 0x65, 0x6e, 0x63, 0x79};
	for(i=0; i<10; i++) {
		lcd_write_char(freq_string[i]);
	}
}
