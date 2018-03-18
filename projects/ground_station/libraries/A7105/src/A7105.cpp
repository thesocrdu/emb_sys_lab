/**
 * @file
 * @brief This file outlines the class structure
 * for the A7105 RF module.
 *
 * Module datasheet can be found here:
 * http://www.guiott.com/CleaningRobot/G-Control/DataSheets/RicevitoreTelecomando_a7105.pdf
 *
 * @author Kyle Mercer
 *
 */

#include "A7105.h"
#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>


/**
 * @defgroup A7105 Address byte flags
 * @{
 */

#define A7105_CMD_CONTROL_REG 0x00
#define A7105_CMD_STROBE      0x80

#define A7105_RW_WRITE        0x00
#define A7105_RW_READ         0x40

/** @} */

A7105::A7105(){
}

A7105::~A7105(){
}

void A7105::begin(const uint8_t csPin){

    _csPin = csPin;
    pinMode(_csPin, OUTPUT);

    /* Driving chip select pin high first seems to help stability. */
    digitalWrite(_csPin, HIGH);

    /* Set up the SPI bus parameters. */
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV2);

    /* Rising edge trigger and clock idle low. */
    SPI.setDataMode(SPI_MODE0);

    /* Set packet transmission to MSB. */
    SPI.setBitOrder(MSBFIRST);

    /* Send module reset command. */
    delay(1000);
    
    /* Drive chip select pin low until end() is called. */
    digitalWrite(_csPin, LOW);
    delayMicroseconds(10);
    write(A7105_00_MODE, 0x00);

    /**
     * @note To configure device to a 4 wire SPI configuration
     *       you would write 0x19 to the GIO1_PIN. This means
     *       that the GIO1 pin will now be the MISO pin.
     *
     *       This has been tested to exhaustion without
     *       (consistent) success. Sometimes the GIO1 pin
     *       will act like the MISO line, but a mere power
     *       cycle will cause different behavior. Perhaps it
     *       is something specific about the XL7105 module
     *       that the MD7105 doesn't suffer from.
     *       For reference, the below line is just commented out.
     *
     *       You can fake a 4 wire SPI setup on the Arduino side
     *       without changing the A7105 to 4 wire SPI. Do this by
     *       connecting a (pull-up?) resistor between the MOSI pin
     *       on the Arduino and the SDIO pin on the XL7105. Then
     *       connect a wire between the SDIO pin and the MISO pin.
     *
     *                |     1KΩ     |
     *           MOSI-|----/\/\/----|-SDIO
     *                |           | |
     *           MISO-|-----------/ |
     *                |             |
     *
     *
     *       EDIT: It seems to behave consistently in 4 wire SPI
     *       mode now. The theory is that the chip select pin
     *       must be driven high or a brief period and then low
     *       before attempting any communication with the chip.
     */
     write(A7105_0B_GIO1_PIN_I, 0x19);

}

void A7105::read(const uint8_t addr, uint8_t &data) {

    //data = SPI.transfer16((addr | (A7105_CMD_CONTROL_REG | A7105_RW_READ)) << 8);

    /* Send address in first byte. */
    SPI.transfer(addr | (A7105_CMD_CONTROL_REG | A7105_RW_READ));

    /* Do a benign transfer to receive data back. */
    data = SPI.transfer(0x00);
}

void A7105::write(const uint8_t addr, const uint8_t data) {

    //SPI.transfer16(((addr | (A7105_CMD_CONTROL_REG | A7105_RW_WRITE)) << 8) | data);

    SPI.transfer(addr | (A7105_CMD_CONTROL_REG | A7105_RW_WRITE));
    SPI.transfer(data);
}
