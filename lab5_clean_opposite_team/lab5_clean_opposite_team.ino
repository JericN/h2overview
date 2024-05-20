#include <PubSubClient.h>

// #include <ESP8266WiFi.h>  // For D1 R1
#include <WiFi.h>   // For ESP32
// #define MYLED D6  // For D1 R1
#define MYLED 2   // For ESP32
// #define BUTTON D5 // For D1 R1
#define BUTTON 4
#define SENSOR 32

char morseCode[500];
char message[500];

// ==============================================================================
// ============================= MQTT SECTION ===================================
// ==============================================================================

WiFiClient espClient;
PubSubClient client(espClient);

const char *ssid = "dcs-students2";
const char *password = "W1F14students";
const char *mqtt_server = "broker.mqtt-dashboard.com";

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    char clientId[50];
    snprintf(clientId, 100,"ESP8266Client-%04X", random(0xffff));
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // FIXME: verify if this is the correct topic
      client.subscribe("cs145/0x3/out");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}










// ==============================================================================
// ============================= MORSE CODE SECTION =============================
// ==============================================================================

typedef struct {
  const char character;
  const char *morse;
} MorseCodeMap;

static const MorseCodeMap morse_map[] = {
    {'A', ".-"},    {'B', "-..."},  {'C', "-.-."},  {'D', "-.."},
    {'E', "."},     {'F', "..-."},  {'G', "--."},   {'H', "...."},
    {'I', ".."},    {'J', ".---"},  {'K', "-.-"},   {'L', ".-.."},
    {'M', "--"},    {'N', "-."},    {'O', "---"},   {'P', ".--."},
    {'Q', "--.-"},  {'R', ".-."},   {'S', "..."},   {'T', "-"},
    {'U', "..-"},   {'V', "...-"},  {'W', ".--"},   {'X', "-..-"},
    {'Y', "-.--"},  {'Z', "--.."},  {'0', "-----"}, {'1', ".----"},
    {'2', "..---"}, {'3', "...--"}, {'4', "....-"}, {'5', "....."},
    {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."},
};

// Function to convert Morse code to character
const char morse_to_char(const char *morse) {
  for (int i = 0; i < 36 && morse_map[i].morse != NULL; i++) {
    if (strcmp(morse, morse_map[i].morse) == 0) return morse_map[i].character;
  }
  return ' ';
}

// Function to convert character to Morse code
const char *char_to_morse(char character) {
  for (int i = 0; morse_map[i].morse != NULL; i++) {
    if (morse_map[i].character == character) {
      return morse_map[i].morse;
    }
  }
  return "";
}

// Function to convert Morse code message to a String message
void convert_morse_to_message() {
  char *token = strtok(morseCode, " ");
  char *ptr = message;

  // while (token != NULL) {
  while (token != NULL) {
    int mtc = morse_to_char(token);
    if (mtc == ' ') break;

    *ptr++ = mtc;
    token = strtok(NULL, " ");
  }

  *ptr = '\0';
}

// Function to check if the morse code contains a space
bool contain_space_flag() {
  for (int count = 0, i = 0; morseCode[i] != '\0'; i++) {
    count = (morseCode[i] == '.') ? count + 1 : 0;
    if (count == 7) return true;
  }
  return false;
}

// Function to add padding to the morse code
void add_letter_padding() {
  int length = strlen(morseCode);
  if (length == 0) return;
  if (morseCode[length - 1] != ' ') {
    strcat(morseCode, " ");
  };
}










// =============================================================================
// =========================== HELPER FUNCTIONS ================================
// =============================================================================

// Send message to the other team using MQTT
void mqtt_send(char *topic, char *msg) { 
  Serial.print("MQTT SEND: ");
  Serial.println(msg);
  client.publish(topic, msg); 
}

// Light up the LED for a dot
void led_dot() {
  digitalWrite(MYLED, HIGH);
  delay(250);
  digitalWrite(MYLED, LOW);
  delay(2000);
}

// Light up the LED for a dash
void led_dash() {
  digitalWrite(MYLED, HIGH);
  delay(1000);
  digitalWrite(MYLED, LOW);
  delay(2000);
}












// ==============================================================================
// ============================= DECODER SECTION ================================
// ==============================================================================

void decode_message(char *payload, unsigned int length) {
  char topic[500];

  for (unsigned int i = 0; i < length; i++) {
    // Initialize topic with the base string
    strcpy(topic, "cs145/0x7/RAW_MORSE/");
    char char_to_add[2] = {payload[i], '\0'};
    strcat(topic, char_to_add);

    // convert the character to morse code
    const char *morse = char_to_morse((char)payload[i]);

    // light up the LED according to the morse code
    // and send the corresponding message
    for (int j = 0; j < strlen(morse); j++) {
      if (morse[j] == '.') {
        led_dot();
        mqtt_send(topic, (char *)"DOT");
      } else if (morse[j] == '-') {
        led_dash();
        mqtt_send(topic, (char *)"DASH");
      }
    }

    // send the "..." after each character
    // Initialize topic with the base string
    strcpy(topic, "cs145/0x7/RAW_MORSE/");
    strcat(topic, "dot");
    mqtt_send(topic, (char *)"...");
  }
}

// Function procedure to terminate the connection
void shutdown() {
  // Flash the "END" message in Morse code.
  // FIXME: do we need padding between the letters E N D?
  const char *END_MORSE = ".-.-..";
  for (int i = 0; i < strlen(END_MORSE); i++) {
    if (END_MORSE[i] == '.') led_dot();
    else led_dash();
  }

  // Send "END" back and disconnect MQTT
  // FIXME: change to correct topic
  Serial.println("Sending");
  mqtt_send((char *)"cs145/0x7/out", (char *)"END");
  client.unsubscribe("cs145/0x3/out");
  client.disconnect();
  Serial.println("MQTT Terminated.");
}

void callback(char *topic, byte *payload, unsigned int length) {
  char* payloadStr = (char*) malloc(length + 1);
  if (payloadStr == NULL) {
      Serial.println("Memory allocation failed!");
      return;
  }

  memcpy(payloadStr, payload, length);
  payloadStr[length] = '\0';

  Serial.print("Message arrived [");
  Serial.print(payloadStr);
  Serial.print("] ");

  // Execute shutdown if the message is "END"
  if (strcmp((char *)payload, "END") == 0) shutdown();
  // Otherwise, execute the decoding procedure
  else decode_message((char *)payloadStr, length);

  Serial.println("Deconding Done!");
  delay(10000);
}













// =============================================================================
// ============================== ENCODER SECTION ==============================
// =============================================================================

bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 100;
unsigned int lastInputTime = 0;
unsigned int inputDelay = 2000;

// Takes the input from the LDR and the button
// The morse code is stored in the `morseCode` variable
void get_user_input() {
  int reading = digitalRead(BUTTON);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay && reading != buttonState) {
    buttonState = reading;
    if (buttonState == HIGH) {
      // read the input from the LDR
      int ldrOutput = analogRead(SENSOR);

      // save the morse code
      if (ldrOutput < 800) strcat(morseCode, ".");
      else strcat(morseCode, "-");

      // used to determine the end of a letter
      lastInputTime = millis();
      digitalWrite(MYLED, LOW);
    } else {
      digitalWrite(MYLED, HIGH);
    }
  }

  lastButtonState = reading;
  // FIXME: remove this after debugging
  Serial.println(morseCode);
}










// =============================================================================
// ============================== SETUP AND LOOP ===============================
// =============================================================================

void setup() {
  Serial.begin(9600);
  pinMode(MYLED, OUTPUT);
  pinMode(SENSOR, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  // FIXME: verify if this is the correct topic
  client.subscribe("cs145/0x3/out");
  client.setCallback(callback);
  delay(10000);
}

void loop() {
  // MQTT LOGIC
  if (!client.connected()) reconnect();
  client.loop();

  // get user input
  get_user_input();

  // check for end of letter
  int current_input = millis();
  if (current_input - lastInputTime > inputDelay) {
    add_letter_padding();
  }

  // check for end of message
  if (contain_space_flag()) {
    // convert the morse code to message
    convert_morse_to_message();

    // send the message to the other team
    // FIXME: verify if this is the correct topic
    mqtt_send((char *)"cs145/0x7/out", message);

    // clear the message and morse code
    morseCode[0] = '\0';
    message[0] = '\0';
  }
}
