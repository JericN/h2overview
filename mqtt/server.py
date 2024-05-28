import time
import threading
from firebase_admin import credentials, firestore
import firebase_admin
import paho.mqtt.client as mqtt
import json

# MQTT configuration
MQTT_BROKER = "broker.mqtt-dashboard.com"
MQTT_PORT = 1883
MQTT_KEEPALIVE = 60

# MQTT client setup
mqtt_client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    if rc == 0:
        # Subscribing to multiple topics
        topics = [("h2overview/out/#", 0)]
        client.subscribe(topics)
    else:
        print("Connection failed")

def on_message(client, userdata, msg):
    print(f"Message received on topic {msg.topic} with payload {msg.payload}")

    device_id = msg.topic.split('/')[2]
    flag_name = msg.topic.split('/')[3]
    print(f"Device ID: {device_id}, Flag Name: {flag_name}, Flag Value: {msg.payload}")

    if flag_name == 'is_valve_open' or flag_name == 'is_manual_leak_scan_running' or flag_name == 'is_automated_scan_running':
        try:
            db.collection(f'devices/{device_id}/flags').document(flag_name).set({
                'value': bool(int(msg.payload)),
                'timestamp': firestore.SERVER_TIMESTAMP
            }, merge=True)
            print(f"Flag {flag_name} updated for device {device_id}")
        except Exception:
            print(f"Failed to update flag {flag_name} for device {device_id}")


    elif flag_name == 'manual_results' or flag_name == 'auto_results':
        try:
            db.collection(f'devices/{device_id}/{flag_name}').document().set({
                'leak_result': bool(int(msg.payload)),
                'timestamp': firestore.SERVER_TIMESTAMP
            })
            print(f"Flag {flag_name} updated for device {device_id}")
        except Exception:
            print(f"Failed to update flag {flag_name} for device {device_id}")


    elif flag_name == 'waterflow' or flag_name =='pressure':
        json_data = json.loads(msg.payload)
        try:
            db.collection(f'devices/{device_id}/{flag_name}').document().set({
                'value': json_data['value'],
                'timestamp': json_data['timestamp']
            })
            print(f"Flag {flag_name} updated for device {device_id}")
        except Exception:
            print(f"Failed to update flag {flag_name} for device {device_id}")

            
    else:
        print("Unknown flag name")



mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, MQTT_KEEPALIVE)

def publish_update(device_id: str, doc_id: str, payload: str):
    """
    Publish the update to the MQTT broker.
    - {device_id}: The ID of the device.
    - {doc_name}: The name of the document that was updated.
    - {doc_str}: The new value of the preference.
    """
    topic = f"h2overview/{device_id}/{doc_id}"
    try:
        print(f"Publishing to {topic} with payload {payload}")
        mqtt_client.publish(topic, payload)
    except Exception:
        print(f"Failed to publish to MQTT at {topic}")


def listen_to_device_flags(device_id: str):
    """
    Set up a listener for flag changes in the specified device's flags collection.
    - {device_id}: The ID of the device to listen for flag changes.
    """
    def on_change_snapshot(collection_snapshot, changes, read_time):
        for change in changes:
            if change.type.name == 'ADDED':
                print(f'Device: {device_id} | New document added: {change.document.id}')
            elif change.type.name == 'REMOVED':
                print(f'Device: {device_id} | Removed document: {change.document.id}')
            elif change.type.name == 'MODIFIED':
                doc_id = change.document.id
                value = change.document.to_dict()
                payload = json.dumps(value)
                print(f'Device: {device_id} | Modified document: {pref_name} => {pref_value}')
                publish_update(device_id, doc_id, payload)
    try:
        flags_ref = db.collection(f'devices/{device_id}/flags')
        flags_ref.on_snapshot(on_change_snapshot)
        pref_ref = db.collection(f'devices/{device_id}/preferences')
        pref_ref.on_snapshot(on_change_snapshot)
    except Exception:
        print(f"Failed to set up flag listener for {device_id}")


def initialize_device_listeners():
    """
    Initialize listeners for all devices to monitor flag changes.
    """
    def on_device_snapshot(device_snapshot, changes, read_time):
        for change in changes:
            if change.type.name == 'ADDED':
                device_id = change.document.id
                print(f'New device added: {device_id}')
                listen_to_device_flags(device_id)

    try:
        devices_ref = db.collection('devices')
        devices_ref.on_snapshot(on_device_snapshot)
    except Exception:
        print("Failed to set up device listener")


# Initialize Firebase
cred = credentials.Certificate('../firebase_key.json')
firebase_admin.initialize_app(cred)
db = firestore.client()

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
    print("Stopping the listener...")
    delete_event.set()  # Signal to stop the main loop
    mqtt_client.disconnect()
    mqtt_thread.join()
