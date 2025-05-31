#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

//pines hcsr04
#define TRIGGER 5
#define ECHO 18

//defines hcsr04
#define SOUND_SPEED 0.034

//variabes wifi
const char* ssid = "SBC";
const char* password = "SBCwifi$";

//variables servidor mqtt
const char* mqtt_server = "192.168.1.116:1884"; //cambiar a servidor real

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value=0;

//variables hcsr04
long duration;
float distance;

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


//recibimos mensaje de topic
void callback(char* topic, byte* message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if (String(topic) == "esp32/silla/movimiento")
  {
    Serial.print("Changing output to ");
    if(messageTemp == "alante")
    {
      Serial.println("alante");
      //mensaje motor
    }
    else if (messageTemp=="atras")
    {
      Serial.println("atras");
    }
    else if (messageTemp=="derecha")
    {
      Serial.println("derecha");
    }
    else if (messageTemp=="izquierda")
    {
      Serial.println("izquerda");
    }
    else if(messageTemp == "parar")
    {
      Serial.println("parar");
      //mensaje motor
    }
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/silla/movimiento"); //motores
      client.subscribe("esp32/silla/distancia/atras"); //distancia atrás
    } 
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  //gestion wifi y mqtt
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  //lecturas de sensores

  client.publish("esp32/silla/distancia/alante", "100");
  client.publish("esp32/silla/diatancia/detras", "100");
}
