#include <bt_smirf.h>
#include <Hubsan.h>
#include <Q_Hubsan.h>
#include <SoftwareSerial.h>

#define BT_RX_PIN 3
#define BT_TX_PIN 4
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
    bt.begin(9600);
    bt.exitCmdMode();

    /* Initialize the Hubsan interface. */
    hubs.init(CS_PIN);

    //printFltControls();
    qh.getFlightControls(fltCnt);
    hubs.updateFlightControlPtr(&fltCnt);
    //printFltControls();
    hubs.bind();
}

void loop(void) {

    if (BT_SERIAL_IF.available() >= 3) {
        status = qh.serialRxMsg(BT_SERIAL_IF, cmd, CMD_BUFF_SIZE);
        printCmdBuffer();

        if (status.status.word != 0) {
            Serial.print("Err: status = ");
            Serial.println(status.status.word, HEX);
        } else if (qh.parseMessage(cmd) != 0) {
            Serial.println("Err: failed to parse message");
        } else {
            qh.getFlightControls(fltCnt);
            hubs.updateFlightControlPtr(&fltCnt);
            //printFltControls();
        }
    }

    //uint16_t hubsanWait = hubs.hubsan_cb();
    //delayMicroseconds(hubsanWait - 400);
    hubs.hubsan_send_data_packet();
    delay(10);
    
}

void printFltControls() {

    Serial.write(reinterpret_cast<uint8_t*>(&fltCnt), sizeof(fltCnt));
}

void printCmdBuffer() {

    for (unsigned int i = 0; i < CMD_BUFF_SIZE; i++) {
        Serial.print(cmd[i],HEX);
        Serial.print(" ");
    }
    Serial.println();
}
