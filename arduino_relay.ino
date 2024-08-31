// Relay pin is controlled with D8 pin in Arduino Nano
int relay = 8;

void setup() {
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH); // Ensure relay starts off
  Serial.begin(115200);
}

void loop() {
  if (Serial.available() > 0) { // Check if data is available
    int relayState = Serial.read();

    // Check for ASCII values of '1' and '0'
    if (relayState == '0') {
      digitalWrite(relay, HIGH);
    } else if (relayState == '1') {
      digitalWrite(relay, LOW);
    }
  }
}
