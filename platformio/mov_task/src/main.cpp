#include <Arduino.h>
#include <WiFi.h>
#include "esp_mac.h"

//FREERTOS

//task managers
TaskHandle_t task_joystick_handle = NULL;

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

void task_joystick(void *parameter)
{
  const int umbral = 100.00;

  for(;;)
  {
    
    int jfv = digitalRead(JF);
    int jbv = digitalRead(JB);
    int jlv = digitalRead(JL);
    int jdv = digitalRead(JD);

    Serial.print(jfv);
    Serial.print("//");
    Serial.print(jbv);
    Serial.print("//");
    Serial.print(jdv);
    Serial.print("//");
    Serial.println(jlv);

    if (jfv==0 || jbv==0 || jlv==0 || jdv==0)
    {
      joystick = true; 
      Serial.println("Joystick activo");

      while (jfv==0 || jbv==0 || jlv==0 || jdv==0)
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
  
  
  //MOTORES
  pinMode(ALANTE, OUTPUT);
  pinMode(ATRAS, OUTPUT);
  pinMode(DERECHA, OUTPUT);
  pinMode(IZQUIERDA, OUTPUT);

  //joystick
  pinMode(JF, INPUT_PULLUP);
  pinMode(JB, INPUT_PULLUP);
  pinMode(JD, INPUT_PULLUP);
  pinMode(JL, INPUT_PULLUP);

  //por defecto en parado
  parar();
  countdown();

  //tareas
  xTaskCreatePinnedToCore(task_joystick, "joystick", CONFIG_ARDUINO_LOOP_STACK_SIZE, NULL, 4, &task_joystick_handle, 1);
  Serial.println("tarea3");
  
}

void loop()
{

}
