#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// --- WiFi e MQTT ---
const char* ssid = "JULIE";
const char* password = "Analu2604";
const char* mqttServer = "mqtt.eclipseprojects.io";
const int mqttPort = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

// --- Pinos ---
#define TRIG_RES 22
#define ECHO_RES 21
#define TRIG_PET 19
#define ECHO_PET 18
#define SERVO_PIN 23
#define SERVO_RELAY 26 
#define BUZZER_PIN 27 

// --- Servo e lógica ---
Servo servoMotor;
int servoDirection = 0;
int dispenseTime = 0;

// --- Nível mínimo ---
const float LIMITE_BAIXO_RACAO = 15.0;
enum NivelRacao { RACAO_OK, RACAO_BAIXO };
NivelRacao estadoAnterior = RACAO_OK;

// --- Funções ---
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
    if (client.connect("ESP32PetFeeder")) {
      Serial.println(" conectado!");
      client.subscribe("petfeeder/dispensar");
    } else {
      Serial.print(" falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5s");
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  int tempo = msg.toInt();
  if (tempo > 0) {
    dispenseTime = tempo;
  }
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

void dispenseFood(int tempo) {
  Serial.println("🔄 Dispensando ração via MQTT...");
  digitalWrite(SERVO_RELAY, LOW);
  delay(500);
  servoMotor.write(servoDirection);
  delay(tempo);
  servoMotor.write(90);
  digitalWrite(SERVO_RELAY, HIGH);
  servoDirection = (servoDirection == 0) ? 180 : 0;
  
  tone(BUZZER_PIN, 1000, 1500); 
  delay(500); 
  noTone(BUZZER_PIN); 
  delay(500); 

  client.publish("petfeeder/alerts", "Ração dispensada");
}


// --- Setup e Loop ---
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_RES, OUTPUT);
  pinMode(ECHO_RES, INPUT);
  pinMode(TRIG_PET, OUTPUT);
  pinMode(ECHO_PET, INPUT);
  pinMode(SERVO_RELAY, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT); 

  digitalWrite(SERVO_RELAY, HIGH);

  servoMotor.attach(SERVO_PIN);
  servoMotor.write(90);

  connectToWiFi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
  connectToMQTT();
}

unsigned long lastPub = 0;
const unsigned long PUBLISH_INTERVAL = 5000;

void loop() {
  if (!client.connected()) connectToMQTT();
  client.loop();

  if (dispenseTime > 0) {
    dispenseFood(dispenseTime);
    dispenseTime = 0;
  }

  unsigned long now = millis();
  if (now - lastPub > PUBLISH_INTERVAL) {
    lastPub = now;

    float distReservatorio = lerUltrassom(TRIG_RES, ECHO_RES);
    float distPet = lerUltrassom(TRIG_PET, ECHO_PET);

    char buffer[16];
    dtostrf(distReservatorio, 5, 2, buffer);
    client.publish("petfeeder/reservatorio", buffer);

    dtostrf(distPet, 5, 2, buffer);
    client.publish("petfeeder/pet", buffer);

    Serial.printf("Reservatório: %.2f cm, Pet: %.2f cm\n", distReservatorio, distPet);

    if (distReservatorio > LIMITE_BAIXO_RACAO && estadoAnterior != RACAO_BAIXO) {
      client.publish("petfeeder/alerts", "⚠️ Nível baixo de ração!");
      estadoAnterior = RACAO_BAIXO;
    } else if (distReservatorio <= LIMITE_BAIXO_RACAO && estadoAnterior != RACAO_OK) {
      client.publish("petfeeder/alerts", "✅ Nível de ração OK");
      estadoAnterior = RACAO_OK;
    }
  }

  delay(10); 
}
