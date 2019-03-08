#include "nec.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

static const uint32_t CARRIER_FREQ = 38000;
static const uint32_t ENVELOPE_FREQ = 1785; // One NEC tick is 560us.

const uint8_t NEC_MESSAGE_LENGTH = 32;

struct necSend nec_init(uint32_t carrier, uint32_t port, uint16_t pin)
{
	// Configure the carrier signal timer.
	uint32_t carrier_div = rcc_ahb_frequency / CARRIER_FREQ;
	timer_set_mode(carrier, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);
	timer_set_oc_mode(carrier, TIM_OC1, TIM_OCM_PWM1);
	timer_set_oc_value(carrier, TIM_OC1, carrier_div / 4);
	timer_set_oc_polarity_high(carrier, TIM_OC1);
	timer_enable_oc_preload(carrier, TIM_OC1);
	timer_set_period(carrier, carrier_div);
	timer_enable_oc_output(carrier, TIM_OC1);
	timer_enable_break_main_output(carrier);

	// Configure the envelope signal timer.
	uint32_t envelope = TIM16;
	uint16_t envelope_irq_index = 21;
	uint32_t envelope_div = rcc_ahb_frequency / ENVELOPE_FREQ;
	timer_set_mode(envelope, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);
	timer_set_oc_mode(envelope, TIM_OC1, TIM_OCM_FORCE_LOW);
	timer_set_oc_polarity_high(envelope, TIM_OC1);
	timer_enable_oc_preload(envelope, TIM_OC1);
	timer_enable_oc_output(envelope, TIM_OC1);
	timer_enable_break_main_output(envelope);
	timer_set_period(envelope, envelope_div);
	timer_enable_irq(envelope, TIM_DIER_UIE);

	// Enable interrupts and reset carrier timer.
	timer_enable_counter(carrier);
	timer_generate_event(carrier, TIM_EGR_UG);

	// Enable interrupts and envelope timer.
	nvic_enable_irq(envelope_irq_index);
	nvic_set_priority(envelope_irq_index, 0);
  timer_enable_counter(envelope);

	// Enable debug led.
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);

	struct necSend sender = { .carrier = carrier,
				  .envelope = envelope,
				  .port = port,
				  .pin = pin };
	return sender;
}

static bool mark(struct necSend *sender, uint8_t ticks)
{
	static uint8_t counter = 0;
	bool completed = false;
	if (counter == 0) {
		timer_set_oc_mode(sender->envelope, TIM_OC1,
				  TIM_OCM_FORCE_HIGH);
		counter++;
	} else if (counter == ticks) {
		counter = 0;
		completed = true;
	} else {
		counter++;
	}

	return completed;
}

static bool space(struct necSend *sender, uint8_t ticks)
{
	static uint8_t counter = 0;
	bool completed = false;
	if (counter == 0) {
		timer_set_oc_mode(sender->envelope, TIM_OC1, TIM_OCM_FORCE_LOW);
		counter++;
	} else if (counter == ticks) {
		counter = 0;
		completed = true;
	} else {
		counter++;
	}

	return completed;
}

enum stateMachine {
	COMM_HDR,
	COMM_HDR_SPC,
	COMM_DATA,
	COMM_DATA_HIGH_SPC,
	COMM_DATA_LOW_SPC,
	COMM_FOOTER,
	COMM_FOOTER_SPC,
};

static enum stateMachine state = COMM_HDR;

static bool completed = true;
static bool ready = false;
static uint32_t message = 0;
static struct necSend *sender = NULL;

void tim16_isr(void)
{
	if (ready) {
		ready = false;
		completed = false;
	}

	if (!completed) {
    if (!sender) {
      return;
    }

		static uint8_t index = 0;
		switch (state) {
		case COMM_HDR:
			if (mark(sender, 16)) {
				state = COMM_HDR_SPC;
        tim16_isr();
			}
			break;
		case COMM_HDR_SPC:
			if (space(sender, 8)) {
				state = COMM_DATA;
        tim16_isr();
			}
			break;
		case COMM_DATA:
			if (index < NEC_MESSAGE_LENGTH) {
				if (mark(sender, 1)) {
					if (message & (1 << index)) {
						state = COMM_DATA_HIGH_SPC;
            tim16_isr();
					} else {
						state = COMM_DATA_LOW_SPC;
            tim16_isr();
					}
					index++;
				}
			} else {
				state = COMM_FOOTER;
        tim16_isr();
			}
			break;
		case COMM_DATA_HIGH_SPC:
			if (space(sender, 3)) {
				state = COMM_DATA;
        tim16_isr();
			}
			break;
		case COMM_DATA_LOW_SPC:
			if (space(sender, 1)) {
				state = COMM_DATA;
        tim16_isr();
			}
			break;
		case COMM_FOOTER:
			if (mark(sender, 1)) {
				state = COMM_FOOTER_SPC;
        tim16_isr();
			}
			break;
		case COMM_FOOTER_SPC:
			if (space(sender, 1)) {
				index = 0;
				completed = true;
				message = 0;
				sender = NULL;
				state = COMM_HDR;
			}
			break;
		}
	}

  timer_clear_flag(TIM16, TIM_SR_UIF);
}

void nec_send(struct necSend *s, uint16_t address, uint8_t command)
{
	while (!completed) {
	}

	sender = s;
	message = (uint32_t)(address | (command << 16) | ((~command) << 24));
  ready = true;
}
