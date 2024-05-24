"""
Module for monitoring and publishing device flag changes using Firestore and MQTT.

This module sets up listeners for Firestore document changes in a collection of devices. 
Whenever a flag within a device is modified, the change is published  to an MQTT broker.
The script uses Firebase Admin SDK for Firestore interactions and  Paho MQTT for
publishing messages.

Dependencies:
- firebase_admin
- paho-mqtt

Functions:
- publish_flag_update(device_id, flag_name, flag_value): Publishes a flag update to the MQTT broker.
- listen_to_device_flags(device_id): Sets up a listener for flag changes in the specified device's flags collection.
- initialize_device_listeners(): Initializes listeners for all devices to monitor flag changes.

Usage:
1. Ensure you have the required dependencies installed.
2. Initialize Firebase with your service account key.
3. Call initialize_device_listeners() to start monitoring devices.
4. The script will run indefinitely, publishing changes to the MQTT broker.
"""


import time
import threading
from firebase_admin import credentials, firestore
import firebase_admin
import paho.mqtt.publish as publish


def publish_flag_update(device_id: str, flag_name: str, flag_value: bool):
    """
    Publish the flag update to the MQTT broker.
    - {device_id}: The ID of the device.
    - {flag_name}: The name of the flag that was updated.
    - {flag_value}: The new value of the flag.
    """

    topic = f"h2overview/{device_id}/{flag_name}"
    payload = str(int(flag_value))

    try:
        print(f"Publishing to {topic} with payload {payload}")
        publish.single(topic, payload, hostname="broker.mqtt-dashboard.com")
    except Exception:
        print(f"Failed to publish to MQTT at {topic}")


def listen_to_device_flags(device_id: str):
    """
    Set up a listener for flag changes in the specified device's flags collection.
    - {device_id}: The ID of the device to listen for flag changes.
    """

    def on_flag_change_snapshot(collection_snapshot, changes, read_time):
        for change in changes:
            if change.type.name == 'ADDED':
                print(f'Device: {device_id} | New flag added: {change.document.id}')
            elif change.type.name == 'REMOVED':
                print(f'Device: {device_id} | Removed flag: {change.document.id}')
            elif change.type.name == 'MODIFIED':
                flag_name = change.document.id
                flag_value = change.document.to_dict().get('value')
                print(f'Device: {device_id} | Modified flag: {flag_name} => {flag_value}')
                publish_flag_update(device_id, flag_name, flag_value)

    try:
        flags_ref = db.collection(f'devices/{device_id}/flags')
        flags_ref.on_snapshot(on_flag_change_snapshot)
    except Exception:
        print(f"Failed to set up listener for {device_id}")


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

# Keep the script running
try:
    while not delete_event.is_set():
        time.sleep(1)
except KeyboardInterrupt:
    print("Stopping the listener...")
    delete_event.set()  # Signal to stop the main loopsd