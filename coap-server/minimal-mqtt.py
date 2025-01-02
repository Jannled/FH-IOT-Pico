import asyncio
import datetime
import logging
import os
from urllib.parse import urlparse

import aiocoap
import aiocoap.resource as resource
import paho.mqtt.client as mqtt
from aiocoap import Code, Message
from aiocoap.numbers.contentformat import ContentFormat
from paho.mqtt.enums import CallbackAPIVersion

# Set Log Level
logging.basicConfig(
    level=logging.INFO,
    format='[%(asctime)s %(name)s %(levelname)s]: %(message)s'
)
log = logging.getLogger('coap2mqtt')


# Create an MQTT client instance
client = mqtt.Client(callback_api_version=CallbackAPIVersion.VERSION2)
DEFAULT_TOPIC = "sensor/default" # Fallback Topic


class MQTT_Bridge(resource.Resource):
    async def render(self, request: Message):
        topic = DEFAULT_TOPIC
        try:
            requestUri = urlparse(request.get_request_uri())
            topic = str(requestUri.path).removeprefix('/')

            # Drop some topics that seem unreasonable
            if topic == None or len(topic) < 1 or topic in ['/', '()', '#', '$']:
                topic = DEFAULT_TOPIC
        except:
            log.error("Unable to get request uri")

        # Log or process the request        
        log.info(f"Received request: {request.code} on {request.get_request_uri()}")
        log.info(f"Payload: {request.payload.decode('utf-8')}")

        # Publish to MQTT Broker
        try:            
            client.publish(
                topic=topic, 
                payload=request.payload.decode(encoding='utf-8')
            )
            log.info(f"Published topic '{topic}'")
        except Exception as e:
            log.error(f'Failed to publish topic {topic}: "{e}"')

        # Respond with a generic message
        return Message(
            code=Code.CONTENT,
            payload=f"Published {len(request.payload)} bytes".encode()
        )


class Welcome(resource.Resource):

    representations = {
        ContentFormat.TEXT: b"Welcome to the demo server",
        ContentFormat.LINKFORMAT: b"</.well-known/core>,ct=40",
        # ad-hoc for application/xhtml+xml;charset=utf-8
        ContentFormat(65000): b'<html xmlns="http://www.w3.org/1999/xhtml">'
        b"<head><title>aiocoap demo</title></head>"
        b"<body><h1>Welcome to the aiocoap demo server!</h1>"
        b'<ul><li><a href="time">Current time</a></li>'
        b'<li><a href="whoami">Report my network address</a></li>'
        b"</ul></body></html>",
    }

    default_representation = ContentFormat.TEXT

    async def render_get(self, request):
        cf = (
            self.default_representation
            if request.opt.accept is None
            else request.opt.accept
        )

        try:
            return aiocoap.Message(payload=self.representations[cf], content_format=cf)

        except KeyError:
            raise aiocoap.error.UnsupportedContentFormat # type: ignore


class TimeResource(resource.ObservableResource):
    """Example resource that can be observed. The `notify` method keeps
    scheduling itself, and calls `update_state` to trigger sending
    notifications."""

    def __init__(self):
        super().__init__()
        self.handle = None


    def notify(self):
        self.updated_state()
        self.reschedule()


    def reschedule(self):
        self.handle = asyncio.get_event_loop().call_later(5, self.notify)


    def update_observation_count(self, count):
        if count and self.handle is None:
            log.info("Starting the clock")
            self.reschedule()

        if count == 0 and self.handle:
            log.info("Stopping the clock")
            self.handle.cancel()
            self.handle = None


    async def render_get(self, request):
        payload = datetime.datetime.now().strftime("%Y-%m-%d %H:%M").encode("ascii")
        return aiocoap.Message(payload=payload)


class WhoAmI(resource.Resource):
    async def render_get(self, request):
        text = ["Used protocol: %s." % request.remote.scheme]

        text.append("Request came from %s." % request.remote.hostinfo)
        text.append("The server address used %s." % request.remote.hostinfo_local)

        claims = list(request.remote.authenticated_claims)

        if claims:
            text.append(
                "Authenticated claims of the client: %s."
                % ", ".join(repr(c) for c in claims)
            )
        else:
            text.append("No claims authenticated.")

        return aiocoap.Message(content_format=0, payload="\n".join(text).encode("utf8"))


def on_message(client2, userdata, msg):
    log.info(f"Received message: {msg.payload.decode()} on topic {msg.topic}")
    client.publish("motis/echo", msg.payload.decode())


def on_connect(client, userdata, flags, rc, properties):
    if rc == 0:
        log.info("Connected successfully!")
    else:
        log.error(f"Connection failed with code {rc}")

async def loop_coap():
    # Resource tree creation
    root = resource.Site()
    sensor = resource.Site()

    # .well.known/* resources
    # CoRE protocol allows for automatic discovery of all endpoints
    # this server provides
    root.add_resource(
        [".well-known", "core"], resource.WKCResource(root.get_resources_as_linkheader)
    )

    # /* resources
    root.add_resource([], Welcome())
    root.add_resource(["time"], TimeResource())
    root.add_resource(["whoami"], WhoAmI())

    # sensor/* resources
    root.add_resource(['sensor'], sensor)
    sensor.add_resource(['data'], MQTT_Bridge())

    # 5683 is the port for unencrypted coap
    # 5684 is the port for DTLS coap
    await aiocoap.Context.create_server_context(root, bind=('0.0.0.0', 5683))

    # Run forever
    await asyncio.get_running_loop().create_future()


if __name__ == "__main__":
    client.on_connect = on_connect
    client.on_message = on_message

    broker_addr = os.environ.get("MQTT_SERVER", "mqtt")
    broker_port = int(os.environ.get("MQTT_PORT", 1883))
    log.info(f'Connecting to "mqtt://{broker_addr}:{broker_port}" ...')

    try:
        client.username_pw_set(
            username=os.environ.get("MQTT_USER", None), 
            password=os.environ.get("MQTT_PASSWORD", None)
        )
        client.connect(
            broker_addr,
            port=broker_port,
            keepalive=60
        )
    except ConnectionRefusedError as e:
        log.error(f'Connection to MQTT failed: "{e}"')
    except Exception as e:
        log.error(f'Exception in MQTT lib: "{e}"')


    # Start CoAP Server & MQTT Client
    client.loop_start()
    asyncio.run(loop_coap())
