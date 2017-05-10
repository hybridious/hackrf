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

uint8_t display_address = 0x00;

// A large amount of this is from https://github.com/CaptainStouf/raspberry_lcd4x20_I2C
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

void write_four_bits(uint8_t data) {
	uint8_t byte = data | backlight_state;
	i2c_bus_transfer(&i2c0, ADDRESS, &byte, 1, NULL, 0);
	byte |=  En;
	i2c_bus_transfer(&i2c0, ADDRESS, &byte, 1, NULL, 0);
	delay(102000);
	byte = byte & ~En;
	i2c_bus_transfer(&i2c0, ADDRESS, &byte, 1, NULL, 0);
	delay(20400);
}

void lcd_write(uint8_t cmd) {
	write_four_bits(cmd & 0xF0);
	write_four_bits((cmd << 4) & 0xF0);
}

void lcd_write_char(uint8_t chr) {
	write_four_bits(Rs | (chr & 0xF0));
	write_four_bits(Rs | ((chr << 4) & 0xF0));
}

void lcd_clear(void) {
	lcd_write(LCD_CLEARDISPLAY);
	lcd_write(LCD_RETURNHOME);
}

void hackrf_ui_init(void) {
	int i;
    // write_four_bits(0x30);
    // write_four_bits(0x30);
    // write_four_bits(0x30);
    // write_four_bits(0x20);
    // lcd_write(LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE);
    // lcd_write(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON);
    // lcd_write(LCD_CLEARDISPLAY);
    // lcd_write(LCD_ENTRYMODESET | LCD_ENTRYLEFT);

	const uint8_t data[] = {0x03,0x03,0x03,0x02,
		LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE,
		LCD_DISPLAYCONTROL | LCD_DISPLAYON,
		LCD_CLEARDISPLAY,
		LCD_ENTRYMODESET | LCD_ENTRYLEFT
	};
	for(i=0; i<8; i++)
		lcd_write(data[i]);

	// delay(40800000);

	lcd_write(0x80);
	char freq_string[] = "1Frequency:";
	// uint8_t test[] = {0x46, 0x72, 0x65, 0x71, 0x75, 0x65, 0x6e, 0x63, 0x79};
	for(i=0; i<11; i++) {
		lcd_write_char(freq_string[i]);
		led_toggle(LED2);
	}
	led_on(LED3);
}
