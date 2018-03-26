//#include <WProgram.h>
#include <HardwareSerial.h>

void setup(void) {
	Serial.begin(115200);
	while(!Serial){}
	Serial.write(27);
	Serial.print("[2J");
	Serial.println("This is a test");
}

int cnt = 0;
void loop(void) {
	Serial.print("Another test");
	Serial.println(cnt++);
	delay(1000);
}
