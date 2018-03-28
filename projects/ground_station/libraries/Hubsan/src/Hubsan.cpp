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
#include <stdlib.h>

#define A7105_IF_CALIBRATION_PASS (1 << 4)
#define A7105_VCO_CALIBRATION_PASS (1 << 3)
#define A7105_CALIBRATION_TIMEOUT_MS 500

#define WAIT_WRITE 0x80

const uint8_t Hubsan::allowed_ch[] =
       {0x14, 0x1e, 0x28, 0x32,
        0x3c, 0x46, 0x50, 0x5a,
        0x64, 0x6e, 0x78, 0x82};

Hubsan::Hubsan() {

    state = BIND_1;
    memset(packet, 0, sizeof(packet));
    sessionid = rand();
    channel = allowed_ch[rand() % sizeof(allowed_ch)];
}

Hubsan::~Hubsan() {
}

int Hubsan::init(const unsigned int cspin) {

    uint8_t if_calibration1;
    uint8_t vco_calibration0;
    uint8_t vco_calibration1;
    unsigned long startTime;

    /* Initialize the A7105 for 4 wire SPI. */
    _a7105.begin(cspin, true);

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
    _a7105.setID(0x55201041);
    _a7105.write(A7105_01_MODE_CONTROL, 0x63);
    _a7105.write(A7105_03_FIFO_I, 0x0f);
    _a7105.write(A7105_0D_CLOCK, 0x05);
    _a7105.write(A7105_0E_DATA_RATE, 0x04);
    _a7105.write(A7105_15_TX_II, 0x2b);
    _a7105.write(A7105_18_RX, 0x62);
    _a7105.write(A7105_19_RX_GAIN_I, 0x80);
    _a7105.write(A7105_1C_RX_GAIN_IV, 0x0A);
    _a7105.write(A7105_1F_CODE_I, 0x07);
    _a7105.write(A7105_20_CODE_II, 0x17);
    _a7105.write(A7105_29_RX_DEM_TEST_I, 0x47);

    _a7105.sendStrobe(A7105_STANDBY);

    /* IF Filter Bank Calibration */
    _a7105.write(A7105_02_CALIB_CONT, 1);
    _a7105.read(A7105_02_CALIB_CONT);

    startTime = millis();
    while(millis() - startTime < A7105_CALIBRATION_TIMEOUT_MS) {
        if(! _a7105.read(A7105_02_CALIB_CONT)) {
            break;
        }
    }
    if (millis() - startTime >= A7105_CALIBRATION_TIMEOUT_MS) {
        return 1;
    }

    /* Check auto calibration passed */
    if_calibration1 = _a7105.read(A7105_22_IF_CALIB_I);
    _a7105.read(A7105_24_VCO_CUR_CALIB);
    if(if_calibration1 & A7105_IF_CALIBRATION_PASS) {
        //Calibration failed...what do we do?
        return 1;
    }

    //VCO Current Calibration
    //_a7105.write(0x24, 0x13); //Recomended calibration from A7105 Datasheet

    //VCO Bank Calibration
    //_a7105.write(0x26, 0x3b); //Recomended limits from A7105 Datasheet

    //VCO Bank Calibrate channel 0?
    //Set Channel
    _a7105.write(A7105_0F_PLL_I, 0);
    //VCO Calibration
    _a7105.write(A7105_02_CALIB_CONT, 2);
    startTime = millis();
    while(millis() - startTime < A7105_CALIBRATION_TIMEOUT_MS) {
        if(! _a7105.read(A7105_02_CALIB_CONT))
            break;
    }
    if (millis() - startTime >= A7105_CALIBRATION_TIMEOUT_MS) {
        return 1;
    }
    vco_calibration0 = _a7105.read(A7105_25_VCO_SB_CAL_I);
    if (vco_calibration0 & A7105_VCO_CALIBRATION_PASS) {
        //Calibration failed...what do we do?
        return 1;
    }

    //Calibrate channel 0xa0?
    //Set Channel
    _a7105.write(A7105_0F_PLL_I, 0xa0);
    //VCO Calibration
    _a7105.write(A7105_02_CALIB_CONT, 2);
    startTime = millis();
    while(millis() - startTime < A7105_CALIBRATION_TIMEOUT_MS) {
        if(! _a7105.read(A7105_02_CALIB_CONT))
            break;
    }
    if (millis() - startTime >= A7105_CALIBRATION_TIMEOUT_MS) {
        return 1;
    }
    vco_calibration1 = _a7105.read(A7105_25_VCO_SB_CAL_I);
    if (vco_calibration1 & A7105_VCO_CALIBRATION_PASS) {
        //Calibration failed...what do we do?
    }

    //Reset VCO Band calibration
    //_a7105.write(0x25, 0x08);

    _a7105.setPower(TXPOWER_150mW);

    _a7105.sendStrobe(A7105_STANDBY);

    return 0;
}

void Hubsan::update_crc() {

    int sum = 0;
    for(int i = 0; i < 15; i++)
        sum += packet[i];
    packet[15] = (256 - (sum % 256)) & 0xff;
}

