
const int pulsePin = 8; // Input signal connected to A1 of Metro M0 Express
int pulseHigh; // Integer variable to capture High time of the incoming pulse
int pulseLow; // Integer variable to capture Low time of the incoming pulse
float pulseTotal; // Float variable to capture Total time of the incoming pulse
float frequency; // Calculated Frequency

void setup() {

//Pin setup
pinMode(pulsePin, INPUT);
delay(5000);
}

void loop() {

pulseHigh = pulseIn(pulsePin, HIGH);
pulseLow = pulseIn(pulsePin, LOW);
pulseTotal = pulseHigh + pulseLow; // Time period of the pulse in microseconds
frequency = 1000000 / pulseTotal; // Frequency in Hertz (Hz)

Serial.println(frequency);

delay(500);
}
