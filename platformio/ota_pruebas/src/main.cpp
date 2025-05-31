#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

//credenciales wifi
const char* ssid = "SBC";
const char* password = "SBCwifi$";

//websockets
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//borrar luego
bool led_state=0;
int led_pin=3;

//motores
#define ALANTE 15
#define ATRAS 2
#define IZQUIERDA 4
#define DERECHA 16

//pines puente joystick
#define JF 32
#define JB 33
#define JL 34
#define JD 35

//voltaje alimentación
#define VIN 5 //5V
#define VIN_L 3.3 //3.3V para el lateral

//ultrasonidos
#define TRIGGER 5
#define ECHO 18

//velocidad del sonido
#define SOUND_SPEED 0.034

//buzzer
#define BUZZER 22
#define BUZ_MOD 2

//html (lo mismo toca borrarlo)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
    overflow: hidden;
    background-color: #143642;
  }
  body {
    margin: 0;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .button {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #0f8b8d;
    border: none;
    border-radius: 5px;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0,0,0,0);
   }
   /*.button:hover {background-color: #0f8b8d}*/
   .button:active {
     background-color: #0f8b8d;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>ESP movimiento joys</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>Output - GPIO 2</h2>
      <p class="state">state: <span id="state">%STATE%</span></p>
      <p><button id="button" class="button">Toggle</button></p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    if (event.data == "1"){
      state = "ON";
    }
    else{
      state = "OFF";
    }
    document.getElementById('state').innerHTML = state;
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }
</script>
</body>
</html>)rawliteral";

//variables joystick
float jfv=HIGH;
float jbv=HIGH;
float jdv=HIGH;
float jlv=HIGH;

//variables para el ultrasonidos
long duration;
float distance;

//variable estado para el buzzer
int buz_st=0;

//funciones websockets
void notifyClients() 
{
  ws.textAll(String(led_state));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) 
{
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      led_state = !led_state;
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) 
             {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() 
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var)
{
  Serial.println(var);
  if(var == "STATE"){
    if (led_state){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}


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

//lectura y procesamiento del joystick
float get_joystick(int pin)
{
  int raw=analogRead(pin);
  return (raw*VIN)/1023; //o 1023 o 4095
  
}

float get_joystick_l(int pin)
{
  int raw=analogRead(pin);
  return (raw*3.3)/1023; //o 1023 o 4095
  
}

//ultrasonidos distancia
float get_atras()
{ 
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  
  duration = pulseIn(ECHO, HIGH);

  return duration * SOUND_SPEED/2;
}

void setup(void) 
{
  Serial.begin(115200);

  pinMode(led_pin, OUTPUT);
  //digitalWrite(led_pin, LOW);

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
  digitalWrite(ALANTE, HIGH);
  digitalWrite(ATRAS, HIGH);
  digitalWrite(DERECHA, HIGH);
  digitalWrite(IZQUIERDA, HIGH);

  //ultrasonidos
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);

  //buzzer
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW); //apagado por defecto

  WiFi.mode(WIFI_STA);
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

  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) 
  {
    request->send(200, "text/html", index_html, processor);
  });
  
  ElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

}


void loop(void) 
{
  ws.cleanupClients();

  //lecturas del joystick
  jfv=get_joystick(JF);
  jbv=get_joystick(JB);
  jdv=get_joystick_l(JD);
  jlv=get_joystick_l(JL);

  /*Serial.print(jfv);
  Serial.print("//");
  Serial.print(jbv);
  Serial.print("//");
  Serial.print(jdv);
  Serial.print("//");
  Serial.println(jlv);*/

  distance = get_atras();

  Serial.print("Distancia: ");
  Serial.println(distance);

  //procesamiento del joystick
  if(jfv==0.00)
  {
    mover_alante();
  }
  else if(jbv==0.00)
  {
    mover_atras_bz();
  }
  else if(jdv==0.00)
  {
    mover_dcha();
  }
  else if(jlv==0.00)
  {
    mover_izqda();
  }
  else
  {
    parar();
  }

  delay(100);

}