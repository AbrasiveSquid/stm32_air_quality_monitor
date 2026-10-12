# Development Log — STM32 Air Quality Monitor

A record of the project's design decisions, firmware work, hardware bring-up, debugging, and remaining questions.

## September 11–12 — Board bring-up
- Built, flashed, and debugged firmware using ST-LINK.
- Controlled the PA5 LED and read the PC13 button. Learned the button is active-low.

## September 20 — SPS30 UART code
- Configured USART2 and began implementing SHDLC framing and measurement parsing.
- Investigated a `memcpy()` pointer-type error. No live PM readings confirmed.

## October 9 — SPS30 communication
- Learned that the Nucleo's UART TX/RX connections depend on solder-bridge configuration; pins aren't necessarily connected to the expected peripheral by default.
- Switched ports to D9/D2 and PA9/PA10 pin.
- Had a wire issue and took a while to debug a bad wire. After switching wires, received 0x7E, but still need to validate a complete response frame.
- Got the sensor fan running and received `0x7E` (SHDLC frame delimiter). A complete valid response is still unconfirmed.

**Next:** Validate a full SPS30 response and decode PM readings; then integrate the e-ink display and SCD41.

