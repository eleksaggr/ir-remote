# UART IR-Remote for generic 24-key IR RGB LED controller based on STM32F0

![Picture of a 24-key IR RGB LED strip controller - Brand näve][controller]
![Picture of the remote that controls the RGB LED Strip - Brand näve][remote]

The board used here was a STM32F0Discovery board.

# Code

First the application waits for an incoming data byte on the UART interface.
Then a value of 48 is subtracted (`data - '0'` for convenience) from the data byte and it is sent out using the IR LED.

You can send anything you want, by using the `nec_send` function from `nec.h`.
To do this first initialize a handle by calling `nec_init` with the appropriate parameters and then you are good to go.
Note: If you do not have a TIM16, or it is used otherwise you need to adjust the code in `nec.c` not to use it.

# Protocol

The controller uses the NEC protocol. If yours looks anything like mine, it's safe to assume it will use NEC too. A nice overview of the protocol can be found [here](https://www.sbprojects.net/knowledge/ir/nec.php).
To be precise the controller uses the "Extended NEC" protocol from the above site.

I found the controller listened for the address `0xEF00`. Other sources on the web have different addresses, so either use your remote to capture a signal and read the address from it, or try some out.

The values for the commands can be taken from the table below.

| Code | Action              |
| ---- | ------------------- |
|  0x0 | Increase brightness |
|  0x1 | Decrease brightness |
|  0x2 | Off                 |
|  0x3 | On                  |
|  0x4 | Red                 |
|  0x5 | Green               |
|  0x6 | Blue                |
|  0x7 | White               |
|  0x8 | Scarlet             |
|  0x9 | Light green         |
|  0xA | Periwinkle          |
|  0xB | Flash               |
|  0xC | Orange              |
|  0xD | Mint                |
|  0xE | Purple              |
|  0xF | Strobe              |
| 0x10 | Tangerine           |
| 0x11 | Sky                 |
| 0x12 | Rose                |
| 0x13 | Fade                |
| 0x14 | Yellow              |
| 0x15 | Aqua                |
| 0x16 | Pink                |
| 0x17 | Smooth              |

It may be interesting to note, that the command codes simply enumerate the keys on the remote - first by column, then by row.

[controller]: https://github.com/zillolo/ir-remote/raw/master/fig/controller.jpg
[remote]: https://github.com/zillolo/ir-remote/raw/master/fig/remote.jpg
