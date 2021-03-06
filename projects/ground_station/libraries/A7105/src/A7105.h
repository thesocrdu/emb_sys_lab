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

#ifndef A7105_H
#define A7105_H

#pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wextra"

#include <Arduino.h>
#include <stdint.h>

/**
 * A7105 register definitions which are
 * accessible over the SPI bus.
 */
enum {
    A7105_00_MODE           = 0x00,
    A7105_01_MODE_CONTROL   = 0x01,
    A7105_02_CALIB_CONT     = 0x02,
    A7105_03_FIFO_I         = 0x03,
    A7105_04_FIFO_II        = 0x04,
    A7105_05_FIFO_DATA      = 0x05,
    A7105_06_ID_DATA        = 0x06,
    A7105_07_RC_OSC_I       = 0x07,
    A7105_08_RC_OSC_II      = 0x08,
    A7105_09_RC_OSC_III     = 0x09,
    A7105_0A_CKO_PIN        = 0x0A,
    A7105_0B_GIO1_PIN_I     = 0x0B,
    A7105_0C_GIO2_PIN_II    = 0x0C,
    A7105_0D_CLOCK          = 0x0D,
    A7105_0E_DATA_RATE      = 0x0E,
    A7105_0F_PLL_I          = 0x0F,
    A7105_10_PLL_II         = 0x10,
    A7105_11_PLL_III        = 0x11,
    A7105_12_PLL_IV         = 0x12,
    A7105_13_PLL_V          = 0x13,
    A7105_14_TX_I           = 0x14,
    A7105_15_TX_II          = 0x15,
    A7105_16_DELAY_I        = 0x16,
    A7105_17_DELAY_II       = 0x17,
    A7105_18_RX             = 0x18,
    A7105_19_RX_GAIN_I      = 0x19,
    A7105_1A_RX_GAIN_II     = 0x1A,
    A7105_1B_RX_GAIN_III    = 0x1B,
    A7105_1C_RX_GAIN_IV     = 0x1C,
    A7105_1D_RSSI_THRESH    = 0x1D,
    A7105_1E_ADC            = 0x1E,
    A7105_1F_CODE_I         = 0x1F,
    A7105_20_CODE_II        = 0x20,
    A7105_21_CODE_III       = 0x21,
    A7105_22_IF_CALIB_I     = 0x22,
    A7105_23_IF_CALIB_II    = 0x23,
    A7105_24_VCO_CUR_CALIB  = 0x24,
    A7105_25_VCO_SB_CAL_I   = 0x25,
    A7105_26_VCO_SB_CAL_II  = 0x26,
    A7105_27_BATT_DETECT    = 0x27,
    A7105_28_TX_TEST        = 0x28,
    A7105_29_RX_DEM_TEST_I  = 0x29,
    A7105_2A_RX_DEM_TEST_II = 0x2A,
    A7105_2B_CHRG_PUMP_CUR  = 0x2B,
    A7105_2C_XTAL_TEST      = 0x2C,
    A7105_2D_PLL_TEST       = 0x2D,
    A7105_2E_VCO_TEST_I     = 0x2E,
    A7105_2F_VCO_TEST_II    = 0x2F,
    A7105_30_IFAT           = 0x30,
    A7105_31_RSCALE         = 0x31,
    A7105_32_FILTER_TEST    = 0x32,
};

/**
 * All possible module states of the A7105 module.
 */
enum A7105_State {
    A7105_SLEEP     = 0x80,
    A7105_IDLE      = 0x90,
    A7105_STANDBY   = 0xA0,
    A7105_PLL       = 0xB0,
    A7105_RX        = 0xC0,
    A7105_TX        = 0xD0,
    A7105_RST_WRPTR = 0xE0,
    A7105_RST_RDPTR = 0xF0,
};

/**
 * Output power values available for the A7105
 */
enum TxPower {
    TXPOWER_100uW = 0,
    TXPOWER_300uW = 1,
    TXPOWER_1mW   = 2,
    TXPOWER_3mW   = 3,
    TXPOWER_10mW  = 4,
    TXPOWER_30mW  = 5,
    TXPOWER_100mW = 6,
    TXPOWER_150mW = 7,
    TXPOWER_LAST  = 8,
};

/**
 * This class provides an interface for controlling the
 * A7105 RF module. The module talks over the SPI.
 */
class A7105 {
    public:

        /** Constructor. */
        A7105();

        /** Destructor. */
        ~A7105();

        /**
         * Initiate the A7105 module communication.
         * @param[in] rxEnPin The RXEN pin number for the module.
         * @param[in] txEnPin The TXEN pin number for the module.
         * @param[in] csPin The Chip Select pin number for the module.
         * @param[in] useFourWireSpi Whether to configure the chip to
         *            use pin GIO1 as the MISO pin for SPI.
         */
        void begin(const uint8_t rxEnPin, const uint8_t txEnPin,
                const uint8_t csPin, const bool useFourWireSpi = true);

        /**
         * Reads a single register over SPI.
         * @param[in] addr The register offset to read.
         * @return Data stored in the requested register.
         */
        uint8_t read(const uint8_t addr);

        /**
         * Write to a single register over SPI
         * @param[in] addr The register offset to write to.
         * @param[in] data The byte to write out over SPI.
         */
        void write(const uint8_t addr, const uint8_t data);

        /**
         * Set the RF ID for this packet session.
         * @param id The 32-bit ID
         */
        void setID(const uint32_t id);

        /**
         * Sends the provided strobe to device
         * @param strobe The @sa A7105_State to be sent.
         */
        void sendStrobe(const A7105_State strobe);

        /**
         * Sets the output power of the A7105.
         * @param[in] The desired @sa TxPower
         */
        void setPower(TxPower power);

        /**
         * Writes the provided data buffer to the A7105.
         * @param[in] dpbuffer The buffer of data to send.
         * @param[in] len The length of the buffer in bytes.
         */
        void writeData(const uint8_t* const dpbuffer, const uint8_t len);

        /**
         * Reads data from the A7105 into the user provided buffer.
         * @param[in/out] dpbuffer The prellocated buffer to place the data.
         * @param[in] len The length of the buffer in bytes.
         */
        void readData(uint8_t* const dpbuffer, const uint8_t len);

    private:

        /** The Chip Select pin number for the module. */
        uint8_t _csPin;

        /**  The RXEN pin number for the module. */
        uint8_t  _rxEnPin;

        /**  The TXEN pin number for the module. */
        uint8_t  _txEnPin;
};

#endif /* A7105_H */
