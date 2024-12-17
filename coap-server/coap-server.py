import asyncio
import time
from aiocoap import *
from paho.mqtt import client as mqtt_client

# TODO WARNING ChatGPT generated

#Setup MQTT
broker = 'mqtt.eclipse.org' #Use your actual MQTT broker.
client_id = 'coap-mqtt-gateway'
username = '<your_username>'
password = '<your_password>'

mqtt_client = mqtt_client.Client(client_id)
mqtt_client.username_pw_set(username, password)
mqtt_client.connect(broker)

async def main():
    protocol = await Context.create_client_context()
    while True: #We are repeating this in a loop to consistently get the temperature and send to MQTT
        request = Message(code=GET)
        request.set_request_uri('coap://deviceip/temperature') #Use actual IP of the device hosting the CoAP server
        response = await protocol.request(request).response
        temperature = response.payload.decode('utf8')

        #send the data to MQTT
        result = mqtt_client.publish("temperature-topic", temperature)

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(main())
