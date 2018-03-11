#include "QoBUP.h"
#include <HardwareSerial.h>

QoBUP qb;
q_status_msg_t status;

uint8_t cmd[32];

void setup(void) {
    Serial.begin(115200);
    while(!Serial){}
    Serial.write(27);
    Serial.print("[2J");
}


void loop(void) {
    Serial.println("Send a command:");
    status = qb.serialRxMsg(Serial, cmd, 32);
    printStatus();
    Serial.println();
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
