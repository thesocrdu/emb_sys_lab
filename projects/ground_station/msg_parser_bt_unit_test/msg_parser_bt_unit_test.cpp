#include <bt_smirf.h>
#include <QoBUP.h>
#include <Q_Hubsan.h>
#include <SoftwareSerial.h>

#define BT_RX_PIN 4
#define BT_TX_PIN 3
#define BT_SERIAL_IF softSerial
#define CMD_BUFF_SIZE 32

static Q_Hubsan qh;
static SoftwareSerial BT_SERIAL_IF(BT_RX_PIN, BT_TX_PIN);
static bt_smirf bt(BT_SERIAL_IF);

q_status_msg_t status;
q_hubsan_flight_controls_t flightControls;
uint8_t cmd[CMD_BUFF_SIZE];

void setup(void) {
    Serial.begin(115200);
    while(!Serial){}

    Serial.write(27);
    Serial.print("[2J");

    bt.begin(9600);
    bt.exitCmdMode();
}


void loop(void) {

    /* Get new message from the bluetooth device and validate it */
    Serial.println("Waiting for Bluetooth data:");
    status = qh.serialRxMsg(BT_SERIAL_IF, cmd, CMD_BUFF_SIZE);

    /* Print out the received command */
    Serial.println("Cmd received/parsed:");
    printCmd();

    /* Print the resulting status of the message  */
    printStatus();

    if (status.status.word == 0) {
        /* Translate the message. */
        qh.parseMessage(cmd);
        qh.getFlightControls(flightControls);
        printFlightControls();
    } else {
        Serial.println("Status non-zero. Not parsing.");
    }

    /* Send response back to transmitter */
    sendStatusResp();

    Serial.println();
}

void sendStatusResp() {
    BT_SERIAL_IF.write(status.sid);
    BT_SERIAL_IF.write(status.status.word);
}

void printCmd() {
    for (unsigned int i = 0; i < sizeof(cmd); i++) {
        Serial.print(cmd[i], HEX);
        Serial.print(" ");
        if ((i + 1) %  16 == 0) {
            Serial.println();
        }
    }
    Serial.println();
}

void printStatus() {
    Serial.print("status = ");
    Serial.println(status.status.word, HEX);
}

void printFlightControls() {
    uint8_t *ptr = reinterpret_cast<uint8_t*>(&flightControls);
    Serial.println("Flight controls:");
    for (unsigned int i = 0; i < sizeof(q_hubsan_flight_controls_t); i++) {
        Serial.println(ptr[i], HEX);
    }
}
