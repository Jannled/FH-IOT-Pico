FROM python:3.13-slim
LABEL author="IOT students from WS24/25 @ Fachhochschule Dortmund"

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

RUN groupadd -r coap-server && useradd -r -g coap-server coap-server
USER coap-server
WORKDIR /usr/src/coap-server
COPY * .

# Port needed for CoAP protocol
EXPOSE 5683
# EXPOSE 5684

# Run the CoAP Server when the container launches
CMD ["python", "--version"]
