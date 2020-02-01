#include <Arduino.h>
#include <EspMQTTClient.h>
#include <ESP8266WiFi.h>

const char *ssid = "HAPPYNET";
const char *password = "***REMOVED***";
const char *mqttServer = "10.1.1.150";
const int mqttPort = 1883;
const char *mqttUser = "";
const char *mqttPassword = "";
const char *mqttClient = "coop";

const char *TOPIC_STATE = "coop/state";
const char *TOPIC_COMMAND = "coop/set";

const char *DOOR_STATE_OPEN = "open";
const char *DOOR_STATE_CLOSED = "closed";

const char *DOOR_COMMAND_OPEN = "open";
const char *DOOR_COMMAND_CLOSE = "close";

EspMQTTClient client(
    ssid,
    password,
    mqttServer,   // MQTT Broker server ip
    mqttUser,     // Can be omitted if not needed
    mqttPassword, // Can be omitted if not needed
    mqttClient,
    mqttPort // Client name that uniquely identify your device
);

int DOOR_MOTOR_ENABLE_PIN = D1;
int DOOR_MOTOR_P1_PIN = D2;
int DOOR_MOTOR_P2_PIN = D3;

void setup(void)
{
  pinMode(DOOR_MOTOR_ENABLE_PIN, OUTPUT);
  pinMode(DOOR_MOTOR_P1_PIN, OUTPUT);
  pinMode(DOOR_MOTOR_P2_PIN, OUTPUT);

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
}

void getState(const String &payload)
{
  Serial.print("GetState called");
}

void openDoor()
{
  Serial.print("Opening coop door");

  // disengage the motor while we change state
  digitalWrite(DOOR_MOTOR_ENABLE_PIN, LOW);

  digitalWrite(DOOR_MOTOR_P1_PIN, HIGH);
  digitalWrite(DOOR_MOTOR_P2_PIN, LOW);

  // re-enable the motor
  digitalWrite(DOOR_MOTOR_ENABLE_PIN, HIGH);

  client.publish(TOPIC_STATE, DOOR_STATE_OPEN, true);
}

void closeDoor()
{
  Serial.print("Closing coop door");

  digitalWrite(DOOR_MOTOR_ENABLE_PIN, LOW);
  digitalWrite(DOOR_MOTOR_P1_PIN, LOW);
  digitalWrite(DOOR_MOTOR_P2_PIN, LOW);

  client.publish(TOPIC_STATE, DOOR_STATE_CLOSED, true);
}

void handleCommand(const String &payload)
{
  if (payload == DOOR_COMMAND_OPEN)
  {
    openDoor();
  }
  else if (payload == DOOR_COMMAND_CLOSE)
  {
    closeDoor();
  }
  else
  {
    Serial.print("Unknown command received");
  }
}

void onConnectionEstablished()
{
  Serial.println("MQTT Connection established.");
  client.subscribe(TOPIC_COMMAND, handleCommand);
}

void loop(void)
{
  client.loop();
}
