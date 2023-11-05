#include "ESPUserConnection.h"

void setup() {
    Serial.begin(115200);
    connect_wifi();
}

void loop () {
    Serial.print("(-.-) ");
    delay(1000);
}
