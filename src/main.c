#include <stdint.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>

#include "nec.h"

enum Code {
	IR_OFF = 0xFD02EF00,
	IR_ON = 0xFC03EF00,
	IR_BRIGHTER = 0xFF00EF00,
	IR_DARKER = 0xFE01EF00,
	IR_RED = 0xF720DF,
	IR_GREEN = 0xFA05EF00,
	IR_BLUE = 0xF906EF00,
	IR_WHITE = 0xF807EF00,
	IR_FLASH = 0xF40BEF00,
	IR_SMOOTH = 0xF00FEF00, // Maybe Smooth?
	IR_FADE = 0xEC13EF00, // Fade and strobe seem to be the same.
	IR_STROBE = 0xE817EF00, // Should be strobe
	IR_SCARLET = 0xF708EF00,
	IR_ORANGE = 0xF30CEF00,
	IR_TANGERINE = 0xEF10EF00,
	IR_YELLOW = 0xEB14EF00,
	IR_LIGHTGREEN = 0xF609EF00,
	IR_MINT = 0xF20DEF00,
	IR_SKY = 0xEE11EF00,
	IR_AQUA = 0xEA15EF00,
	IR_PURPLE = 0xF10EEF00,
	IR_ROSE = 0xED12EF00,
	IR_PINK = 0xE916EF00,
	IR_UNKNOWN = 0xFFFFFF,
};

enum Command {
	CMD_OFF = 0x0,
	CMD_ON = 0x1,
	CMD_BRIGHTER = 0x2,
	CMD_DARKER = 0x3,
	CMD_RED = 0x4,
	CMD_SCARLET = 0x5,
	CMD_ORANGE = 0x6,
	CMD_TANGERINE = 0x7,
	CMD_YELLOW = 0x8,
	CMD_GREEN = 0x9,
	CMD_LIGHTGREEN = 0xA,
	CMD_MINT = 0xB,
	CMD_SKY = 0xC,
	CMD_BLUE = 0xD,
	CMD_AQUA = 0xE,
	CMD_PURPLE = 0xF,
	CMD_ROSE = 0x10,
	CMD_PINK = 0x11,
	CMD_FLASH = 0x12,
	CMD_STROBE = 0x13,
	CMD_FADE = 0x14,
	CMD_SMOOTH = 0x15,
};

static void setup_clock(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();
}

/* static void setup_led(void) */
/* { */
/* 	rcc_periph_clock_enable(RCC_GPIOC); */
/* 	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9); */
/* } */

static void setup_usart(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);
	gpio_set_af(GPIOA, GPIO_AF1, GPIO9 | GPIO10);

	rcc_periph_clock_enable(RCC_USART1);

	usart_set_baudrate(USART1, 38400);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, 1);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_mode(USART1, USART_MODE_TX_RX);

	usart_enable(USART1);
}

static void delay_ms(uint16_t n)
{
	for (uint32_t i = 0; i < 12000 * n; i++) {
		__asm__("nop");
	}
}

static void put(const char *msg)
{
	int i = 0;
	while (msg[i] != 0) {
		usart_send_blocking(USART1, (uint16_t)msg[i++]);
	}
}

static const char *greeting = "Hello.";

int main(void)
{
	setup_clock();
	NECInit();
	setup_usart();

	put(greeting);
	while (1) {
		uint16_t data = 0;
		while (!(data = usart_recv_blocking(USART1)))
			;

		enum Code code = IR_UNKNOWN;
		switch (data) {
		case CMD_OFF:
			code = IR_OFF;
			break;
		case CMD_ON:
			code = IR_ON;
			break;
		case CMD_BRIGHTER:
			code = IR_BRIGHTER;
			break;
		case CMD_DARKER:
			code = IR_DARKER;
			break;
		case CMD_RED:
			code = IR_RED;
			break;
		case CMD_SCARLET:
			code = IR_SCARLET;
			break;
		case CMD_ORANGE:
			code = IR_ORANGE;
			break;
		case CMD_TANGERINE:
			code = IR_TANGERINE;
			break;
		case CMD_YELLOW:
			code = IR_YELLOW;
			break;
		}

		if (code != IR_UNKNOWN) {
			for (int i = 0; i < 3; i++) {
				NECSend(code);
				delay_ms(20);
			}
		}
	}

	return 0;
}