void Hubsan::hubsan_build_bind_packet(u8 state) {
    packet[0] = state;
    packet[1] = channel;
    packet[2] = (sessionid >> 24) & 0xff;
    packet[3] = (sessionid >> 16) & 0xff;
    packet[4] = (sessionid >>  8) & 0xff;
    packet[5] = (sessionid >>  0) & 0xff;
    packet[6] = 0x08;
    packet[7] = 0xe4; //???
    packet[8] = 0xea;
    packet[9] = 0x9e;
    packet[10] = 0x50;
    packet[11] = (txid >> 24) & 0xff;
    packet[12] = (txid >> 16) & 0xff;
    packet[13] = (txid >>  8) & 0xff;
    packet[14] = (txid >>  0) & 0xff;
    update_crc();
}


int16_t Hubsan::get_channel(uint8_t ch, int32_t scale, int32_t center, int32_t range) {
    static int a=0;
    if (a++<2550) return 0;

    //  return 254;
    return 128;
    //int32_t value = (int32_t)Channels[ch] * scale / CHAN_MAX_VALUE + center;
    //if (value < center - range)
    //    value = center - range;
    //if (value >= center + range)
    //    value = center + range -1;
    //return value;
}


volatile uint8_t throttle=0, rudder=0, aileron = 0, elevator = 0;

void Hubsan::hubsan_build_packet() {

    memset(packet, 0, 16);
    //20 00 00 00 80 00 7d 00 84 02 64 db 04 26 79 7b
    packet[0] = 0x20;
    packet[2] = throttle;//get_channel(2, 0x80, 0x80, 0x80);
    packet[4] = 0xff - rudder; // get_channel(3, 0x80, 0x80, 0x80); //Rudder is reversed
    packet[6] = 0xff - elevator; // get_channel(1, 0x80, 0x80, 0x80); //Elevator is reversed
    packet[8] = aileron; // get_channel(0, 0x80, 0x80, 0x80);
    //packet[4] = 0x80; // get_channel(3, 0x80, 0x80, 0x80); //Rudder is reversed
    //packet[6] = 0x80; // get_channel(1, 0x80, 0x80, 0x80); //Elevator is reversed
    //packet[8] = 0x80; // get_channel(0, 0x80, 0x80, 0x80);
    packet[9] = 0x0e;
    packet[10] = 0x19;
    //packet[11] = (txid >> 24) & 0xff;
    //packet[12] = (txid >> 16) & 0xff;
    //packet[13] = (txid >>  8) & 0xff;
    //packet[14] = (txid >>  0) & 0xff;
    packet[11] = 0x00;
    packet[12] = 0x00;
    packet[13] = 0x00;
    packet[14] = 0x00;
    update_crc();
}

uint16_t Hubsan::hubsan_cb() {
    int i;
    switch(state) {
    case BIND_1:
    case BIND_3:
    case BIND_5:
    case BIND_7:
        hubsan_build_bind_packet(state == BIND_7 ? 9 : (state == BIND_5 ? 1 : state + 1 - BIND_1));
        _a7105.sendStrobe(A7105_STANDBY);
        _a7105.writeData(packet, 16, channel);
        state |= WAIT_WRITE;
        return 3000;
    case BIND_1 | WAIT_WRITE:
    case BIND_3 | WAIT_WRITE:
    case BIND_5 | WAIT_WRITE:
    case BIND_7 | WAIT_WRITE:
        //wait for completion
        for(i = 0; i< 20; i++) {
          if(! (_a7105.read(A7105_00_MODE) & 0x01))
            break;
        }
        if (i == 20)
            Serial.println("Failed to complete write\n");
       // else 
       //     Serial.println("Completed write\n");
        _a7105.sendStrobe(A7105_RX);
        state &= ~WAIT_WRITE;
        state++;
        return 4500; //7.5msec elapsed since last write
    case BIND_2:
    case BIND_4:
    case BIND_6:
        if(_a7105.read(A7105_00_MODE) & 0x01) {
            state = BIND_1; //Serial.println("Restart");
            return 4500; //No signal, restart binding procedure.  12msec elapsed since last write
        } 

       _a7105.readData(packet, 16);
        state++;
        if (state == BIND_5)
            _a7105.setID((packet[2] << 24) | (packet[3] << 16) | (packet[4] << 8) | packet[5]);
        
        return 500;  //8msec elapsed time since last write;
    case BIND_8:
        if(_a7105.read(A7105_00_MODE) & 0x01) {
            state = BIND_7;
            return 15000; //22.5msec elapsed since last write
        }
        _a7105.readData(packet, 16);
        if(packet[1] == 9) {
            state = DATA_1;
            _a7105.write(A7105_1F_CODE_I, 0x0F);
            //PROTOCOL_SetBindState(0);
            return 28000; //35.5msec elapsed since last write
        } else {
            state = BIND_7;
            return 15000; //22.5 msec elapsed since last write
        }
    case DATA_1:
        //Keep transmit power in sync
        _a7105.setPower(TXPOWER_150mW);
    case DATA_2:
    case DATA_3:
    case DATA_4:
    case DATA_5:
        hubsan_build_packet();
        _a7105.writeData(packet, 16, state == DATA_5 ? channel + 0x23 : channel);
        if (state == DATA_5)
            state = DATA_1;
        else
            state++;
        return 10000;
    }
    return 0;
}
