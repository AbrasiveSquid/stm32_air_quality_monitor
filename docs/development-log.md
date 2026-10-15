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

## October 14 - SPS30 driver working
- Able to receive PM measurements
- Refactored user code into separate driver files (sps30.c and sps30.h)
- Currently have functions for start, stop measurements, poll measurements, receive frames, unstuff bytes verify checksum, flip endianness, and an orchestrator function that calls all the functions to convert to pm data. 
- Debugged a number of issues of off by one errors, and array indexing.
- Tested sensor next to a lit candle before and after blowing it out. Observed substantial increase in PM concentration <10 µg/m3 to >1000 µg/m3 after blowing it out.

Next: Test measurements of sensor in various conditions to ensure working.
Test and add handling if sensor is disconnected.
Update get_response_frame to number of bytes received not just 0/1

