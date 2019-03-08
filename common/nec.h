#ifndef NEC_H
#define NEC_H

#include <stdint.h>

struct necSend {
	uint32_t carrier;
	uint32_t envelope;

	uint32_t port;
	uint16_t pin;
};

/**
 * @brief      Initializes a NEC sender using the provided timer.
 *
 * @details    Initializes a NEC sender with the provided timer. It outputs the modulated NEC signals on the provided pin.
 *             The timer must support PWM.
 *             This function also makes use of TIM16, so it's clock must be enabled.
 *             The function does not enable any clocks for peripherals and does also not set the alternate function on the pin needed for IR_OUT.
 *             Also note that the output on the IR_OUT pin will be inverted.
 *
 * @param      carrier The timer that generates the 38kHz carrier signal. This timer must support PWM.
 * @param      port The port of the pin the modulated signal will be output on.
 * @param      pin The pin the modulated signal will be output on.
 *
 * @return     A handle to the initialized NEC sender.
 */
struct necSend nec_init(uint32_t, uint32_t, uint16_t);

/**
 * @brief      Sends a address and command encoded as a NEC modulated signal.
 *
 * @param      s The handle to a configured NEC sender instance.
 *             address The address part of the data.
 *             command The command part of the data.
 */
void nec_send(struct necSend *, uint16_t, uint8_t);
#endif /* NEC_H */
