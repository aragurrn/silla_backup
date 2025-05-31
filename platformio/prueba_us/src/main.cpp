#include <Arduino.h>

#define TRIGGER 5
#define ECHO 18

#define SOUND_SPEED 0.034

long duration;
float distance;

float get_atras()
{
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  
  duration = pulseIn(ECHO, HIGH);

  return duration * SOUND_SPEED/2;
}

void setup() 
{
  Serial.begin(9600); 
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
}

void loop() 
{
  
  // Calculate the distance
  distance = get_atras();

  Serial.print("Distancia: ");
  Serial.println(distance);
}


