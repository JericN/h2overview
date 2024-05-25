/**
 * SYNTAX:
 *
 * String Firestore::Documents::batchWrite(<AsyncClient>, <Firestore::Parent>, <Writes>);
 *
 * <AsyncClient> - The async client.
 * <Firestore::Parent> - The Firestore::Parent object included project Id and database Id in its constructor.
 * <Writes> - The writes to apply.
 *
 * The Firebase project Id should be only the name without the firebaseio.com.
 * The Firestore database id should be (default) or empty "".
 *
 * This function returns response payload when task is complete.
 *
 * The complete usage guidelines, please visit https://github.com/mobizt/FirebaseClient
 */

#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseClient.h>


void authHandler();

void timeStatusCB(uint32_t &ts);

void printResult(AsyncResult &aResult);

void printError(int code, const String &msg);

DefaultNetwork network; // initilize with boolean parameter to enable/disable network reconnection

#define WIFI_SSID "Rex Judicata"
#define WIFI_PASSWORD "93291123aaaA."

#define FIREBASE_PROJECT_ID "h2overview-iot"
#define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-w6f0t@h2overview-iot.iam.gserviceaccount.com"

const char PRIVATE_KEY[] PROGMEM =
    "-----BEGIN PRIVATE "
    "KEY-----\nMIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCfIqHkaUPPA/"
    "wx\nAMfBgu3P+99TaA8maL7mzmnsg9VNKnn6UF912MSpDfb3kO9bkugz/"
    "lRRas3y8cIm\nvLlFnqhOqeq6KR65Jw0lUe+YJsn1BGWMx0a6BZvm7Y/yr2bcN/"
    "1M9e6N8o+"
    "cBgau\nF2jFO65fD9O2HCY9TYy5XbZWzoaQjTc4A6TVNUKfK7vfIfxDJeQkDZ86SWRRTLnc\nc"
    "r0v5cKOOhEg7AJyPjzTHmvRWZXK5nR/FVKzuyVGdYBixcIHeX7/"
    "A6Dox7jlWrOZ\nOT2Dp73X8NnifLCtq9GjPZe9a4Nr7LgM2YtVVou86KEIcRvsTyQsrIglf61K"
    "NK19\nCUvA9hNlAgMBAAECggEACQ5ogN44vK7xCueMzAd5JI1PT/"
    "4rtoze0wsg3hPr71X2\ngSH9J3DBqGP3ZRlmIHkDe/StCcEhSAKC1moxGl7axzdGTiKJnBHhM/"
    "mDas5HpLOu\nWnemH8SynaQcErycBvZXalTTUa1h3N2UQxNpTqgvK+"
    "ZzrvklIqH82zkTvRbzHjDE\nNUOeqRikxDsy6Ta14ROOUUKaHDTXnXU/"
    "RMwezPKo+PIWfBRoDF8vSLYwxfyqVC+"
    "V\nsJ3CgAaM7rDlPIfDr52S3aeRHqPWYhfDccpM5IsliUblpbkzlVbMnZyYdpTw7Coz\nbhE/"
    "I7xbt4zxpEd7TJGr4jeJNJuh4aCD7HjHg5yY9QKBgQDL2V3+x1+"
    "DsqVufyxf\nBLi8m1m4h7GFR0ev5x09alwxvZL8FMKHNA3cBwX++"
    "ZAuxBPy8EisTh1qq68d5ja4\nGQW73a3ntcJZ0q+FI+"
    "qb1guNia03TULFZPfvn+"
    "b8a7bkAzhXf1HNzkmjhQ0WMdp3\nEiwSxFyGuQCvMonVC2VeK3FPdwKBgQDH2NYFdWm3ubYcYl"
    "eONCV9x6ZsKuXEBUUq\nlOSAXKAnQLCjpQc4yvlgUupP9Oe99N4jMEC7sJD3CTQqhETk3QAG6C"
    "bhqL1S"
    "ZwQI\nKl79RqYuffYZ7RgmjQSPOrFGAwSHic2Mtwh+G0DV67bx07J4vJn3FKiik/"
    "OfQvOE\n43DoUVpDAwKBgGIUF3F6I66NyaK8dXDSKMA2TpjGUNc7UHaF0D+"
    "4aNtVxt6abm32\n0aRHjM787C5UEPUWPyEIBIdKS1srZLB2+"
    "ZdGNWICxZvKDsEUYtDPz4ct8rVk6vdm\neRRF7zEL8lcKZZVToNxQoIWYgo9nNSGNEL+"
    "G0q3PXVDuE/r5HLCOXlpJAoGAM/"
    "97\nUFhIdYFgnOoJlfqDdsKnzqUqu2ITqQysl6mCEjSGwU2DH4fQcvuf88XpNkesNMOt\nVCiQ"
    "z3YZmKZptpK6GxH0a+qXuAUNy4IazRPA0X/8tKo4Cm/"
    "Oq+W9klXeq9VzIhXN\no+"
    "I7cG2wYCOQ4FFSTjOV3RdZ4CpvajgzfH8OWTkCgYBbv43TqsEOewk7ru/"
    "e61tS\n8YG2XfDQukxHV/dqc4pLUtcehLGH3IQVKj8uYlJQtkyTcXkDcrjJoB5oEmV/"
    "lhN7\nm3pB6dTI/IraaJfeZ0WZm/U7NFhXr7sn1AwX/NM/ms1wYe67uQ+YOqEphPly4Wz1\n8/"
    "Rd4UnOdOG5Ft8YSitVnQ==\n-----END PRIVATE KEY-----\n"; 
  
