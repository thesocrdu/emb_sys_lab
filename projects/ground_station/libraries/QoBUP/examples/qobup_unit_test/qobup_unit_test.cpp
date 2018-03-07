#include "QoBUP.h"
#include <HardwareSerial.h>

QoBUP qb;

const uint8_t cmd[] = {
    0xaa,0x0b,0x1d,
    0x0a,0x06,0x10,0x11,0x12,0x13,
    0xa5,0x02
};

q_status_msg_t status;

void setup(void) {
    Serial.begin(115200);
    while(!Serial){}
    Serial.write(27);
    Serial.print("[2J");
}

int cnt = 0;
void loop(void) {
    Serial.println("Validating Cmd:");
    printCmd();

    status = qb.validateMessage(cmd);

    Serial.println("Status returned:");
    printStatus();
    Serial.print("\n\n");

    while(1);
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
    delay(1000);
}
