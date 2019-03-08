#include "nec.h"

#include <stdbool.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

static const uint32_t outPort = GPIOB;
static const enum rcc_periph_clken outPortRcc = RCC_GPIOB;
static const uint16_t outPin = GPIO9;

struct Timer {
	uint32_t tim;
	enum rcc_periph_clken rcc;
	uint32_t divider;
};

static const struct Timer timCarrier = { TIM17, RCC_TIM17, 1264 };
static const struct Timer timEnvelope = { TIM16, RCC_TIM16, 26881 };

void NECInit(void)
{
	// Enable timer RCCs.
	rcc_periph_clock_enable(timCarrier.rcc);
	rcc_periph_clock_enable(timEnvelope.rcc);

	// Setup output pin.
	rcc_periph_clock_enable(outPortRcc);
	gpio_mode_setup(outPort, GPIO_MODE_AF, GPIO_PUPD_NONE, outPin);
	gpio_set_af(outPort, GPIO_AF0, outPin);

	/* rcc_periph_clock_enable(RCC_GPIOB); */
	/* gpio_mode_setup(outPort, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8); */
	/* gpio_set_af(GPIOB, GPIO_AF2, GPIO8); */

	// Enable debug led.
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO8);

	// Configure the carrier signal timer.
	timer_set_mode(timCarrier.tim, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);
	timer_set_period(timCarrier.tim, timCarrier.divider);
	timer_set_oc_value(timCarrier.tim, TIM_OC1, timCarrier.divider / 4);
	timer_set_oc_mode(timCarrier.tim, TIM_OC1, TIM_OCM_PWM1);
	timer_enable_oc_preload(timCarrier.tim, TIM_OC1);
	timer_set_oc_polarity_high(timCarrier.tim, TIM_OC1);
	timer_enable_oc_output(timCarrier.tim, TIM_OC1);
	timer_enable_break_main_output(timCarrier.tim);

	// Configure the envelope signal timer.
	timer_set_mode(timEnvelope.tim, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);
	timer_set_period(timEnvelope.tim, timEnvelope.divider);
	timer_set_oc_mode(timEnvelope.tim, TIM_OC1, TIM_OCM_INACTIVE);
	timer_set_oc_polarity_high(timEnvelope.tim, TIM_OC1);
	timer_enable_oc_preload(timEnvelope.tim, TIM_OC1);
	timer_enable_break_main_output(timEnvelope.tim);
	timer_enable_oc_output(timEnvelope.tim, TIM_OC1);
	timer_enable_irq(timEnvelope.tim, TIM_DIER_UIE);

	// Enable and reset.
	timer_enable_counter(timCarrier.tim);
	timer_generate_event(timCarrier.tim, TIM_EGR_UG);

	nvic_enable_irq(21);
	nvic_set_priority(21, 0);

	timer_enable_counter(timEnvelope.tim);
}

static const uint8_t necMessageLen = 32;

static void mark(void)
{
	timer_set_oc_mode(timEnvelope.tim, TIM_OC1, TIM_OCM_FORCE_HIGH);
}

static void space(void)
{
	timer_set_oc_mode(timEnvelope.tim, TIM_OC1, TIM_OCM_FORCE_LOW);
}

enum transmissionState {
	STATE_HDR_MARK,
	STATE_HDR_SPACE,
	STATE_DATA_MARK,
	STATE_DATA_HIGH_SPACE,
	STATE_DATA_LOW_SPACE,
	STATE_FINAL_SPACE,
};

static enum transmissionState state = STATE_HDR_MARK;

static uint32_t nextMessage = 0;
static bool sendReady = false;
static bool sendCompleted = true;

void tim16_isr(void)
{
	static bool changedState = true;
	static uint8_t ticks = 0;
	static uint8_t index = 0;

	if (changedState) {
		// We transitioned into another state.
		switch (state) {
		case STATE_HDR_MARK:
			ticks = 16;
			break;
		case STATE_HDR_SPACE:
			ticks = 8;
			break;
		case STATE_DATA_MARK:
			ticks = 3;
			break;
		default:
			break;
		}
		changedState = false;
	}

  if(sendReady) {
    sendReady = false;
    sendCompleted = false;
  }

	if (!sendCompleted) {
		switch (state) {
		case STATE_HDR_MARK:
			mark();

			ticks--;
			if (!ticks) {
				state = STATE_HDR_SPACE;
				changedState = true;
			}
			break;
		case STATE_HDR_SPACE:
			space();

			ticks--;
			if (!ticks) {
				state = STATE_DATA_MARK;
				changedState = true;
			}
			break;
		case STATE_DATA_MARK:
      mark();
			if (index < necMessageLen) {
				if (nextMessage & (1 << index)) {
					state = STATE_DATA_HIGH_SPACE;
					changedState = true;
				} else {
					state = STATE_DATA_LOW_SPACE;
					changedState = true;
				}
				index++;
			} else {
				state = STATE_FINAL_SPACE;
				changedState = true;
			}
			break;
		case STATE_DATA_HIGH_SPACE:
			space();

			ticks--;
			if (!ticks) {
				state = STATE_DATA_MARK;
				changedState = true;
			}
			break;
		case STATE_DATA_LOW_SPACE:
			space();

			state = STATE_DATA_MARK;
			changedState = true;
			break;
		/* case STATE_FINAL_MARK: */
    /*   mark(); */

    /*   state = STATE_FINAL_SPACE; */
    /*   changedState = true; */
		/* 	break; */
    case STATE_FINAL_SPACE:
      space();

      sendReady = false;
      sendCompleted = true;
      index = 0;

      state = STATE_HDR_MARK;
      changedState = true;
      break;
		}
	}

	// Clear the envelope timer's update interrupt flag.
	timer_clear_flag(timEnvelope.tim, TIM_SR_UIF);
}

void NECSend(uint32_t data)
{
	while (!sendCompleted) {
	}

	nextMessage = data;
	sendReady = true;
}
