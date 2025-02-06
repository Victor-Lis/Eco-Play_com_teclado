#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>

// Define hardware type, size, and output pins:
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

// Explicação Display
// Display | Arduino
//   VCC   |   5v
//   GND   |   GND
//   DIN   |   11
//   CS    |   10
//   CLK   |   13
#define MAX_DEVICES 8
#define CLK_PIN   18 // Verde
#define DATA_PIN  23 // Amarelo
#define CS_PIN    5  // Roxo

// Criando Objeto "Display" usando a Class MD_PAROLA da própria lib
MD_Parola Display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Função responsável por inicializar display
void initDisplay() {
  Display.begin();
  Display.setIntensity(5);
  Display.setTextEffect(1, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  Display.setTextAlignment(PA_LEFT);
  Display.displayClear();
}

// Setando placar no display
void setPlacar(int meta, int alcancado) {
  //Serial.println("Tampas"+String(alcancado));
  Display.displayClear();
  Display.print("Total    "+String(alcancado));
}

// -------------------------------------------------------------------------------- Display ^

// -------------------------------------------------------------------------------- Teclado v

#include <Keypad.h>

const byte LINHAS = 4;
const byte COLUNAS = 4;
char teclas[LINHAS][COLUNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pinosLinhas[LINHAS] = {4, 14, 12, 13};  // Pinos das linhas
byte pinosColunas[COLUNAS] = {25, 26, 27, 32};  // Pinos das colunas

Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

unsigned long ultimoTempo = 0;

const char* curso = "DS";  // Agora é um ponteiro para string
char ano = 1;            // Mantém como char
const char* periodo = "tarde";  // Ponteiro para string

void setCurso(char tecla) {
   if(tecla == '4') {
    curso = "ADM";
    if (periodo == "noite") periodo = "tarde";
   }
   if(tecla == '5') {
    curso = "DG";
    periodo = "noite";
    ano = 1;
   }
   if(tecla == '6') {
    curso = "DS";
    if (periodo == "noite") periodo = "tarde";
    if (ano == 3 && periodo == "tarde") ano = 2;
   }
   if(tecla == '7') {
    curso = "MKT";
    if (periodo == "noite") periodo = "tarde";
   }
   if(tecla == '8') {
    curso = "RH";
    periodo = "noite";
   }
}

void setAno(char tecla) {
   if(tecla == '1') ano = 1;
   if(tecla == '2' && curso != "DG") ano = 2;
   if(tecla == '3' && ((curso != "DS") || (curso == "DS" && periodo == "manha")) && curso != "DG") ano = 3;
}

void setPeriodo(char tecla) {
  if(tecla == 'A' && curso != "DG" && curso != "RH") periodo = "manha";
  if(tecla == 'B' && curso != "DG" && curso != "RH") periodo = "tarde"; 
  if(tecla == 'C' && curso == "DG" || curso == "RH") periodo = "noite"; 
}

void showAll() {
  Display.displayClear();
  String messageToPrint = "";
  
  if (ano == 1) messageToPrint += "1";
  if (ano == 2) messageToPrint += "2";
  if (ano == 3) messageToPrint += "3";
  
  if(String(curso).length() == 2) { 
    if(ano == 1) messageToPrint += "-  ";
    if(ano != 1) messageToPrint += "- ";
  } else { 
    if(ano == 1) messageToPrint += "- ";
    if(ano != 1) messageToPrint += "-";
  }

  messageToPrint += String(curso);

  if(ano == 1) messageToPrint += "  ";
  if((ano != 1) && (String(curso).length() == 2)) messageToPrint += "  ";
  if((ano != 1) && (String(curso).length() == 3)) messageToPrint += " ";
  
  messageToPrint += String(periodo);
  
  Display.print(messageToPrint);
}

void setAll(){
  char tecla = teclado.getKey();
  if (tecla) {
    ultimoTempo = millis();
    setAno(tecla); 
    setCurso(tecla);
    setPeriodo(tecla);
  }
}

// -------------------------------------------------------------------------------- Wifi v

#include <WiFi.h>
#include <HTTPClient.h>;
#include <ArduinoJson.h>;

const char* ssid = "Oalis";
const char* password = "oalis636807";

void wifiConfig() {
  WiFi.begin(ssid, password);
  Serial.println("Configuring access point...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("Conectado");
  Serial.println(WiFi.localIP());
  Serial.println("");
}

void saveCap() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("+");

    HTTPClient client;
    String apiUrl = "https://eco-play.vercel.app/api/tampinha?";
    apiUrl += "curso="+String(curso);
    if (ano == 1) apiUrl += "&ano=1";
    if (ano == 2) apiUrl += "&ano=2";
    if (ano == 3) apiUrl += "&ano=3";
    apiUrl += "&periodo="+String(periodo);
    apiUrl += "&senha=etec147";

    Serial.println(apiUrl);
    client.begin(apiUrl);

    int httpCode = client.GET();

    client.addHeader("Connection", "close");

    if (httpCode > 0) {
      String payload = client.getString();
      Serial.println("\nStatuscode: " + String(httpCode));
      Serial.println(payload);
    } else {
      Serial.println("Error on HTTP Request");
      Serial.println("\nStatuscode: " + String(httpCode));
    }
  } else {
    delay(100);
  }
}

// -------------------------------------------------------------------------------- Sensor v

// Explicação Sensor

//   Sensor  | Arduino
//   Marrom  |   5v
//   Azul    |   GND
//   Preto   |   Data

const int sensor = 34; // Branco;
//const int sensor = 2;

boolean hasMoviment() {
  //return (digitalRead(sensor) == LOW);
  Serial.println(analogRead(sensor));
  return (analogRead(sensor) < 3000);
}

// -------------------------------------------------------------------------------- Variáveis de "Software" v

int meta = 10000;
int alcancado = 0;

const int intervalo = 5000;

void verifyGoal() {
  if (alcancado >= meta) {
    alcancado = 0;
  }
}

void increment() {
  if (hasMoviment()) {
    alcancado++;

    saveCap();

    verifyGoal();
    
    setPlacar(meta, alcancado);
  } else if(millis() - ultimoTempo < intervalo) {
    showAll();
  } else if(millis() - ultimoTempo >= intervalo) {
    setPlacar(meta, alcancado);
  }
}

// -------------------------------------------------------------------------------- Main v

void setup() {
  Serial.begin(115200);

  initDisplay();

  wifiConfig();

  pinMode(sensor, INPUT);
}

void loop() {
  increment();
  setAll();
}
