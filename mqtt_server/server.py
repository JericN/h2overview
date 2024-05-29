import time
import threading
import json
from datetime import datetime
from firebase_admin import credentials, firestore
import firebase_admin
import paho.mqtt.client as mqtt

# MQTT configuration
MQTT_BROKER = "broker.mqtt-dashboard.com"
MQTT_PORT = 1883
MQTT_KEEPALIVE = 600

# Initialize Firebase
cred = credentials.Certificate("../firebase_key.json")
firebase_admin.initialize_app(cred)
db = firestore.client()

# MQTT client setup
mqtt_client = mqtt.Client()


# ============================================================================
# ============================== MQTT Callbacks ==============================
# ============================================================================


def on_connect(client, userdata, flags, rc):
    """
    Callback function for when the MQTT client connects to the broker.
    Subscribes to the required topics upon successful connection.
    """
    print(f"[LOGS] Connected with result code {rc}")
    if rc == 0:
        topics = [("h2overview/out/#", 0)]
        client.subscribe(topics)
    else:
        print("[ERROR] MQTT Connection failed")


def update_flag(device_id, flag_name, payload):
    """
    Updates the flag value in Firestore for the given device.
    """
    try:
        db.collection(f"devices/{device_id}/flags").document(flag_name).set(
            {
                "value": bool(int(payload)),
                "timestamp": firestore.SERVER_TIMESTAMP,
            },
            merge=True,
        )
        print(
            f"[LOGS] Flag {flag_name} updated for device {device_id} to {bool(int(payload))}"
        )
    except Exception as e:
        print(f"[ERROR] Failed to update flag {flag_name} for device {device_id}: {e}")


def update_scan_results(device_id, flag_name, payload):
    """
    Updates the scan results in Firestore for the given device.
    """
    try:
        db.collection(f"devices/{device_id}/{flag_name}").document().set(
            {"leak_result": str(payload), "timestamp": firestore.SERVER_TIMESTAMP}
        )
        print(f"[LOGS] Scan results {flag_name} updated for device {device_id}")
    except Exception as e:
        print(
            f"[ERROR] Failed to update scan results {flag_name} for device {device_id}: {e}"
        )


def update_sensor_reading(device_id, flag_name, payload):
    """
    Updates the sensor reading in Firestore for the given device.
    """
    try:
        json_data = json.loads(payload)
        db.collection(f"devices/{device_id}/{flag_name}").document().set(
            {
                "value": json_data["value"],
                "timestamp": datetime.fromtimestamp(json_data["timestamp"]),
            }
        )
        print(f"[LOGS] Sensor reading {flag_name} updated for device {device_id}")
    except Exception as e:
        print(
            f"[ERROR] Failed to update sensor reading {flag_name} for device {device_id}: {e}"
        )


def on_message(client, userdata, msg):
    """
    Callback function for when a message is received on a subscribed topic.
    Updates the Firestore database based on the message topic and payload.
    """
    print(f"[LOGS] Message received on topic {msg.topic} with payload {msg.payload}")
    device_id = msg.topic.split("/")[2]
    flag_name = msg.topic.split("/")[3]

    if flag_name in [
        "is_alive",
        "is_valve_open",
        "is_manual_leak_scan_running",
        "is_automated_scan_running",
    ]:
        update_flag(device_id, flag_name, msg.payload)
    elif flag_name in ["manual_results", "auto_results"]:
        update_scan_results(device_id, flag_name, msg.payload)
    elif flag_name in ["waterflow", "pressure"]:
        update_sensor_reading(device_id, flag_name, msg.payload)
    else:
        print(f"[ERROR] Unknown flag name: {flag_name}")


# ============================================================================
# ================================ MQTT Listeners ============================
# ============================================================================


def publish_update(device_id: str, doc_id: str, payload: str):
    """
    Publish the update to the MQTT broker.
    """
    topic = f"h2overview/{device_id}/{doc_id}"
    try:
        print(f"[LOGS] Publishing to {topic} with payload {payload}")
        mqtt_client.publish(topic, payload)
    except Exception as e:
        print(f"[ERROR] Failed to publish to MQTT at {topic}: {e}")

def listen_to_device_flags(device_id: str):
    """
    Set up a listener for flag changes in the specified device's flags collection.
    """
    def on_change_snapshot(collection_snapshot, changes, read_time):
        for change in changes:
            if change.type.name == "ADDED":
                print(f"[LOGS] Device: {device_id} | New document added: {change.document.id}")
            elif change.type.name == "REMOVED":
                print(f"[LOGS] Device: {device_id} | Removed document: {change.document.id}")
            elif change.type.name == "MODIFIED":
                doc_id = change.document.id
                value = change.document.to_dict()
                print(f"[LOGS] Device: {device_id} | Modified document: {doc_id} => {value}")
                if doc_id == "is_alive" and value["value"]:
                    continue
                value["timestamp"] = value["timestamp"].isoformat()
                payload = json.dumps(value)
                publish_update(device_id, doc_id, payload)

    try:
        flags_ref = db.collection(f"devices/{device_id}/flags")
        flags_ref.on_snapshot(on_change_snapshot)
        pref_ref = db.collection(f"devices/{device_id}/preferences")
        pref_ref.on_snapshot(on_change_snapshot)
    except Exception as e:
        print(f"[ERROR] Failed to set up  listener for {device_id}: {e}")


def initialize_device_listeners():
    """
    Initialize listeners for all devices to monitor flag changes.
    """
    def on_device_snapshot(device_snapshot, changes, read_time):
        for change in changes:
            if change.type.name == "ADDED":
                device_id = change.document.id
                print(f"[LOGS] New device added: {device_id}")
                listen_to_device_flags(device_id)

    try:
        devices_ref = db.collection("devices")
        devices_ref.on_snapshot(on_device_snapshot)
    except Exception as e:
        print(f"[ERROR] Failed to set up device listener: {e}")


# Assign MQTT callbacks
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

# Connect to the MQTT broker
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, MQTT_KEEPALIVE)

# Start listening to all device flag changes
initialize_device_listeners()

# Create an event for notifying the main thread
delete_event = threading.Event()

# Start MQTT loop
mqtt_thread = threading.Thread(target=mqtt_client.loop_forever)
mqtt_thread.start()

# Keep the script running
try:
    while not delete_event.is_set():
        time.sleep(1)
except KeyboardInterrupt:
    print("[LOGS] Stopping the listener...")
    delete_event.set()
    mqtt_client.disconnect()
    mqtt_thread.join()
