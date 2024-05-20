#define pressureSensorPin A0
int flowSensorPin = 2;

int relay = 3;
volatile long pulse;
float volume;
int openSolenoid = 0;
float lastTime = 0.0;

void setup() {
  // put your setup code here, to run once:
  pinMode(flowSensorPin, INPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), increase, RISING);

  pinMode(relay, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

  // Water flow sensor
  volume = 2.663 * pulse;
  if (millis() - lastTime > 1000) {
    pulse = 0;
    lastTime = millis();
  }
  Serial.print("Flow Rate: ");
  Serial.print(volume);
  Serial.println(" mL/s");
1
  // solenoid
  // if (openSolenoid == 1) {
  //   digitalWrite(relay, HIGH);
  // } else {
  //   digitalWrite(relay, LOW);
  // }

  // Pressure sensor
  int pressureSensorValue = analogRead(pressureSensorPin);
  float pressureVoltage = pressureSensorValue * (5.0 / 1023.0);
  float baselinePressureVoltage = 0.45;
  float pressure = (pressureVoltage - baselinePressureVoltage) * (100.0 / (4.5 - baselinePressureVoltage));
  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" psi");
  delay(1000);
}

void increase() {
  pulse++;
}