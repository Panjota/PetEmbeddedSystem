#include <WiFi.h>
#include <PubSubClient.h>

// --- WiFi e MQTT ---
const char* ssid = "JULIE";
const char* password = "Analu2604";
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

// --- Pinos ---
#define TRIG_RES 22
#define ECHO_RES 21
#define TRIG_PET 19
#define ECHO_PET 18
#define BOMBA_RELAY 25
#define NIVEL_POTE 34  // GPIO34 (ADC1) para sensor de nível da tigela

// --- Controle da bomba ---
int tempoBomba = 0;

// --- Visita do pet ---
bool petPresente = false;
unsigned long tempoInicioPresenca = 0;
bool visitaRegistrada = false;

// --- Config do sensor de nível da tigela ---
int nivelTigela = 0;
const int NIVEL_VAZIO = 700;
const int NIVEL_MINIMO = 1100;
const int NIVEL_ALTO = 2000;

// --- Funções WiFi e MQTT ---
void connectToWiFi() {
  Serial.print("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" conectado!");
}

void connectToMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32WaterDispenser")) {
      Serial.println(" conectado!");
      client.subscribe("petwater/dispensar");
    } else {
      Serial.print(" falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando em 5s");
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  int tempo = msg.toInt();
  if (tempo > 0) tempoBomba = tempo;
}

float lerUltrassom(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duracao = pulseIn(echo, HIGH, 25000);
  return duracao * 0.034 / 2;
}

// Função para suavizar a leitura do sensor de nível da tigela
int lerNivelTigelaSuavizado() {
  int total = 0;
  for (int i = 0; i < 10; i++) {
    int leitura = analogRead(NIVEL_POTE);
    // Ignora valores extremos (provável ruído)
    if (leitura > 50 && leitura < 4090) {
      total += leitura;
    }
    delay(5);
  }
  return total / 10;
}

// Função para converter leitura bruta em % nível
int nivelPercentual(int valorSensor) {
  if (valorSensor <= 400) return 0;
  else if (valorSensor <= 1100)
    return map(valorSensor, 400, 1100, 0, 20);
  else if (valorSensor <= 1300)
    return map(valorSensor, 1100, 1300, 20, 50);
  else if (valorSensor <= 1500)
    return map(valorSensor, 1300, 1500, 50, 100);
  else
    return 100;
}

void ativarBomba(int tempo) {
  Serial.println("💧 Liberando água via MQTT...");
  digitalWrite(BOMBA_RELAY, LOW);
  delay(tempo);
  digitalWrite(BOMBA_RELAY, HIGH);
  client.publish("petwater/alerts", "Água liberada");
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_RES, OUTPUT);
  pinMode(ECHO_RES, INPUT);
  pinMode(TRIG_PET, OUTPUT);
  pinMode(ECHO_PET, INPUT);
  pinMode(BOMBA_RELAY, OUTPUT);
  digitalWrite(BOMBA_RELAY, HIGH);
  pinMode(NIVEL_POTE, INPUT);

  connectToWiFi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
  connectToMQTT();
}

unsigned long lastPub = 0;
const unsigned long PUBLISH_INTERVAL = 2000;

void loop() {
  if (!client.connected()) connectToMQTT();
  client.loop();

  if (tempoBomba > 0) {
    ativarBomba(tempoBomba);
    tempoBomba = 0;
  }

  float distReservatorio = lerUltrassom(TRIG_RES, ECHO_RES);
  float distPet = lerUltrassom(TRIG_PET, ECHO_PET);
  int leituraBruta = lerNivelTigelaSuavizado();
  nivelTigela = nivelPercentual(leituraBruta);

  unsigned long now = millis();

  // Publicações periódicas
  if (now - lastPub > PUBLISH_INTERVAL) {
    lastPub = now;

    char buffer[16];
    dtostrf(distReservatorio, 5, 2, buffer);
    client.publish("petwater/reservatorio", buffer);

    dtostrf(distPet, 5, 2, buffer);
    client.publish("petwater/pet", buffer);

    Serial.printf("Reservatório: %.2f cm, Pet: %.2f cm\n", distReservatorio, distPet);
    Serial.print("Nível da tigela (suavizado): ");
    Serial.println(nivelTigela);

    // Publica valor percentual em vez do valor bruto
    itoa(nivelTigela, buffer, 10);
    client.publish("petwater/nivelpote", buffer);  // Agora envia %

    // Publica estado com base no percentual calibrado
    if (nivelTigela == 0) {
      client.publish("petwater/nivelpote/estado", "Vazio");
      client.publish("petwater/alerts", "⚠️ Tigela de água vazia!");
    } else if (nivelTigela > 0 && nivelTigela < 20) {
      client.publish("petwater/nivelpote/estado", "Baixo");
      client.publish("petwater/alerts", "⚠️ Tigela de água com nível crítico!");
    } else if (nivelTigela >= 20 && nivelTigela < 50) {
      client.publish("petwater/nivelpote/estado", "Médio");
    } else {
      client.publish("petwater/nivelpote/estado", "Alto");
    }
    // Alerta de reservatório
    if (distReservatorio > 10.0) {
      Serial.println("⚠️ Nível baixo no reservatório de água!");
      client.publish("petwater/alerts", "⚠️ Reservatório com nível baixo!");
    }

  }

  // Monitoramento de visita do pet
  if (distPet < 20.0) {
    if (!petPresente) {
      tempoInicioPresenca = now;
      petPresente = true;
      visitaRegistrada = false;
    } else if (!visitaRegistrada && now - tempoInicioPresenca > 4000) {
      Serial.println("📌 Visita registrada do pet à tigela de água.");
      client.publish("petwater/visitas", "Visita registrada");
      visitaRegistrada = true;
    }
  } else {
    petPresente = false;
    visitaRegistrada = false;
  }

  delay(10);
}
