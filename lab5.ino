/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are char*s not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "........";
const char* password = "........";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

const char* END_MORSE = "...-.--.";

char* char_to_morse(char c) {
    switch (c) {
    case 'A':
        return ".-";
    case 'B':
        return "-...";
    case 'C':
        return "-.-.";
    case 'D':
        return "-..";
    case 'E':
        return ".";
    case 'F':
        return "..-.";
    case 'G':
        return "--.";
    case 'H':
        return "....";
    case 'I':
        return "..";
    case 'J':
        return ".---";
    case 'K':
        return "-.-";
    case 'L':
        return ".-..";
    case 'M':
        return "--";
    case 'N':
        return "-.";
    case 'O':
        return "---";
    case 'P':
        return ".--.";
    case 'Q':
        return "--.-";
    case 'R':
        return ".-.";
    case 'S':
        return "...";
    case 'T':
        return "-";
    case 'U':
        return "..-";
    case 'V':
        return "...-";
    case 'W':
        return ".--";
    case 'X':
        return "-..-";
    case 'Y':
        return "-.--";
    case 'Z':
        return "--..";
    case '0':
        return "-----";
    case '1':
        return ".----";
    case '2':
        return "..---";
    case '3':
        return "...--";
    case '4':
        return "....-";
    case '5':
        return ".....";
    case '6':
        return "-....";
    case '7':
        return "--...";
    case '8':
        return "---..";
    case '9':
        return "----.";
    default:
        Serial.println("Invalid character");
        return "";
    }
}

void setup_wifi() {

    delay(10);
    // We start by connecting to a WiFi network
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

// Send message to the other team using MQTT
void mqtt_send(char* topic, char* msg) {
    // FIXME: update to other team's endpoint
    char* endpoint = "cs145/0xPAKYU/" + topic;
    client.publish(endpoint, msg);
}

// Light up the LED for a dot
void led_dot() {
    digitalWrite(BUILTIN_LED, LOW);
    delay(250);
    digitalWrite(BUILTIN_LED, HIGH);
    // FIXME: correct timeout delay
    delay(1000);
}

// Light up the LED for a dash
void led_dash() {
    digitalWrite(BUILTIN_LED, LOW);
    delay(1000);
    digitalWrite(BUILTIN_LED, HIGH);
    // FIXME: correct timeout delay
    delay(1000);
}

// Function procedure to terminate the connection
void shutdown() {
    // Flash the "END" message in Morse code.
    for (int i = 0; i < strlen(END_MORSE); i++) {
        if (END_MORSE[i] == '.') led_dot();
        else led_dash();
    }

    // Send "END" back and disconnect MQTT
    mqtt_send("in", "END");
    // FIXME: need to change this to the correct topic
    client.unsubscribe("ENEMY TOPIC");
    client.disconnect();
    Serial.println("MQTT Terminated.");
}

// Function procedure for decoded Morse code
void decode_action(char* morse, char* topic) {
    for (int i = 0; i < strlen(morse); i++) {
        if (morse[i] == '.') {
            led_dot();
            mqtt_send(topic, "DOT");
        } else if (morse[i] == '-') {
            led_dash();
            mqtt_send(topic, "DASH");
        }
    }
}

void decode(byte* payload, unsigned int length) {
    // Iterate through the payload
    for (int i = 0; i < length; i++) {
        // Convert the character to Morse code
        char letter = (char)payload[i];
        char* morse = char_to_morse(letter);
        char* topic = "RAW_MORSE/" + letter;

        // Execute the decoding procedure
        decode_action(morse, topic);
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [" + topic + "]");
    // TODO: listerner address, di ko sure anong topic

    // Execute shutdown if the message is "END"
    if (strcmp((char*)payload, "END") == 0) shutdown();
    // Otherwise, execute the decoding procedure
    else decode(payload, length);
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        char* clientId = "ESP8266Client-";
        clientId += char*(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish("outTopic", "hello world");
            // ... and resubscribe
            // FIXME: need to change this to the correct topic
            client.subscribe("inTopic");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup() {
    pinMode(BUILTIN_LED, OUTPUT); // Initialize the BUILTIN_LED pin as an output
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop() {

    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > 2000) {
        lastMsg = now;
        ++value;
        snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
        Serial.print("Publish message: ");
        Serial.println(msg);
        // FIXME: need to change this to the correct topic
        client.publish("outTopic", msg);
    }
}
