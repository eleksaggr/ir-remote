#include <stdint.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>

#include "nec.h"

enum Code {
	IR_BRIGHTER = 0x00,
	IR_DARKER = 0x01,
	IR_OFF = 0x02,
	IR_ON = 0x03,
	IR_RED = 0x04,
	IR_GREEN = 0x05,
	IR_BLUE = 0x06,
	IR_WHITE = 0x07,
	IR_SCARLET = 0x08,
	IR_LIGHTGREEN = 0x09,
	IR_FLASH = 0x0B,
	IR_ORANGE = 0x0C,
	IR_MINT = 0x0D,
	IR_PURPLE = 0x0E,
	IR_SMOOTH = 0x0F, // Maybe Smooth?
	IR_TANGERINE = 0x10,
	IR_SKY = 0x11,
	IR_ROSE = 0x12,
	IR_FADE = 0x13, // Fade and strobe seem to be the same.
	IR_YELLOW = 0x14,
	IR_AQUA = 0x15,
	IR_PINK = 0x16,
	IR_STROBE = 0x17, // Should be strobe
};

static void setup_clock(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();

	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_TIM16);
	rcc_periph_clock_enable(RCC_TIM17);
}

static void setup_irout(void)
{
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
	gpio_set_af(GPIOB, GPIO_AF0, GPIO9);
}

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
	usart_set_mode(USART1, USART_MODE_RX);

	usart_enable(USART1);
}

static void delay_ms(uint16_t n)
{
	for (uint32_t i = 0; i < 12000 * n; i++) {
		__asm__("nop");
	}
}

int main(void)
{
	setup_clock();
	setup_irout();
	struct necSend handle = nec_init(TIM17, GPIOB, GPIO9);
	setup_usart();

  const uint16_t address = 0xEF00;
	while (1) {
		/* uint16_t data = 0; */
		/* while (!(data = usart_recv_blocking(USART1))) */
		/* 	; */
		for (int i = 0; i < 3; i++) {
			nec_send(&handle, address, IR_OFF);
			delay_ms(100);
		}
    gpio_set(GPIOC, GPIO9);
	}

	return 0;
}
