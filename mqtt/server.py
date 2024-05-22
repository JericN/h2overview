import paho.mqtt.publish as publish
import firebase_admin
from firebase_admin import credentials
from firebase_admin import firestore
import threading
import time
import json

# initialize firebase
cred = credentials.Certificate('../firebase_key.json')
app = firebase_admin.initialize_app(cred)
db = firestore.client()

# Create an Event for notifying main thread.
delete_done = threading.Event()

def on_database_change(change):
    data = change.document.to_dict()
    endpoint = "h2overview/devices/"+data.get("serial")
    payload = json.dumps(data.get("flags"))

    print(f"Publishing to {endpoint} with payload {payload}")
    publish.single(endpoint, payload, hostname="broker.mqtt-dashboard.com")

# Create a callback on_snapshot function to capture changes
def on_snapshot(col_snapshot, changes, read_time):
    print("Callback received query snapshot.")
    for change in changes:
        if change.type.name == "ADDED":
            print(f"New: {change.document.to_dict()}")
        elif change.type.name == "MODIFIED":
            print(f"Modified: {change.document.to_dict()}")
            on_database_change(change)
        elif change.type.name == "REMOVED":
            print(f"Removed: {change.document.to_dict()}")
            delete_done.set()

col_query = db.collection("devices")

# Watch the collection query
query_watch = col_query.on_snapshot(on_snapshot)

while True:
    print('', end='', flush=True)
    time.sleep(1)