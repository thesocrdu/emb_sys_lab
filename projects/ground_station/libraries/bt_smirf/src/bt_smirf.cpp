/**
 * @file
 * @brief This file is used for interfacing with the BlueSMiRF Silver
 *
 * @author Kyle Mercer
 *
 */

#include "bt_smirf.h"

/** Factory default baud rate of module. */
#define BT_SMIFT_DEFAULT_BAUD 115200

/**
 * @defgroup BlueSMiRF AT Commands
 * @{
 */

#define ENTER_CMD       "$$$"
#define EXIT_CMD        "---"
#define INFO_CMD        "D"
#define EXTINFO_CMD     "E"
#define INQUIRY_CMD     "I"
#define INQUIRYN_CMD    "IN"
#define BAUD_CMD        "U,%s,N"
#define SETNAME_CMD     "SN"
#define REBOOT_CMD      "R,1"
#define HELP_CMD        "H"
#define LINKQLT_CMD     "L"
#define QUIET_CMD       "Q"
#define WAKE_CMD        "W"

/** @} */

bt_smirf::bt_smirf(HardwareSerial &h) {
    _serial_if = &h;
    _hw_serial = &h;
    _sw_serial = NULL;
    _internal_serial = false;
    _curr_mode = DATA_MODE;
}

bt_smirf::bt_smirf(SoftwareSerial &s) {
    _serial_if = &s;
    _hw_serial = NULL;
    _sw_serial = &s;
    _internal_serial = false;
    _curr_mode = DATA_MODE;
}

bt_smirf::bt_smirf(const uint8_t rx, const uint8_t tx) {
    _sw_serial = new SoftwareSerial(rx, tx);
    _hw_serial = NULL;
    _serial_if = _sw_serial;
    _internal_serial = true;
    _curr_mode = DATA_MODE;
}

bt_smirf::~bt_smirf() {}

const char* bt_smirf::getResponse() {

    size_t i;
    for (i = 0; i < BT_SMIRF_RESP_BUF_LEN - 1; i++) {
        if (_serial_if->available()){
            _cmdResp[i] = _serial_if->read();
        } else {
            break;
        }
    }
    _cmdResp[i] = '\0';

#ifdef GS_DEBUG
    size_t j;
    for(j = 0; j <= i; j++){
        Serial.print(_cmdResp[j]);
    }
#endif

    return _cmdResp;
}

const char* bt_smirf::sendCmd(const char *cmd, bool appendNewline) {

    if (appendNewline) {
        _serial_if->println(cmd);
    } else {
        _serial_if->print(cmd);
    }

    /* Give the module some time to respond. */
    delay(100);

    return getResponse();
}

void bt_smirf::begin(const long baud_rate) {

    /* The BlueSmirf modules default to 115200 */
    if (_sw_serial) {
        _sw_serial->begin(BT_SMIFT_DEFAULT_BAUD);
        while(!*_sw_serial);
    } else if (_hw_serial) {
        _hw_serial->begin(BT_SMIFT_DEFAULT_BAUD);
        while(!*_hw_serial);
    } else {
        // Should never hit this case.
    }

    resetCmdQueue();
    enterCmdMode();

    if (baud_rate != BT_SMIFT_DEFAULT_BAUD) {
        char cmd[16];
        const char *newBaud;

        switch (baud_rate) {
            case 1200:
                newBaud = "1200";
                break;
            case 2400:
                newBaud = "2400";
                break;
            case 4800:
                newBaud = "4800";
                break;
            case 9600:
                newBaud = "9600";
                break;
            case 19200:
                newBaud = "19.K";
                break;
            case 38400:
                newBaud = "38.K";
                break;
            case 57600:
                newBaud = "57.K";
                break;
            case 115200:
                newBaud = "115K";
                break;
            case 230400:
                newBaud = "230K";
                break;
            case 460800:
                newBaud = "460K";
                break;
            case 921600:
                newBaud = "921K";
                break;

            default:
                /* Do something safe. */
                newBaud = "9600";
                break;

        }

        snprintf(cmd, 16, BAUD_CMD, newBaud);
        sendCmd(cmd);
        getResponse();

        if (_sw_serial) {
            _sw_serial->begin(baud_rate);
            while(!*_sw_serial);
        } else if (_hw_serial) {
            _hw_serial->begin(baud_rate);
            while(!*_hw_serial);
        }
        delay(100);
        /*
         * According to the AT command reference
         * setting a new baud rate will cause the
         * device to immediately exit cmd mode.
         * So let's put it back into cmd mode.
         */
        enterCmdMode();
    }
}

int bt_smirf::enterCmdMode() {

    const char *resp = sendCmd(ENTER_CMD, false);
    if (strcmp(resp, "CMD\r\n") != 0) {
        return -1;
    }
    _curr_mode = CMD_MODE;
    return 0;
}

int bt_smirf::exitCmdMode() {

    const char *resp = sendCmd(EXIT_CMD);
    if (strcmp(resp, "END\r\n") != 0) {
        return -1;
    }
    _curr_mode = CMD_MODE;
    return 0;
}

void bt_smirf::resetCmdQueue() {
    sendCmd("\r\n", false);
}

const char* bt_smirf::flushRxBuffer() {
    return getResponse();
}
