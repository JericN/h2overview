import paho.mqtt.client as mqtt

def on_subscribe(client, userdata, mid, reason_code_list, properties):
    for reason_code in reason_code_list:
        if reason_code.is_failure:
            print(f"Broker rejected you subscription: {reason_code}")
        else:
            print(f"Broker granted the following QoS: {reason_code.value}")

def on_unsubscribe(client, userdata, mid, reason_code_list, properties):
    if len(reason_code_list) == 0 or not reason_code_list[0].is_failure:
        print("unsubscribe succeeded (if SUBACK is received in MQTTv3 it success)")
    else:
        print(f"Broker replied with failure: {reason_code_list[0]}")
    client.disconnect()

def on_message(client, userdata, message):
    data = message.payload.decode('utf-8')
    print(f"Received message '{data}' on topic '{message.topic}'")

def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code.is_failure:
        print(f"Failed to connect: {reason_code}. loop_forever() will retry connection")
    else:
        client.subscribe("h2overview/devices/3HcclYxwnFsrlTODAeKa")

mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.on_connect = on_connect
mqttc.on_message = on_message
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

mqttc.user_data_set([])
mqttc.connect("broker.mqtt-dashboard.com")
mqttc.loop_forever()
print(f"Received the following message: {mqttc.user_data_get()}")