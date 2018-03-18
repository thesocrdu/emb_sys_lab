#include "A7105.h"

#define A7105_CS_PIN 9

A7105 a7105_if;

void setup(void) {
    Serial.begin(115200);
    while(!Serial){}
    Serial.write(27);
    Serial.print("[2J");

    a7105_if.begin(A7105_CS_PIN);
}

void loop(void) {
    uint8_t data;
    delay(1000);

    //a7105_if.write(A7105_01_MODE_CONTROL, 0x02);
    //delay(1000);
    a7105_if.read(A7105_0D_CLOCK, data);
    a7105_if.read(A7105_00_MODE, data);
    //Serial.print("A7105_19_RX_GAIN_I = ");
    //Serial.println(data, HEX);

    //delay(1000);
    //a7105_if.write(A7105_0C_GIO2_PIN_II, 0x19);
}
