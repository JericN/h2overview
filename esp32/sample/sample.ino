#define PHOTO_SENSOR 2
#define LED 4
void setup() {
  pinMode(PHOTO_SENSOR, INPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  Serial.println("Hello, World!");
  delay(5000);
}

void loop() {
  int lightVal = analogRead(PHOTO_SENSOR); // read the current light levels
  Serial.println(lightVal); // print the light levels to the serial monitor
  if (lightVal < 100) {
    Serial.println("light");
    digitalWrite(LED, HIGH);
  } else {
    Serial.println("dark");
    digitalWrite(LED, LOW);
  }
  delay(500); // wait for a second
}
