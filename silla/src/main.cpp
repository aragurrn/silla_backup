#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <PubSubClient.h>
#include "esp_mac.h"

//FREERTOS

//task managers
TaskHandle_t task_sensores_handle = NULL;
TaskHandle_t task_joystick_handle = NULL;
TaskHandle_t task_wifi_mqtt_handle = NULL;

//motores
#define ALANTE 23
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

//buzzer
#define BUZZER 22
#define BUZ_MOD 2

//defines hcsr04
#define SOUND_SPEED 0.034

//variabes wifi
const char* ssid = "SBC";
const char* password = "SBCwifi$";

//variables servidor mqtt
const char* mqtt_server = "iot.etsisi.upm.es"; 
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value=0;

//ota
AsyncWebServer server(80);

//variables hcsr04
long duration;
float distance_back; //distancia atrás
float distance_front; //distancia alante (ultrasonidos)

//variable estado para el buzzer
int buz_st=0;

//variable de estado de joystick
bool joystick = false;

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

void mover_atras_bz()
{
  buz_st++;
  if ((buz_st%BUZ_MOD)==1)
  {
    digitalWrite(BUZZER, HIGH);
  }
  else if ((buz_st%BUZ_MOD)==0)
  {
    digitalWrite(BUZZER, LOW);
    buz_st=0;
  }
  mover_atras();
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

//wifi
void setup_wifi()
{
  delay(10);
  WiFi.mode(WIFI_STA);
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

  //elegantota
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "is this the real life?");
  });

  server.begin();
  Serial.println("HTTP server started");

  ElegantOTA.begin(&server);    // Start ElegantOTA

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

//TAREAS

//tarea sensores
void task_sensores(void *parameter)
{
  for(;;)
  {
    distance_back = get_back();
    distance_front = get_front();

    char atras_string[16];
    dtostrf(distance_back, 6, 4, atras_string);

    char alante_string[16];
    dtostrf(distance_front, 6, 4, alante_string);

    client.publish("esp32/silla/distancia/atras", atras_string, sizeof(atras_string));
    client.publish("esp32/silla/distancia/alante", alante_string, sizeof(alante_string));

    if(distance_back<=50.00 || distance_front<=50.00)
    {
      parar();
    }

    vTaskDelay(20/portTICK_PERIOD_MS);
  }
}

void task_joystick(void *parameter)
{

  for(;;)
  {
    
    int jfv=digitalRead(JF);
    int jbv=digitalRead(JB);
    int jdv=digitalRead(JD);
    int jlv=digitalRead(JL);

    Serial.print(jfv);
    Serial.print("//");
    Serial.print(jbv);
    Serial.print("//");
    Serial.print(jdv);
    Serial.print("//");
    Serial.println(jlv);

    if (jfv==0 || jbv==0 || jdv==0 || jlv==0)
    {
      joystick = true; 
      Serial.println("Joystick activo");

      while (jfv==0 || jbv==0 || jdv==0 || jlv==0)
      {
        if (jfv==0) mover_alante();
        else if (jbv==0) mover_atras();
        else if (jlv==0) mover_izqda();
        else if (jdv==0) mover_dcha();

        vTaskDelay(20 / portTICK_PERIOD_MS); 
            
        jfv=digitalRead(JF);
        jbv=digitalRead(JB);
        jdv=digitalRead(JD);
        jlv=digitalRead(JL);
      }

      
      parar();
      Serial.println("Joystick soltado. Bloqueo de seguridad (2s)...");
      
      vTaskDelay(2000 / portTICK_PERIOD_MS); 
      
      joystick = false;
      Serial.println("Control devuelto");
    }

    
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

//tarea wifi mqtt
void task_wifi_mqtt(void *parameter)
{
  for(;;)
  {
    if (!client.connected())
    {
      reconnect();
    }
    client.loop();
    ElegantOTA.loop();

    vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

//secuencia de arranque
void countdown()
{
  Serial.println("5");
  delay(1000);
  Serial.println("4");
  delay(1000);
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);
  Serial.println("0");
}


void setup()
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //buzzer
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW); //apagado por defecto
  
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
  countdown();

  //tareas
  xTaskCreatePinnedToCore(task_sensores, "sensores", CONFIG_ARDUINO_LOOP_STACK_SIZE, NULL, 3, &task_sensores_handle, 1);
  Serial.println("tarea1");
  xTaskCreatePinnedToCore(task_wifi_mqtt, "wifi_mqtt", CONFIG_ARDUINO_LOOP_STACK_SIZE, NULL, 3, &task_wifi_mqtt_handle, 1);
  Serial.println("tarea2");
  xTaskCreatePinnedToCore(task_joystick, "joystick", CONFIG_ARDUINO_LOOP_STACK_SIZE, NULL, 4, &task_joystick_handle, 1);
  Serial.println("tarea3");
}

void loop()
{

}