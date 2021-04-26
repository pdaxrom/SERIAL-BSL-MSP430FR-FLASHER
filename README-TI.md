## Example Summary

Command line utility to showcase the UART BSL of MSP430 FRAM devices with BeagleBone
black as host.

This example uses the GPIO and UART driver to program an MSP430 FRAM based device.

## Peripherals Exercised

* `UART4` - Used to send BSL commands to the target MSP430 device
* `RESET_PIN` - Used to control the reset pin of the target MSP430 device
* `TEST_PIN` - Used to control the test pin of the target MSP430 device

## Example Usage

To configure the BeagleBone pins for GPIO usage, you must run the following commands:

* config-pin P9.14 gpio
* config-pin P9.16 gpio

To configure the BeagleBone pins for uart usage, you must run the following commands:

* config-pin P9.11 uart
* config-pin P9.13 uart

Compile the code:

* gcc -I ./ main.c uart_if.c pinmux.c gpio_if.c utils.c bsl.c -o bsl_command_line

Run the example program

* Reset pin is GPIO 50, Test pin is GPIO 51, UART module #4, TI TXT Hex file path
* ./bsl_command_line 50 51 4 image/FR2311_blinky.txt


## Connections

* Connect the RESET_PIN (P9_14: GPIO1_18) to the reset pin of the target device (MSP430FR2311 rst pin used for testing)
* Connect the TEST_PIN (P9_16: GPIO1_19) to the test pin of the target device (MSP430FR2311 test pin used for testing)
* Connect the UART4_RXD (P9_11) to the BSL Transmit pin of the target device (MSP430FR2311 P1.7 pin used for testing)
* Connect the UART4_TXD (P9_13) to the BSL Receive pin of the target device (MSP430FR2311 P1.6 pin used for testing)

* The BeagleBone Host begins to program the target MSP430 with the blinky image. The target MSP430 image is located in image/msp430fr_image.h

BeagleBone Image:

* The BeagleBone image used in this project can be found at,
https://debian.beagleboard.org/images/bone-debian-8.6-iot-armhf-2016-12-09-4gb.img.xz
