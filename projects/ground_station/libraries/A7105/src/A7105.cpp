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

/**
 * @defgroup A7105 chip select and RX/TX mode macros
 * @{
 */

#define CS_LOW() digitalWrite(_csPin, LOW)
#define CS_HIGH() digitalWrite(_csPin, HIGH)

#define RX_EN()  digitalWrite(_rxEnPin, HIGH);
#define RX_DIS() digitalWrite(_rxEnPin, LOW);
#define TX_EN()  digitalWrite(_txEnPin, HIGH);
#define TX_DIS() digitalWrite(_txEnPin, LOW);

/** @} */

A7105::A7105() {
}

A7105::~A7105() {
}

void A7105::begin(const uint8_t rxEnPin, const uint8_t txEnPin,
        const uint8_t csPin, const bool useFourWireSpi) {

    _csPin = csPin;
    _rxEnPin = rxEnPin;
    _txEnPin = txEnPin;
    pinMode(_csPin, OUTPUT);
    pinMode(_rxEnPin, OUTPUT);
    pinMode(_txEnPin, OUTPUT);

    /* Default to RX mode first. TX is only set during writeData().  */
    RX_EN();
    TX_DIS();

    /* Driving chip select pin high first seems to help stability. */
    CS_HIGH();

    /* Set up the SPI bus parameters. */
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV2);

    /* Rising edge trigger and clock idle low. */
    SPI.setDataMode(SPI_MODE0);

    /* Set packet transmission to MSB. */
    SPI.setBitOrder(MSBFIRST);

    /* Send module reset command. */
    delay(10);
    
    /* Drive chip select pin low until is called. */
    CS_LOW();
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
     *       must be driven high for a brief period and then low
     *       before attempting any communication with the chip.
     */
    if (useFourWireSpi) {
        write(A7105_0B_GIO1_PIN_I, 0x19);
    }

    CS_HIGH();
}

uint8_t A7105::read(const uint8_t addr) {

    uint8_t data;

    CS_LOW();

    /* Send address in first byte. */
    SPI.transfer(addr | (A7105_CMD_CONTROL_REG | A7105_RW_READ));

    /* Do a benign transfer to receive data back. */
    data = SPI.transfer(0x00);

    CS_HIGH();

    return data;
}

void A7105::write(const uint8_t addr, const uint8_t data) {

    CS_LOW();

    SPI.transfer(addr | (A7105_CMD_CONTROL_REG | A7105_RW_WRITE));
    SPI.transfer(data);

    CS_HIGH();
}

void A7105::setID(const uint32_t id) {

    CS_LOW();
    SPI.transfer(A7105_06_ID_DATA);
    SPI.transfer((id >> 24) & 0xFF);
    SPI.transfer((id >> 16) & 0xFF);
    SPI.transfer((id >> 8) & 0xFF);
    SPI.transfer((id >> 0) & 0xFF);
    CS_HIGH();
}

void A7105::sendStrobe(const A7105_State strobe) {
    CS_LOW();
    SPI.transfer(strobe);
    CS_HIGH();
}

void A7105::setPower(TxPower power) {
    /*
    Power amp is ~+16dBm so:
    TXPOWER_100uW  = -23dBm == PAC=0 TBG=0
    TXPOWER_300uW  = -20dBm == PAC=0 TBG=1
    TXPOWER_1mW    = -16dBm == PAC=0 TBG=2
    TXPOWER_3mW    = -11dBm == PAC=0 TBG=4
    TXPOWER_10mW   = -6dBm  == PAC=1 TBG=5
    TXPOWER_30mW   = 0dBm   == PAC=2 TBG=7
    TXPOWER_100mW  = 1dBm   == PAC=3 TBG=7
    TXPOWER_150mW  = 1dBm   == PAC=3 TBG=7
    */
    uint8_t pac, tbg;
    switch(power) {
        case TXPOWER_100uW: pac = 0; tbg = 0; break;
        case TXPOWER_300uW: pac = 0; tbg = 1; break;
        case TXPOWER_1mW  : pac = 0; tbg = 2; break;
        case TXPOWER_3mW  : pac = 0; tbg = 4; break;
        case TXPOWER_10mW : pac = 1; tbg = 5; break;
        case TXPOWER_30mW : pac = 2; tbg = 7; break;
        case TXPOWER_100mW: pac = 3; tbg = 7; break;
        case TXPOWER_150mW: pac = 3; tbg = 7; break;
        default: pac = 0; tbg = 0; break;
    };
    write(A7105_28_TX_TEST, (pac << 3) | tbg);
}

void A7105::writeData(const uint8_t* const dpbuffer, const uint8_t len) {

    RX_DIS();
    TX_EN();

    CS_LOW();
    SPI.transfer(A7105_RST_WRPTR);    //reset write FIFO PTR
    CS_HIGH();

    CS_LOW();
    SPI.transfer(A7105_05_FIFO_DATA); // FIFO DATA register - about to send data to put into FIFO
    for (unsigned int i = 0; i < len; i++) {
        SPI.transfer(dpbuffer[i]); // send some data
    }
    CS_HIGH();

    CS_LOW();
    SPI.transfer(A7105_TX); // strobe command to actually transmit the daat
    CS_HIGH();

    while (true){ // Check to see if the transmission has completed.
        uint8_t modeData = read(A7105_00_MODE);
        if (bitRead(modeData, 0) == 0){
            break;
        }
    }

    TX_DIS();
    RX_EN();
}

void A7105::readData(uint8_t* const dpbuffer, const uint8_t len) {
    sendStrobe(A7105_RST_RDPTR);
    for(unsigned int i = 0; i < len; i++) {
        dpbuffer[i] = read(A7105_05_FIFO_DATA);
    }
}
