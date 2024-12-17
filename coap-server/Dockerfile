FROM python:3.13-alpine
LABEL author="IOT students from WS24/25 @ Fachhochschule Dortmund"

USER coap-server
WORKDIR /usr/src/coap-server
COPY * .
RUN pip install --no-cache-dir -r requirements.txt

# Port needed for CoAP protocol
EXPOSE 5683
# EXPOSE 5684

# Run the CoAP Server when the container launches
CMD ["python", "--version"]
