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

const uint8_t Hubsan::allowed_ch[HUBSAN_CHAN_ARR_LEN] =
       {0x14, 0x1e, 0x28, 0x32,
        0x3c, 0x46, 0x50, 0x5a,
        0x64, 0x6e, 0x78, 0x82};

Hubsan::Hubsan() {

    state = BIND_1;
    memset(packet, 0, sizeof(packet));
    _channel = allowed_ch[0];
    //_channel = allowed_ch[rand() % sizeof(allowed_ch)];
}

Hubsan::~Hubsan() {
}

int Hubsan::init(const unsigned int cspin) {

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

    /* Initialize the A7105 for 4 wire SPI. */
    _a7105.begin(cspin, true);

    _a7105.setID(0x55201041);

    // Set Mode Control Register (x01) Auto RSSI measurement, Auto IF Offset, FIFO mode enabled.
    _a7105.write(A7105_01_MODE_CONTROL, 0x63);
    // Set Calibration Control Reg (x02) - Reset.
    _a7105.write(A7105_02_CALIB_CONT, 0x00);
    // Set FIFO Register 1 (x03) - Set FIFO length to 16 bytes
    _a7105.write(A7105_03_FIFO_I, 0x0F);
    // Set FIFO Register 2 (x04) - 16 Byte TX/RX.
    _a7105.write(A7105_04_FIFO_II, 0xC0);
    // Set RC OSC Reg 1 (x07) - Reset.
    _a7105.write(A7105_07_RC_OSC_I, 0x00);
    // Set RC OSC Reg 2 (x08) - Reset.
    _a7105.write(A7105_08_RC_OSC_II, 0x00);
    // Set RC OSC Reg 3 (x09) - RC-oscillator Enable.
    _a7105.write(A7105_09_RC_OSC_III, 0x04);
    // Set CKO Pin Control Register (x0A) - Disable CLK out, TX clock, RX Recovery CLK, Non-Inverted CLK, Hi-Z CLK Out, Non-Inverted SPI Pin CLK.
    _a7105.write(A7105_0A_CKO_PIN, 0x00);
    // OMITTED: Set GIO1 Pin Control Register (x0B) - Reset.
    // Set GIO2 Pin Control Register (x0C) - GIO2 Pin Enable.
    _a7105.write(A7105_0C_GIO2_PIN_II, 0x01);
    // Set Clock Register (x0D) - Use Crystal Oscillator, CLK divider = /2.
    _a7105.write(A7105_0D_CLOCK, 0x05);
    // Set Data Rate Register (x0E) - Set data rate to 25kbps.
    _a7105.write(A7105_0E_DATA_RATE, 0x04);
    // Set PLL Register 1 (x0F) - Set Channel Offset to 80.
    _a7105.write(A7105_0F_PLL_I, 0x50);
    // Set PLL Register 2 (x10) - Reset.
    _a7105.write(A7105_10_PLL_II, 0x9E);
    // Set PLL Register 3 (x11) - Reset.
    _a7105.write(A7105_11_PLL_III, 0x4B);
    // Set PLL Register 4 (x12) - Reset.
    _a7105.write(A7105_12_PLL_IV, 0x00);
    // Set PLL Register 5 (x13) - Autofrequency Compensation.
    _a7105.write(A7105_13_PLL_V, 0x00);
    // Set TX Register 1 (x14) - Reset.
    _a7105.write(A7105_14_TX_I, 0x16);
    // Set TX Register 2 (x15) - Frequency Deviation: 186KHz
    _a7105.write(A7105_15_TX_II, 0x2B);
    // Set Delay Register 1 (x16) - Reset.
    _a7105.write(A7105_16_DELAY_I, 0x12);
    // Set Delay Register 2 (x17) - 200us settling delay, 10us AGC delay settling, 10us RSSI measurement delay.
    _a7105.write(A7105_17_DELAY_II, 0x00);
    // Set RX Register (x18) - BPF bandwidth = 500 KHz.
    _a7105.write(A7105_18_RX, 0x62);
    // Set RX Gain Register 1 (x19) - Manual VGA, Mixer Gain: 24dB, LNA gain: 24dB.
    _a7105.write(A7105_19_RX_GAIN_I, 0x80);
    // Set RX Gain Register 2 (x1A) - Internal Use?
    _a7105.write(A7105_1A_RX_GAIN_II, 0x80);
    // Set RX Gain Register 3 (x1B) - Internal Use?
    _a7105.write(A7105_1B_RX_GAIN_III, 0x00);
    // Set RX Gain Register 4 (x1C) - Reserved.
    _a7105.write(A7105_1C_RX_GAIN_IV, 0x0A);
    // Set RSSI Threshold Register (x1D) to x32.
    _a7105.write(A7105_1D_RSSI_THRESH, 0x32);
    // Set ADC Control Register (x1E) - RSSI Margin: 20, RSSI Measurement continue, FSARS: 4 MHZ, XADS = Convert RSS, RSSI measurement selected, RSSI continuous mode.
    _a7105.write(A7105_1E_ADC, 0xC3);
    // Set Code Register 1 (x1F) - Reset.
    _a7105.write(A7105_1F_CODE_I, 0x07);
    // Set Code Register 2 (x20) - Demodulator avg mode, 2 bit ID error code tolerance, 16bit  preamble detect.
    _a7105.write(A7105_20_CODE_II, 0x17);
    // Set Code Register 3 (x21) - Encryption Key (XOR…) - All zeroes (Data Whitening not enabled in register x1F.
    _a7105.write(A7105_21_CODE_III, 0x00);
    // Set IF Calibration Register (x22) - Autocalibrate.
    _a7105.write(A7105_22_IF_CALIB_I, 0x00);
    // Set VCO Current Calibration Reg (x24) - Autocalibrate.
    _a7105.write(A7105_24_VCO_CUR_CALIB, 0x00);
    // Set VCO Single band Calibration Reg (x25) - Autocalibrate.
    _a7105.write(A7105_25_VCO_SB_CAL_I, 0x00);
    // Set VCO Single band Calibration Reg 2 (x26) - Upper threshold: -1.3V, Lower threshold: 0.4V
    _a7105.write(A7105_26_VCO_SB_CAL_II, 0x3B);
    // Set Battery Detect Register (x27) - 2V battery detect threshold.
    _a7105.write(A7105_27_BATT_DETECT, 0x00);
    // Set TX Test Register (x28) - Reset.
    _a7105.write(A7105_28_TX_TEST, 0x17);
    // Set RX DEM test Reg (x29) - Internal Use?
    _a7105.write(A7105_29_RX_DEM_TEST_I, 0x47);
    // Set RX DEM test Reg 2 (x2A) - Reset.
    _a7105.write(A7105_2A_RX_DEM_TEST_II, 0x80);
    // Set Charge Pump Current Reg (x2B) - 2.0mA.
    _a7105.write(A7105_2B_CHRG_PUMP_CUR, 0x03);
    // Set Crystal Test Reg (x2C) - Internal Use?
    _a7105.write(A7105_2C_XTAL_TEST, 0x01);
    // Set PLL test Register (x2D) - Internal Use?
    _a7105.write(A7105_2D_PLL_TEST, 0x45);
    // Set VCO test Reg (x2E) - Internal Use?
    _a7105.write(A7105_2E_VCO_TEST_I, 0x18);
    // Set VCO test reg 2 (x2F) - Internal Use?
    _a7105.write(A7105_2F_VCO_TEST_II, 0x00);
    // Set IFAT reg (x30) - Internal Use?
    _a7105.write(A7105_30_IFAT, 0x01);
    // Set RSCALE reg (x31) - Reset.
    _a7105.write(A7105_31_RSCALE, 0x0F);

    // CALIBRATION TESTS
    uint8_t test_result; // var to hold the test results for each calibration test in turn.
    int timeout; // var to hold timeout watchdog counter.

    // IF Filter Bank Calibration START.
#ifdef GS_DEBUG
    Serial.println("Performing IF Filter Bank Calibration Test.");
#endif
    timeout = 0;
    _a7105.sendStrobe(A7105_PLL); // Strobe - STANDBY.
    _a7105.write(A7105_02_CALIB_CONT, 0x01); // Set Calibration Control Reg (x02) - IF Filter Bank calibration enable.
    while (true) {
        test_result = _a7105.read(A7105_02_CALIB_CONT);
        if (test_result == 0x00){
            break;
        }
        if (timeout > 100){ // Allow 100 iterations before signalling a failure.
#ifdef GS_DEBUG
            Serial.println("ERROR: IF Filter Bank Calibration Test FAILED - (timeout).");
#endif
        }
        timeout++;
        delayMicroseconds(20);
    }
    test_result = _a7105.read(A7105_22_IF_CALIB_I);
    if (bitRead(test_result,4)){
#ifdef GS_DEBUG
        Serial.println("ERROR: IF Filter Bank Calibration Test FAILED - FBCF Flag: " + String(test_result));
#endif
    } else {
#ifdef GS_DEBUG
        Serial.println(" - Passed.");
#endif
    }
    _a7105.write(A7105_22_IF_CALIB_I, 0x13); //Set IF Calibration Register - Configure relative control calibration.
    _a7105.write(A7105_23_IF_CALIB_II, 0x3B); // Set IF Calibration Register 2 - as above.

    // VCO Bank Calibration - TEST 1: START
#ifdef GS_DEBUG
    Serial.println("Performing VCO Bank Calibration - Test 1");
#endif
    timeout = 0;
    _a7105.write(A7105_0F_PLL_I, 0x00); // Set PLL Register 1 - Reset.
    _a7105.sendStrobe(A7105_PLL); // Strobe - PLL Mode.
    _a7105.write(A7105_02_CALIB_CONT, 0x02); // Set Calibration Control Reg - VCO Bank Calibration enable.
    while (true) {
        test_result = _a7105.read(A7105_02_CALIB_CONT);
        if (test_result == 0x00){
            break;
        }
        if (timeout > 10){ // Allow 10 iterations before signalling a failure.
#ifdef GS_DEBUG
            Serial.println("ERROR: VCO Bank Calibration Test FAILED - (timeout).");
#endif
        }
        timeout++;
        delayMicroseconds(20);
    }
    test_result = _a7105.read(A7105_25_VCO_SB_CAL_I);
    if (bitRead(test_result,3)){
#ifdef GS_DEBUG
        Serial.println("ERROR: VCO Bank Calibration Test FAILED - VBCF Flag: " + String(test_result));
#endif
    } else {
#ifdef GS_DEBUG
        Serial.println(" - Passed.");
#endif
    }
    _a7105.write(A7105_0F_PLL_I, 0x78); // Set PLL Register 1 - Select Channel Offset.

    // VCO Bank Calibration - TEST 2: START
#ifdef GS_DEBUG
    Serial.println("Performing VCO Bank Calibration - Test 2");
#endif
    timeout = 0;
    _a7105.sendStrobe(A7105_PLL); // Strobe - PLL Mode.
    _a7105.write(A7105_02_CALIB_CONT, 0x02); // Set Calibration Control Reg - VCO Bank Calibration enable.
    while (true) {
        test_result = _a7105.read(A7105_02_CALIB_CONT);
        if (test_result == 0x00){
            break;
        }
        if (timeout > 10){ // Allow 10 iterations before signalling a failure.
#ifdef GS_DEBUG
            Serial.println("ERROR: VCO Bank Calibration Test FAILED - (timeout).");
#endif
        }
        timeout++;
        delayMicroseconds(20);
    }
    test_result = _a7105.read(A7105_25_VCO_SB_CAL_I);
    if (bitRead(test_result,3)){
#ifdef GS_DEBUG
        Serial.println("ERROR: VCO Bank Calibration Test FAILED - VBCF Flag: " + String(test_result));
#endif
    } else {
#ifdef GS_DEBUG
        Serial.println(" - Passed.");
#endif
    }
    _a7105.write(A7105_25_VCO_SB_CAL_I, 0x0B); // Set VCO Single band Calibration Register 1 - Manual Calibration settings.

    _a7105.sendStrobe(A7105_STANDBY);

    delay(1);

    _a7105.write(A7105_0F_PLL_I, 0xA0); // Set PLL Register 1 - Select Channel Offset.
    _a7105.sendStrobe(A7105_PLL);
    _a7105.sendStrobe(A7105_RX);

    // Setup RSSI measurement.
    _a7105.write(A7105_1E_ADC, 0xC3); // Set ADC Control Register (x1E) - RSSI Margin: 20, RSSI Measurement continue, FSARS: 4 MHZ, XADS = Convert RSS, RSSI measurement selected, RSSI continuous mode.

    // Cycle through the 12 channels and identify the best one to use.
#ifdef GS_DEBUG
    Serial.println("Scanning Channel RSSI values:");
#endif
    long chan_rssi[HUBSAN_CHAN_ARR_LEN] = {0,0,0,0,0,0,0,0,0,0,0,0};
    for (unsigned int i = 0; i < HUBSAN_CHAN_ARR_LEN; i++){
        const unsigned int num_samples = 15;
        _a7105.write(A7105_0F_PLL_I, allowed_ch[i]); // Set PLL Register 1 - Select Channel Offset.
        _a7105.sendStrobe(A7105_PLL);
        _a7105.sendStrobe(A7105_RX);
        for (unsigned int j = 0; j < num_samples; j++){
            test_result = _a7105.read(A7105_1D_RSSI_THRESH);
            chan_rssi[i] = (chan_rssi[i] + test_result);
        }
    }

    long temp_rssi = 0;
    for (int i = 0; i < HUBSAN_CHAN_ARR_LEN; i++){
        if (chan_rssi[i] > temp_rssi){
            temp_rssi = chan_rssi[i];
            _channel = allowed_ch[i];
        }
    }
#ifdef GS_DEBUG
    Serial.println(" - Selected Channel: 0x" + String(_channel,HEX));
#endif
    _a7105.write(A7105_28_TX_TEST, 0x1F); // Set TX test Register - TX output: -4.8dBm, current: 14.9mA.
    _a7105.write(A7105_19_RX_GAIN_I, 0x9B); // Set RX Gain register - Manual, Mixer gain: 6dB, LNA gain: 6dB
    _a7105.write(A7105_0F_PLL_I, _channel); // Set PLL Register 1 - Select Channel Offset to channel with the HIGHEST average RSSI from the scanning
    _a7105.sendStrobe(A7105_PLL);
    _a7105.sendStrobe(A7105_STANDBY);

    return 0;
}

