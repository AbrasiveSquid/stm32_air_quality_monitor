# STM32 Air Quality Monitor

An embedded air quality monitor built around an **STM32 NUCLEO-F446RE** development board. The goal is a standalone device that measures airborne particulate matter (PM) and carbon dioxide (CO₂), displays the readings on an e-ink screen, and fits into an enclosure.

**Status:** Work in progress. The current focus is communicating reliably with the SPS30 particulate matter sensor over UART.

## Hardware

| Component | Role | Interface | Status |
| --- | --- | --- | --- |
| STM32 NUCLEO-F446RE | Microcontroller development board | — | Firmware builds and runs |
| Sensirion SPS30 | PM1.0, PM2.5, PM4, and PM10 measurements | UART | Bring-up and debugging |
| Sensirion SCD41 | CO₂ measurements | I²C | Planned |
| Waveshare 2.13-inch e-Paper HAT V4 (black/white) | Local display | SPI | Planned |
| Perfboard and enclosure | Permanent assembly | — | Planned |

## Software and tools

- **Language:** C
- **Firmware:** STM32 HAL, configured with STM32CubeMX
- **Communication:** UART for the SPS30; I²C and SPI planned for the other components

## Current progress

- [x] Set up the STM32 development environment and build the firmware.
- [x] Connect the SPS30 to the Nucleo and confirm that its fan operates.
- [x] Debug UART transmissions and observe an initial receive response.
- [ ] Receive and validate a complete SPS30 response frame.
- [ ] Decode and report particulate matter measurements.
- [ ] Connect the e-ink display and show sensor readings.
- [ ] Format readings for an organized display.
- [ ] Integrate the SCD41 CO₂ sensor.
- [ ] Move the wiring to perfboard and mount the system in an enclosure.

## SPS30 UART prototype wiring

Current test configuration using **USART1** on the NUCLEO-F446RE:

| SPS30 signal | Nucleo connection |
| --- | --- |
| VDD | 5 V |
| GND | GND |
| RX (sensor input) | D8 / PA9 / USART1_TX |
| TX (sensor output) | D2 / PA10 / USART1_RX |
| SEL | Not connected (selects UART mode) |

The SPS30 uses Sensirion's SHDLC protocol over UART. The receive logic must identify complete frames and validate them before treating the payload as sensor data.

## Planned operation

1. Initialize the STM32 peripherals and connected devices.
2. Start and read particulate measurements from the SPS30.
3. Read CO₂ measurements from the SCD41.
4. Update the e-ink display with the latest available readings.
5. Handle communication failures without requiring a manual reset whenever possible.

## Build and development

The project is currently built and debugged using a CMake-based STM32 toolchain. Build, flash, and hardware setup instructions will be documented once the prototype configuration is stable.

## Development notes

See [the development log](docs/development-log.md) for ongoing hardware bring-up, debugging observations, and design decisions.

## Reference

- [Sensirion SPS30 product documentation](https://sensirion.com/sps30)