// ServiceAuth is required.
ServiceAuth sa_auth(timeStatusCB, FIREBASE_CLIENT_EMAIL, FIREBASE_PROJECT_ID, PRIVATE_KEY, 3000 /* expire period in seconds (<= 3600) */);

FirebaseApp app;

#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;

using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client, getNetwork(network));

Firestore::Documents Docs;

AsyncResult aResult_no_callback;

int counter = 0;

unsigned long dataMillis = 0;

void setup()
{

    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
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

    initializeApp(aClient, app, getAuth(sa_auth), aResult_no_callback);

    authHandler();

    app.getApp<Firestore::Documents>(Docs);

    // In case setting the external async result to the sync task (optional)
    // To unset, use unsetAsyncResult().
    aClient.setAsyncResult(aResult_no_callback);
}

void loop()
{
    authHandler();

    Docs.loop();

    if (app.ready() && (millis() - dataMillis > 60000 || dataMillis == 0))
    {
        dataMillis = millis();
        counter++;

        Serial.println("Batch write documents... ");

        String documentPath = "test_collection/test_document_map_value";

        Values::MapValue mapV("name", Values::StringValue("value" + String(counter)));
        mapV.add("count", Values::StringValue(String(counter)));

        Values::MapValue mapV2("key" + String(counter), mapV);

        Document<Values::Value> updateDoc;
        updateDoc.setName(documentPath);
        updateDoc.add("myMap", Values::Value(mapV2));

        Values::MapValue labels;

        Writes writes(Write(DocumentMask("myMap.key" + String(counter)) /* updateMask */, updateDoc, Precondition() /* currentDocument precondition */), Values::MapValue() /* Labels */);

        String documentPath2 = "test_collection/test_document_timestamp";
        String fieldPath = "myTime";
        FieldTransform::SetToServerValue setValue(FieldTransform::REQUEST_TIME);
        FieldTransform::FieldTransform fieldTransforms(fieldPath, setValue);
        DocumentTransform transform(documentPath2, fieldTransforms);

        writes.add(Write(transform, Precondition() /* currentDocument precondition */));

        Serial.println(writes);

        // All Writes, DocumentTransform and Values::xxxx objects can be printed on Serial port

        // You can set the content of write and writes objects directly with write.setContent("your content") and writes.setContent("your content")

        String payload = Docs.batchWrite(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), writes);

        if (aClient.lastError().code() == 0)
            Serial.println(payload);
        else
            printError(aClient.lastError().code(), aClient.lastError().message());
    }
}

void authHandler()
{
    // Blocking authentication handler with timeout
    unsigned long ms = millis();
    while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000)
    {
        // This JWT token process required for ServiceAuth and CustomAuth authentications
        JWT.loop(app.getAuth());
        printResult(aResult_no_callback);
    }
}

void timeStatusCB(uint32_t &ts)
{
  if (time(nullptr) < FIREBASE_DEFAULT_TS)
  {

      configTime(3 * 3600, 0, "pool.ntp.org");
      while (time(nullptr) < FIREBASE_DEFAULT_TS)
      {
          delay(100);
      }
  }
  ts = time(nullptr);
}

void printResult(AsyncResult &aResult)
{
    if (aResult.isEvent())
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    }

    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }

    if (aResult.available())
    {
        Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
    }
}

void printError(int code, const String &msg)
{
    Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}