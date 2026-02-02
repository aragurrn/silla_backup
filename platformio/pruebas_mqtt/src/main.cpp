#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

//motores
#define ALANTE 27
#define ATRAS 2
#define IZQUIERDA 4
#define DERECHA 16

//pines puente joystick
#define JF 32
#define JB 33
#define JL 35
#define JD 34

//pines hcsr04
//atras
#define TRIGGER_B 5
#define ECHO_B 18
//alante
#define TRIGGER_F 15
#define ECHO_F 19

//defines hcsr04
#define SOUND_SPEED 0.034

//variabes wifi
const char* ssid = "SBC";
const char* password = "SBCwifi$";

//variables servidor mqtt
const char* mqtt_server = "iot.etsisi.upm.es"; //cambiar a servidor real
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value=0;

//variables joystick
int jfv=HIGH;
int jbv=HIGH;
int jdv=HIGH;
int jlv=HIGH;

//variables hcsr04
long duration;
float distance_back; //distancia atrás
float distance_front; //distancia alante (ultrasonidos)

//control de movimiento nativo
void mover_alante()
{
  digitalWrite(ALANTE, LOW);
  digitalWrite(ATRAS, HIGH);
  digitalWrite(DERECHA, HIGH);
  digitalWrite(IZQUIERDA, HIGH);
}

void mover_atras()
{
  digitalWrite(ALANTE, HIGH);
  digitalWrite(ATRAS, LOW);
  digitalWrite(DERECHA, HIGH);
  digitalWrite(IZQUIERDA, HIGH);
}

void mover_dcha()
{
  digitalWrite(ALANTE, HIGH);
  digitalWrite(ATRAS, HIGH);
  digitalWrite(DERECHA, HIGH);
  digitalWrite(IZQUIERDA, LOW);
}

void mover_izqda()
{
  digitalWrite(ALANTE, HIGH);
  digitalWrite(ATRAS, HIGH);
  digitalWrite(DERECHA, LOW);
  digitalWrite(IZQUIERDA, HIGH);
}

void parar()
{
  digitalWrite(ALANTE, HIGH);
  digitalWrite(ATRAS, HIGH);
  digitalWrite(DERECHA, HIGH);
  digitalWrite(IZQUIERDA, HIGH);
}


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
    if(messageTemp == "{\"movimiento\":\"alante\"}")
    {
      Serial.println("alante");
      mover_alante();
    }
    else if (messageTemp=="{\"movimiento\":\"atras\"}")
    {
      Serial.println("atras");
      mover_atras();
    }
    else if (messageTemp=="{\"movimiento\":\"derecha\"}")
    {
      Serial.println("derecha");
      mover_dcha();
    }
    else if (messageTemp=="{\"movimiento\":\"izquierda\"}")
    {
      Serial.println("izquerda");
      mover_izqda();
    }
    else if(messageTemp == "{\"movimiento\":\"parar\"}")
    {
      Serial.println("parar");
      parar();
    }
    else //default: parar
    {
      Serial.println("parar");
      parar();
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
    if (client.connect("ESP32client"))
    {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/silla/movimiento"); //motores
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

//ultrasonidos
float get_back()
{
  digitalWrite(TRIGGER_B, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIGGER_B, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_B, LOW);
  
  duration = pulseIn(ECHO_B, HIGH);

  return duration * SOUND_SPEED/2;
}

float get_front()
{
  digitalWrite(TRIGGER_F, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIGGER_F, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_F, LOW);
  
  duration = pulseIn(ECHO_F, HIGH);

  return duration * SOUND_SPEED/2;
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  //MOTORES
  pinMode(ALANTE, OUTPUT);
  pinMode(ATRAS, OUTPUT);
  pinMode(DERECHA, OUTPUT);
  pinMode(IZQUIERDA, OUTPUT);

  //joystick
  pinMode(JF, INPUT);
  pinMode(JB, INPUT);
  pinMode(JD, INPUT);
  pinMode(JL, INPUT);

  //ultrasonidos
  pinMode(TRIGGER_B, OUTPUT);
  pinMode(TRIGGER_F, OUTPUT);
  pinMode(ECHO_B, INPUT);
  pinMode(ECHO_F, INPUT);

  //por defecto en parado
  parar();
}

//intervalos de tiempo para publicar y que no sature el servidor -> 0.5s
unsigned long anterior = 0;
unsigned long intervalo = 500;

void loop()
{
  //gestion wifi y mqtt
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  //lecturas del joystick
  jfv=digitalRead(JF);
  jbv=digitalRead(JB);
  jdv=digitalRead(JD);
  jlv=digitalRead(JL);

  ///lecturas de sensores y publicación en MQTT, tarea periódica
  unsigned long actual=millis();
  if (actual-anterior>=intervalo)
  {
    anterior=actual;
    distance_back = get_back();
    distance_front = get_front();

    char atras_string[16];
    dtostrf(distance_back, 6, 4, atras_string);

    char alante_string[16];
    dtostrf(distance_front, 6, 4, alante_string);

    client.publish("esp32/silla/distancia/atras", atras_string, sizeof(atras_string));
    client.publish("esp32/silla/distancia/alante", alante_string, sizeof(alante_string));

  }

  delay(100);
}
