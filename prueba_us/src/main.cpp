#include <Arduino.h>

//atras
#define TRIGGER 5
#define ECHO_B 18
//alante
#define TRIGGER_F 15
#define ECHO_F 19

#define SOUND_SPEED 0.034

long duration;
float distance_back; //distancia atrás
float distance_front; //distancia alante (ultrasonidos)

float get_back()
{
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  
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
  Serial.begin(9600); 
  pinMode(TRIGGER, OUTPUT);
  pinMode(TRIGGER_F, OUTPUT);
  pinMode(ECHO_B, INPUT);
  pinMode(ECHO_F, INPUT);
}

void loop() 
{
  
  // Calculate the distance
  distance_back = get_back();
  distance_front = get_front();


  Serial.print("Atras: ");
  Serial.println(distance_back);
  Serial.print("Alante: ");
  Serial.println(distance_front);
}


