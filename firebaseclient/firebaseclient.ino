#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseClient.h>

#define WIFI_SSID "Arduino"
#define WIFI_PASSWORD "edcba54321"

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "Web_API_KEY"

#define FIREBASE_PROJECT_ID "h2overview-iot"                                                    // Taken from "project_id" key in JSON file.
#define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-w6f0t@h2overview-iot.iam.gserviceaccount.com"  // Taken from "client_email" key in JSON file.
const char PRIVATE_KEY[] PROGMEM =
    "-----BEGIN PRIVATE "
    "KEY-----\nMIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCfIqHkaUPPA/wx\nAMfBgu3P+99TaA8maL7mzmnsg9VNKnn6UF912MSpDfb3kO9bkugz/"
    "lRRas3y8cIm\nvLlFnqhOqeq6KR65Jw0lUe+YJsn1BGWMx0a6BZvm7Y/yr2bcN/"
    "1M9e6N8o+cBgau\nF2jFO65fD9O2HCY9TYy5XbZWzoaQjTc4A6TVNUKfK7vfIfxDJeQkDZ86SWRRTLnc\ncr0v5cKOOhEg7AJyPjzTHmvRWZXK5nR/FVKzuyVGdYBixcIHeX7/"
    "A6Dox7jlWrOZ\nOT2Dp73X8NnifLCtq9GjPZe9a4Nr7LgM2YtVVou86KEIcRvsTyQsrIglf61KNK19\nCUvA9hNlAgMBAAECggEACQ5ogN44vK7xCueMzAd5JI1PT/"
    "4rtoze0wsg3hPr71X2\ngSH9J3DBqGP3ZRlmIHkDe/StCcEhSAKC1moxGl7axzdGTiKJnBHhM/"
    "mDas5HpLOu\nWnemH8SynaQcErycBvZXalTTUa1h3N2UQxNpTqgvK+ZzrvklIqH82zkTvRbzHjDE\nNUOeqRikxDsy6Ta14ROOUUKaHDTXnXU/"
    "RMwezPKo+PIWfBRoDF8vSLYwxfyqVC+V\nsJ3CgAaM7rDlPIfDr52S3aeRHqPWYhfDccpM5IsliUblpbkzlVbMnZyYdpTw7Coz\nbhE/"
    "I7xbt4zxpEd7TJGr4jeJNJuh4aCD7HjHg5yY9QKBgQDL2V3+x1+DsqVufyxf\nBLi8m1m4h7GFR0ev5x09alwxvZL8FMKHNA3cBwX++ZAuxBPy8EisTh1qq68d5ja4\nGQW73a3ntcJZ0q+FI+"
    "qb1guNia03TULFZPfvn+"
    "b8a7bkAzhXf1HNzkmjhQ0WMdp3\nEiwSxFyGuQCvMonVC2VeK3FPdwKBgQDH2NYFdWm3ubYcYleONCV9x6ZsKuXEBUUq\nlOSAXKAnQLCjpQc4yvlgUupP9Oe99N4jMEC7sJD3CTQqhETk3QAG6CbhqL1S"
    "ZwQI\nKl79RqYuffYZ7RgmjQSPOrFGAwSHic2Mtwh+G0DV67bx07J4vJn3FKiik/"
    "OfQvOE\n43DoUVpDAwKBgGIUF3F6I66NyaK8dXDSKMA2TpjGUNc7UHaF0D+4aNtVxt6abm32\n0aRHjM787C5UEPUWPyEIBIdKS1srZLB2+"
    "ZdGNWICxZvKDsEUYtDPz4ct8rVk6vdm\neRRF7zEL8lcKZZVToNxQoIWYgo9nNSGNEL+G0q3PXVDuE/r5HLCOXlpJAoGAM/"
    "97\nUFhIdYFgnOoJlfqDdsKnzqUqu2ITqQysl6mCEjSGwU2DH4fQcvuf88XpNkesNMOt\nVCiQz3YZmKZptpK6GxH0a+qXuAUNy4IazRPA0X/8tKo4Cm/"
    "Oq+W9klXeq9VzIhXN\no+I7cG2wYCOQ4FFSTjOV3RdZ4CpvajgzfH8OWTkCgYBbv43TqsEOewk7ru/e61tS\n8YG2XfDQukxHV/dqc4pLUtcehLGH3IQVKj8uYlJQtkyTcXkDcrjJoB5oEmV/"
    "lhN7\nm3pB6dTI/IraaJfeZ0WZm/U7NFhXr7sn1AwX/NM/ms1wYe67uQ+YOqEphPly4Wz1\n8/Rd4UnOdOG5Ft8YSitVnQ==\n-----END PRIVATE KEY-----\n";  // Taken from
                                                                                                                                      // "private_key" key in
                                                                                                                                      // JSON file.

void authHandler();

void timeStatusCB(uint32_t& ts);

void printResult(AsyncResult& aResult);

void printError(int code, const String& msg);

DefaultNetwork network;  // initilize with boolean parameter to enable/disable network reconnection

// ServiceAuth is required for Databases functions.
ServiceAuth sa_auth(timeStatusCB, FIREBASE_CLIENT_EMAIL, FIREBASE_PROJECT_ID, PRIVATE_KEY, 3000 /* expire period in seconds (<= 3600) */);

FirebaseApp app;

#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;

using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client, getNetwork(network));

Firestore::Databases Databases;

bool taskCompleted = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  Serial.println("Initializing app...");

  ssl_client.setInsecure();
  ssl_client.setBufferSizes(4096, 1024);

  initializeApp(aClient, app, getAuth(sa_auth), asyncCB, "authTask");

  app.getApp<Firestore::Databases>(Databases);
}

void loop() {
  // The async task handler should run inside the main loop
  // without blocking delay or bypassing with millis code blocks.

  // This JWT token process required for ServiceAuth and CustomAuth authentications
  JWT.loop(app.getAuth());

  app.loop();

  Databases.loop();

  if (app.ready() && !taskCompleted) {
    taskCompleted = true;

    Serial.println("Updates a database... ");

    Firestore::Database db;
    db.pointInTimeRecoveryEnablement(Firestore::PointInTimeRecoveryEnablement::POINT_IN_TIME_RECOVERY_ENABLED);

    String updateMask;

    Databases.patch(aClient, Firestore::Parent(FIREBASE_PROJECT_ID, "myDb" /* database Id */), db, updateMask, asyncCB, "patchTask");
  }
}

void timeStatusCB(uint32_t& ts) {
  if (time(nullptr) < FIREBASE_DEFAULT_TS) {
    configTime(3 * 3600, 0, "pool.ntp.org");
    while (time(nullptr) < FIREBASE_DEFAULT_TS) {
      delay(100);
    }
  }
  ts = time(nullptr);
}

void asyncCB(AsyncResult& aResult) {
  // WARNING!
  // Do not put your codes inside the callback and printResult.

  printResult(aResult);
}

void printResult(AsyncResult& aResult) {
  if (aResult.isEvent()) {
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
  }

  if (aResult.isDebug()) {
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  }

  if (aResult.isError()) {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  }

  if (aResult.available()) {
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
  }
}