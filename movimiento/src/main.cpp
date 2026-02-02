#include <Arduino.h>

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

//voltaje alimentación
#define VIN 3.3 //5V, 3.3V
#define VIN_L 1.8 //3.3V, 1.8V para el lateral

//buzzer
#define BUZZER 22
#define BUZ_MOD 2

//variables joystick
int jfv=HIGH;
int jbv=HIGH;
int jdv=HIGH;
int jlv=HIGH;

//variable estado para el buzzer
int buz_st=0;

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
  buz_st=0;
  digitalWrite(BUZZER, LOW);
  digitalWrite(ALANTE, HIGH);
  digitalWrite(ATRAS, HIGH);
  digitalWrite(DERECHA, HIGH);
  digitalWrite(IZQUIERDA, HIGH);
}

void setup(void) 
{
  setCpuFrequencyMhz(240);
  Serial.begin(115200);

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

  //por defecto en parado
  parar();


  //buzzer
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW); //apagado por defecto

}


void loop(void) 
{
  //lecturas del joystick
  jfv=digitalRead(JF);
  jbv=digitalRead(JB);
  jdv=digitalRead(JD);
  jlv=digitalRead(JL);

  Serial.print(jfv);
  Serial.print("//");
  Serial.print(jbv);
  Serial.print("//");
  Serial.print(jdv);
  Serial.print("//");
  Serial.println(jlv);

  //procesamiento del joystick
  if(jfv==0)
  {
    mover_alante();
  }
  else if(jbv==0)
  {
    mover_atras();
    //mover_atras_bz();
  }
  else if(jdv==0)
  {
    mover_dcha();
  }
  else if(jlv==0)
  {
    mover_izqda();
  }
  else
  {
    parar();
  }

  delay(100);

}