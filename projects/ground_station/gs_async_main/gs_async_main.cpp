/**
 * @file
 * @brief This file serves as the top level
 * driver program for the entire ground station.
 *
 * @author Kyle Mercer
 *
 */

#include <bt_smirf.h>
#include <Hubsan.h>
#include <Q_Hubsan.h>


#ifdef GS_DEBUG

#include <SoftwareSerial.h>
#define BT_RX_PIN 3
#define BT_TX_PIN 4
#define BT_SERIAL_IF softSerial
#define BT_BAUD 9600
static SoftwareSerial BT_SERIAL_IF(BT_RX_PIN, BT_TX_PIN);

#else

#define BT_SERIAL_IF Serial
#define BT_BAUD 57600

#endif

#define CMD_BUFF_SIZE 32
#define CS_PIN 9
#define HUBSAN_TX_PERIOD_US 10000

static bt_smirf bt(BT_SERIAL_IF);
static Q_Hubsan qh;
static Hubsan hubs;

uint8_t cmd[CMD_BUFF_SIZE];
q_hubsan_flight_controls_t fltCnt;
q_status_msg_t status;
unsigned long txTimestamp = 0;

void hubsanControlUpdate() {

    hubs.hubsan_send_data_packet(0);
}

void initSerialDebug(void) {
    
#ifdef GS_DEBUG
    Serial.begin(115200);
    while(!Serial){}

    Serial.write(27);
    Serial.print("[2J");
#endif
}

void initBluetoothInterface(void) {

    bt.begin(BT_BAUD);
    bt.exitCmdMode();
}

void initHubsanInterface(void) {

    hubs.init(CS_PIN);

    qh.getFlightControls(fltCnt);
    hubs.updateFlightControlPtr(&fltCnt);
    hubs.bind();
}

void initTimer() {

    txTimestamp = micros();
}

void sendStatusResp() {
    BT_SERIAL_IF.write(status.sid);
    BT_SERIAL_IF.write(status.status.word);
}

void setup(void) {

    initSerialDebug();
    initBluetoothInterface();
    initHubsanInterface();
    initTimer();
}

void loop(void) {

    if (BT_SERIAL_IF.available() >= 3) {
        status = qh.serialRxMsg(BT_SERIAL_IF, cmd, CMD_BUFF_SIZE);

        if (status.status.word != 0) {
#ifdef GS_DEBUG
            Serial.print("Err: status = ");
            Serial.println(status.status.word, HEX);
#endif
        } else if (qh.parseMessage(cmd) != 0) {
#ifdef GS_DEBUG
            Serial.println("Err: failed to parse message");
#endif
        } else {
            qh.getFlightControls(fltCnt);
            hubs.updateFlightControlPtr(&fltCnt);
            //printFltControls();
        }
        sendStatusResp();
    }

    /*
     * Wait 10ms since last update to send
     * new controls to Hubsan quadcopter.
     */
    while(txTimestamp - micros() < HUBSAN_TX_PERIOD_US);
    txTimestamp = micros();
    hubsanControlUpdate();
}
