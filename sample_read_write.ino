#include <ESP8266Firebase.h>
#include <ESP8266WiFi.h>

#define _SSID "Raspberry"
#define _PASSWORD "54321edcba"
#define REFERENCE_URL "https://h2overview-iot-default-rtdb.asia-southeast1.firebasedatabase.app/"

Firebase firebase(REFERENCE_URL);

int counter = 0;
unsigned long prevTime = 0;
const long interval = 10000;

void setup()
{
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

  // Connect to WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(_SSID);
  WiFi.begin(_SSID, _PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("-");
  }

  Serial.println("");
  Serial.println("WiFi Connected");

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  digitalWrite(LED_BUILTIN, HIGH);

  // get initial data
  counter = firebase.getInt("Counter/num");
}

void loop()
{
  // Loop every 'interval' milliseconds
  unsigned long currTime = millis();
  if (currTime - prevTime < interval)
    return;

  // Print the current timestamp
  Serial.print("Timestamp: ");
  Serial.print(currTime);
  Serial.print("  |  ");

  // Write to firebase
  firebase.setInt("Counter/num", counter);

  // Print write delay
  Serial.print("Set: ");
  Serial.print(millis() - prevTime - interval);
  Serial.print("  |  ");

  // Read from firebase
  int flag = firebase.getInt("Counter/flag");
  if (flag == 1) {
    digitalWrite(LED_BUILTIN, LOW);
    counter++;
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  // Print read delay
  Serial.print("Flag: ");
  Serial.print(millis() - prevTime - interval);
  Serial.print("  |  ");

  // Logs
  Serial.print("Counter: ");
  Serial.println(counter-1);

  // Update the previous timestamp
  prevTime = currTime;
}