void Hubsan::update_crc() {

    int sum = 0;
    for(int i = 0; i < 15; i++)
        sum += packet[i];
    packet[15] = (256 - (sum % 256)) & 0xff;
}

void Hubsan::update_flight_control_crc() {

    int sum = 0;
    uint8_t *flt_cnt_ptr = reinterpret_cast<uint8_t *>(currFlightControls);
    for(int i = 0; i < 15; i++)
        sum += flt_cnt_ptr[i];
    flt_cnt_ptr[15] = (256 - (sum % 256)) & 0xff;
}

void Hubsan::hubsan_build_bind_packet(u8 state) {
    packet[0] = state;
    packet[1] = _channel;
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


int16_t Hubsan::get_channel() {

    static int a=0;
    if (a++<2550) {
        return 0;
    }

    return 128;
}


volatile uint8_t throttle=0, rudder=0, aileron = 0, elevator = 0, flags = 0xe;

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
    packet[9] = flags;
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

void Hubsan::updateFlightControlPtr(q_hubsan_flight_controls_t* const newControls) {

    currFlightControls = newControls;
}

void Hubsan::bind() {

#ifdef GS_DEBUG
    Serial.println("Sending beacon packets...");
#endif
    uint8_t status_byte = 0x00; // variable to hold W/R register data.
    uint8_t *_sessionid = reinterpret_cast<uint8_t*>(&sessionid);


    // Generate 4 byte random session id.
    randomSeed(analogRead(0));
    for (unsigned int i = 0; i < 4; i++){
        _sessionid[i] = random(255);
    }

    for (unsigned int i = 0; i < 16; i++){ // Initialize packet array.
        _txpacket[i] = 0x00;
    }
    _txpacket[0] = 0x01; // Bind level = 01 (Unbound - BEACON lvl 1 Packet)
    _txpacket[1] = _channel; // Selected Channel
    for (unsigned int i = 0; i < 4; i++){
        _txpacket[i+2] = _sessionid[i];
    }
    getChecksum(_txpacket);

    // Transmit ANNOUNCE Packet until a response is heard.
#ifdef GS_DEBUG
    Serial.println("Announce Tx");
#endif
    while (true){
        _a7105.writeData(_txpacket, 16, _channel);
        //printPacket("Announce packet", _txpacket);
        _a7105.sendStrobe(A7105_RX); // Switch to RX mode.
        bool response = false;
        for (int i=0;i<15;i++){ // Listen to see if there was a response.
            status_byte = _a7105.read(A7105_00_MODE);
            if (bitRead(status_byte, 0) == false){
                response = true;
#ifdef GS_DEBUG
                Serial.println("Got response to ANNOUNCE");
#endif
                break;
            }
            delay(1);
        }
        if (response){
            break;
        }
        _a7105.sendStrobe(A7105_STANDBY);
    }
    _a7105.readData(_rxpacket, 16);
    //printPacket("Announce Rx", _rxpacket);

    // Escalate handshake.
    _txpacket[0] = 0x03; // Bind Level = 01 (Unbound - BEACON lvl 3 Packet)
#ifdef GS_DEBUG
    Serial.println("Escalating bind to Level 01, BEACON lvl 3");
#endif
    getChecksum(_txpacket);
    while (true){
        _a7105.writeData(_txpacket, 16, _channel);
        //printPacket("Escalation 1 Tx", _txpacket);
        _a7105.sendStrobe(A7105_RX); // Switch to RX mode.
        bool response = false;
        for (unsigned int i = 0; i < 15; i++){ // Listen to see if there was a response.
            status_byte = _a7105.read(A7105_00_MODE);
            if (bitRead(status_byte, 0) == false){
                response = true;
#ifdef GS_DEBUG
                Serial.println("Got Response to BEACON lvl 3 packet");
#endif
                break;
            }
            delay(1);
        }
        if (response){
            break;
        }
        _a7105.sendStrobe(A7105_STANDBY);
    }
    _a7105.readData(_rxpacket, 16);
    //printPacket("Escalation 1 Rx", _rxpacket);
    delay(50);

    // Set IDCode to the session value.
    //_a7105.setID(0x12345678);
    _a7105.setID(static_cast<uint32_t>(_rxpacket[2]) << 24 |
            static_cast<uint32_t>(_rxpacket[3]) << 16 |
            static_cast<uint32_t>(_rxpacket[4]) << 8 |
            static_cast<uint32_t>(_rxpacket[5]));

    // Commence confirmation handshake.
    _txpacket[0] = 0x01; // Bind Level = 01 (Mid-Bind - Confirmation of IDCODE change packet)
#ifdef GS_DEBUG
    Serial.println("Escalating to MidBind");
#endif
    getChecksum(_txpacket);
    while (true){
        _a7105.writeData(_txpacket, 16, _channel);
        //printPacket("MidBind Tx", _txpacket);
        _a7105.sendStrobe(A7105_RX); // Switch to RX mode.
        bool response = false;
        for (unsigned int i = 0; i < 15; i++){ // Listen to see if there was a response.
            status_byte = _a7105.read(A7105_00_MODE);
            if (bitRead(status_byte, 0) == false){
                response = true;
                break;
            }
            delay(1);
        }
        if (response){
            break;
        }
        _a7105.sendStrobe(A7105_STANDBY);
    }
    _a7105.readData(_rxpacket, 16);
    //printPacket("MidBind Rx", _rxpacket);

    // Commence full handshake escalation.
#ifdef GS_DEBUG
    Serial.println("commencing full handshake");
#endif
    _txpacket[0] = 0x09;
    for (unsigned int i = 0; i < 10; i++){
        _txpacket[2] = static_cast<uint8_t>(i);
        getChecksum(_txpacket);
        while (true){
            _a7105.writeData(_txpacket, 16, _channel);
            //printPacket("Full Handshake Tx", _txpacket);
            _a7105.sendStrobe(A7105_RX); // Switch to RX mode.
            bool response = false;
            for (unsigned int j = 0; j < 15; j++){ // Listen to see if there was a response.
                status_byte = _a7105.read(A7105_00_MODE);
                if (bitRead(status_byte, 0) == false){
                    response = true;
                    break;
                }
                delay(1);
            }
            if (response){
                break;
            }
            _a7105.sendStrobe(A7105_STANDBY);
        }
        _a7105.readData(_rxpacket, 16);
        //printPacket("Full Handshake Rx", _rxpacket);

    }
    _a7105.write(A7105_1F_CODE_I, 0x0F); // Enable FEC.
    _a7105.sendStrobe(A7105_STANDBY);
#ifdef GS_DEBUG
    Serial.println("Binding finished");
#endif
}

void Hubsan::getChecksum(uint8_t *ppacket) {

    int sum = 0;
    for (int i=0;i<15;i++){
        sum = sum + int(ppacket[i]);
    }
    ppacket[15] = byte(256-(sum % 256));
}

void Hubsan::hubsan_send_data_packet(const uint8_t ch) {

    update_flight_control_crc();
    _a7105.writeData(reinterpret_cast<uint8_t *>(currFlightControls), sizeof(*currFlightControls), ch);
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
        _a7105.writeData(packet, 16, _channel);
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
        if (i == 20) {
#ifdef GS_DEBUG
            Serial.println("Failed to complete write\n");
#endif
        }
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
            _a7105.setID(static_cast<uint32_t>(packet[2]) << 24 |
                    static_cast<uint32_t>(packet[3]) << 16 |
                    static_cast<uint32_t>(packet[4]) << 8 |
                    static_cast<uint32_t>(packet[5]));
        
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
        hubsan_send_data_packet(state == DATA_5 ? _channel + 0x23 : _channel);
        //hubsan_build_packet();
        //_a7105.writeData(packet, 16, state == DATA_5 ? _channel + 0x23 : _channel);
        //_a7105.writeData(packet, 16, _channel);
        if (state == DATA_5)
            state = DATA_1;
        else
            state++;
        return 10000;
    }
    return 0;
}
