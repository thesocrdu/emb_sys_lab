#include <bt_smirf.h>
#include <QoBUP.h>

#define BT_SERIAL_IF Serial2
#define CMD_BUFF_SIZE 32

static QoBUP qb;
static bt_smirf bt(BT_SERIAL_IF);
q_status_msg_t status;

uint8_t cmd[CMD_BUFF_SIZE];

void setup(void) {
    Serial.begin(115200);
    while(!Serial){}

    bt.begin(115200);
    bt.exitCmdMode();

    Serial.write(27);
    Serial.print("[2J");
}


void loop(void) {

    /* Get new message from the bluetooth device and validate it */
    Serial.println("Waiting for Bluetooth data:");
    status = qb.serialRxMsg(BT_SERIAL_IF, cmd, CMD_BUFF_SIZE);

    /* Print out the received command */
    Serial.println("Cmd received/parsed:");
    printCmd();

    /* Print the resulting status of the message  */
    printStatus();

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
