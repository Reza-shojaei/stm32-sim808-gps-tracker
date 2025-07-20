# STM32F103 SIM808 GPS Tracker

A GPS tracking system using STM32F103 microcontroller and SIM808 GSM/GPS module that sends location data to a web server via HTTP requests.

## Features

- GPS location tracking using SIM808 module
- GPRS connectivity for data transmission
- Real-time location updates to web server
- Interrupt-based UART communication
- Debug output via serial console
- Robust error handling and GPS fix detection

## Hardware Requirements

- STM32 microcontroller (tested on STM32F4xx series)
- SIM808 GSM/GPS module
- GPS antenna
- GSM antenna
- SIM card with data plan
- 5V 2A power supply

## Wiring Diagram

```
STM32          SIM808
-----          ------
PA9 (TX1)  ->  RX
PA10 (RX1) <-  TX
PA2 (TX2)  ->  Debug UART (optional)
GND        ->  GND
```

## Software Dependencies

- STM32CubeIDE or compatible IDE
- STM32 HAL Library
- STM32CubeMX (for configuration)
- AT_commands DataSheet

## Configuration

### Building

- Open the project in STM32CubeIDE.
- Configure the clock to HSI and SYSCLK at 8 MHz.
- Enable USART1 (AT commands) and USART2 (debug) in Asynchronous mode.
- Enable NVIC interrupts for USART1.
- Generate code and replace src/main.c with the provided implementation.
- Build and flash the firmware to the STM32.

### UART Configuration
- USART1: SIM808 communication (9600 baud)
- USART2: Debug output (9600 baud)
- Enable  interrupt

### SIM808 Settings
- APN: Update in `main.c` according to your carrier
- Server URL: Modify the HTTP endpoint in `submitHttpRequest()`

## Installation

1. Clone this repository:
```bash
git clone https://github.com/Reza-Shojaei/stm32-sim808-gps-tracker.git
cd stm32-sim808-gps-tracker
```

2. Open the project in STM32CubeIDE

3. Configure your target STM32 device in STM32CubeMX

4. Update the following in `Core/Src/main.c`:
   - APN settings for your mobile carrier
   - Server URL and API key
   - GPIO pin assignments if different

5. Build and flash to your STM32 device

## Usage

1. Insert a SIM card with data plan into the SIM808 module
2. Connect GPS and GSM antennas
3. Power on the system
4. Monitor debug output via UART2
5. The system will:
   - Initialize SIM808 module
   - Wait for GPS fix
   - Send location data to server every 10 seconds

## API Response Format

The system sends HTTP GET requests in this format:
```
http://yourserver.com/gpsdata.php?key=YOUR_KEY&lat=LATITUDE&lng=LONGITUDE
```

## Debug Output

Monitor the serial output to see:
- System initialization status
- GPS fix status
- Raw GPS data from SIM808
- HTTP request status
- Error messages

## Troubleshooting

### GPS Not Getting Fix
- Ensure GPS antenna is connected and has clear sky view
- Wait 30+ seconds for initial GPS fix
- Check that GPS power is enabled (`AT+CGNSPWR=1`)

### GPRS Connection Issues
- Verify SIM card has data plan and is active
- Check APN settings for your carrier
- Ensure good GSM signal strength

### UART Communication Problems
- Verify baud rate settings match between STM32 and SIM808
- Check RX/TX wiring connections
- Monitor for proper AT command responses

## File Structure

```
stm32-sim808-gps-tracker/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── usart.h
│   │   └── gpio.h
│   └── Src/
│       ├── main.c
│       ├── usart.c
│       ├── gpio.c
│       └── stm32f4xx_it.c
├── Drivers/
│   └── STM32F4xx_HAL_Driver/
├── docs/
│   └── AT_commands.md
├── README.md
├── LICENSE
└── uart_gps_01.ioc
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- STMicroelectronics for STM32 HAL library
- SIMCom for SIM808 module documentation
- Community contributors and testers


**Note**: This project is for educational and hobbyist purposes. Ensure compliance with local regulations regarding GPS tracking and data transmission.
