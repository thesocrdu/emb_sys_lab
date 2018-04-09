#include <bt_smirf.h>
#include <Hubsan.h>
#include <Q_Hubsan.h>
#include <SoftwareSerial.h>

#define BT_RX_PIN 4
#define BT_TX_PIN 3
#define BT_SERIAL_IF softSerial
#define CMD_BUFF_SIZE 32
#define CS_PIN 9

static SoftwareSerial BT_SERIAL_IF(BT_RX_PIN, BT_TX_PIN);
static bt_smirf bt(BT_SERIAL_IF);
static Q_Hubsan qh;
static Hubsan hubs;

uint8_t cmd[CMD_BUFF_SIZE];
q_hubsan_flight_controls_t fltCnt;
q_status_msg_t status;

void setup(void) {
    Serial.begin(115200);
    while(!Serial){}

    Serial.write(27);
    Serial.print("[2J");

    /* Init bluetooth module. */
    bt.begin(115200);
    bt.exitCmdMode();

    /* Initialize the Hubsan interface. */
    hubs.init(CS_PIN);

}

void loop(void) {

    if (BT_SERIAL_IF.available() > 3) {
        status = qh.serialRxMsg(BT_SERIAL_IF, cmd, CMD_BUFF_SIZE);
        
        if(qh.parseMessage(cmd) != 0) {
            Serial.println("Err: failed to parse message");
        } else {
            qh.getFlightControls(fltCnt);
            hubs.updateFlightControlPtr(&fltCnt);
        }
    }

    uint16_t hubsanWait = hubs.hubsan_cb();
    delayMicroseconds(hubsanWait - 400);
    
}
