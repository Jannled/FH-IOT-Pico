FROM python:3.13-slim
LABEL title="CoAP 2 MQTT Bridge"
LABEL author="IOT students from WS24/25 @ Fachhochschule Dortmund"

# Cargo (Rust Package Manager) is needed for cbor
# Yeet the apt-cache afterwards, since it is no longer needed
#RUN apt-get update && apt-get install -y cargo nano htop && rm -rf /var/lib/apt/lists/*

# Need to install as root user, since a home directory must exist
COPY requirementsMinimal.txt .
RUN pip install --no-cache-dir -r requirementsMinimal.txt

# Create new user to drop down privilegues
RUN groupadd -r coap-server && useradd -r -g coap-server coap-server
USER coap-server
WORKDIR /usr/src/coap-server
COPY * .

# Port needed for CoAP protocol
EXPOSE 5683
# EXPOSE 5684

# Run the CoAP Server when the container launches
CMD ["python", "-m", "minimal-mqtt"]
