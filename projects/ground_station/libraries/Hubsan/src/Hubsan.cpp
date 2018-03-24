/**
 * @file
 * @brief This file implements the functions for
 * the @sa Hubsan class.
 *
 * @author Kyle Mercer
 *
 */

#include <A7105.h>
#include <Arduino.h>
#include <Hubsan.h>
#include <stdint.h>

#define A7105_IF_CALIBRATION_PASS (1 << 4)
#define A7105_VCO_CALIBRATION_PASS (1 << 3)
#define A7105_CALIBRATION_TIMEOUT_MS 500

Hubsan::Hubsan() {
}

Hubsan::~Hubsan() {
}

void Hubsan::init(int cspin) {

    uint8_t if_calibration1;
    uint8_t vco_calibration0;
    uint8_t vco_calibration1;
    unsigned long startTime;

    /**
     * @note The following init code was referenced from both
     *       the Deviation10 firmware code located here
     *
     *       https://bitbucket.org/PhracturedBlue/
     *
     *       This code was ultimately derived from the reverse
     *       engineering efforts of Jim Hung available here:
     *
     *       http://www.jimhung.co.uk/wp-content/uploads/2014/11/HubsanX4_ProtocolSpec_v1.txt
     *
     */
    a7105.setID(0x55201041);
    a7105.write(A7105_01_MODE_CONTROL, 0x63);
    a7105.write(A7105_03_FIFO_I, 0x0f);
    a7105.write(A7105_0D_CLOCK, 0x05);
    a7105.write(A7105_0E_DATA_RATE, 0x04);
    a7105.write(A7105_15_TX_II, 0x2b);
    a7105.write(A7105_18_RX, 0x62);
    a7105.write(A7105_19_RX_GAIN_I, 0x80);
    a7105.write(A7105_1C_RX_GAIN_IV, 0x0A);
    a7105.write(A7105_1F_CODE_I, 0x07);
    a7105.write(A7105_20_CODE_II, 0x17);
    a7105.write(A7105_29_RX_DEM_TEST_I, 0x47);

    a7105.sendStrobe(A7105_STANDBY);

    /* IF Filter Bank Calibration */
    a7105.write(A7105_02_CALIB_CONT, 1);
    a7105.read(A7105_02_CALIB_CONT);

    startTime = millis();
    while(millis() - startTime < A7105_CALIBRATION_TIMEOUT_MS) {
        if(! a7105.read(A7105_02_CALIB_CONT)) {
            break;
        }
    }
    if (millis() - startTime >= A7105_CALIBRATION_TIMEOUT_MS) {
        return 1;
    }

    /* Check auto calibration passed */
    if_calibration1 = a7105.read(A7105_22_IF_CALIB_I);
    a7105.read(A7105_24_VCO_CURCAL);
    if(if_calibration1 & A7105_IF_CALIBRATION_PASS) {
        //Calibration failed...what do we do?
        return 1;
    }

    //VCO Current Calibration
    //a7105.write(0x24, 0x13); //Recomended calibration from A7105 Datasheet

    //VCO Bank Calibration
    //a7105.write(0x26, 0x3b); //Recomended limits from A7105 Datasheet

    //VCO Bank Calibrate channel 0?
    //Set Channel
    a7105.write(A7105_0F_CHANNEL, 0);
    //VCO Calibration
    a7105.write(A7105_02_CALIB_CONT, 2);
    startTime = millis();
    while(millis() - startTime < A7105_CALIBRATION_TIMEOUT_MS) {
        if(! a7105.read(A7105_02_CALIB_CONT))
            break;
    }
    if (millis() - startTime >= A7105_CALIBRATION_TIMEOUT_MS)
        return 1;
    vco_calibration0 = a7105.read(A7105_25_VCO_SBCAL_I);
    if (vco_calibration0 & A7105_VC0_CALIBRATION_PASS) {
        //Calibration failed...what do we do?
        return 1;
    }

    //Calibrate channel 0xa0?
    //Set Channel
    a7105.write(A7105_0F_CHANNEL, 0xa0);
    //VCO Calibration
    a7105.write(A7105_02_CALC, 2);
    startTime = millis();
    while(millis() - startTime < A7105_CALIBRATION_TIMEOUT_MS) {
        if(! a7105.read(A7105_02_CALC))
            break;
    }
    if (millis() - startTime >= A7105_CALIBRATION_TIMEOUT_MS)
        return 1;
    vco_calibration1 = a7105.read(A7105_25_VCO_SBCAL_I);
    if (vco_calibration1 & A7105_VC0_CALIBRATION_PASS) {
        //Calibration failed...what do we do?
    }

    //Reset VCO Band calibration
    //a7105.write(0x25, 0x08);

    a7105_SetPower(TXPOWER_150mW);

    a7105_Strobe(A7105_STANDBY);

    return 0;
}
