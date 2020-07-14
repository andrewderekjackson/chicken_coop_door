#include <Arduino.h>
#include <EspMQTTClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

const char *ssid = "SSID";
const char *password = "your-password";
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

  // setup OTA Updating
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void getState(const String &payload)
{
  Serial.print("GetState called");
}

void closeDoor()
{
  Serial.print("Closing coop door");

  // disengage the motor while we change state
  digitalWrite(DOOR_MOTOR_ENABLE_PIN, LOW);

  digitalWrite(DOOR_MOTOR_P1_PIN, HIGH);
  digitalWrite(DOOR_MOTOR_P2_PIN, LOW);

  // re-enable the motor
  digitalWrite(DOOR_MOTOR_ENABLE_PIN, HIGH);

  client.publish(TOPIC_STATE, DOOR_STATE_CLOSED, true);
}

void openDoor()
{
  Serial.print("Opening coop door");

  digitalWrite(DOOR_MOTOR_ENABLE_PIN, LOW);
  digitalWrite(DOOR_MOTOR_P1_PIN, LOW);
  digitalWrite(DOOR_MOTOR_P2_PIN, LOW);

  client.publish(TOPIC_STATE, DOOR_STATE_OPEN, true);
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
  ArduinoOTA.handle();
}
