#include <SoftwareSerial.h> // Bluetooth

// HC-05
SoftwareSerial BT(6, 7); // RX, TX

// Buzzer
int buzzer = 13;

// Ultrassônico
int ECHO = 12;
int TRIG = 11;
long duracao;
int distancia;

// Leds
int ledF = 9;
int ledT = 8;

// LDR
int ldr = A0;

// Encoder
const int encoder = 10;
int leituraEncoder = 0;
int leituraAnterior = 0;
unsigned long contagem = 0;
const int pulsosPorVolta = 20;
const float circunferencia = 0.68;  // metros
unsigned long tempoAnterior = 0;
const unsigned long intervalo = 1000;  // 1 segundo

// Motor
char command;

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ledF, OUTPUT);
  pinMode(ledT, OUTPUT);
  pinMode(ldr, INPUT);
  pinMode(encoder, INPUT);

  pinMode(2, OUTPUT);  // Motor E frente
  pinMode(3, OUTPUT);  // Motor E trás
  pinMode(4, OUTPUT);  // Motor D frente
  pinMode(5, OUTPUT);  // Motor D trás

  Serial.begin(9600);
  BT.begin(9600);

  Serial.println("Arduino pronto");
  BT.println("Arduino pronto");
}

void loop() {

  // ===== ULTRASSÔNICO =====
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  duracao = pulseIn(ECHO, HIGH);
  distancia = duracao / 58;  // cm

  if (distancia <= 0 || distancia > 300) distancia = 300;

  if (distancia <= 20) {
    tone(buzzer, 2000, 500);
    digitalWrite(ledT, HIGH);
  } else {
    digitalWrite(ledT, LOW);
  }

  // ===== LDR - Farol AUTOMÁTICO =====
  int valorLDR = analogRead(ldr);
  if (valorLDR <= 150) {
    digitalWrite(ledF, HIGH);
  } else {
    digitalWrite(ledF, LOW);
  }

  // ===== ENCODER =====
  leituraEncoder = digitalRead(encoder);
  if (leituraEncoder == HIGH && leituraAnterior == LOW) contagem++;
  leituraAnterior = leituraEncoder;

  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= intervalo) {
    tempoAnterior = tempoAtual;

    float rpm = (contagem / (float)pulsosPorVolta) * 60.0;
    contagem = 0;
    float velocidade = rpm * circunferencia * 60.0 / 1000.0;

    // ===== ENVIO DE DADOS PARA SERIAL E BLUETOOTH =====
    Serial.println("----- SENSORES -----");
    BT.println("----- SENSORES -----");

    Serial.print("RPM: "); Serial.println(rpm);
    BT.print("RPM: "); BT.println(rpm);

    Serial.print("Velocidade: "); Serial.print(velocidade); Serial.println(" km/h");
    BT.print("Velocidade: "); BT.print(velocidade); BT.println(" km/h");

    Serial.print("LDR: "); Serial.println(valorLDR);
    BT.print("LDR: "); BT.println(valorLDR);

    Serial.print("Distancia: "); Serial.print(distancia); Serial.println(" cm");
    BT.print("Distancia: "); BT.print(distancia); BT.println(" cm");

    Serial.println("---------------------");
    BT.println("---------------------");
  }

  // ===== HC-05 (Bluetooth) COMANDOS =====
  if (BT.available() > 0) {
    command = BT.read();
    if (command == '\n' || command == '\r' || command == ' ') return;

    Serial.print("Comando recebido via Bluetooth: ");
    Serial.println(command);
    BT.print("Comando recebido: "); BT.println(command);

    // Buzina
    if (command == 'B') {
      tone(buzzer, 1000, 500);
      Serial.println("Buzina");
      BT.println("Buzina");
    }

    // Movimentação
    else if (command == 'F') {  // Frente
      digitalWrite(2, HIGH); digitalWrite(3, LOW);
      digitalWrite(4, HIGH); digitalWrite(5, LOW);
    }
    else if (command == 'R') {  // Ré

      if (distancia > 20) {
        digitalWrite(2, LOW); digitalWrite(3, HIGH);
        digitalWrite(4, LOW); digitalWrite(5, HIGH);
        Serial.println("Movendo para Ré");
        BT.println("Movendo para Ré");
      } else {
        digitalWrite(2, LOW); digitalWrite(3, LOW);
        digitalWrite(4, LOW); digitalWrite(5, LOW);
        Serial.println("Ré bloqueada, objeto perto demais");
        BT.println("Ré bloqueada, objeto perto demais");
      }
    }
    else if (command == 'E') {  // Esquerda
      digitalWrite(2, HIGH); digitalWrite(3, LOW);
      digitalWrite(4, LOW); digitalWrite(5, HIGH);
    }
    else if (command == 'D') {  // Direita
      digitalWrite(2, LOW); digitalWrite(3, HIGH);
      digitalWrite(4, HIGH); digitalWrite(5, LOW);
    }
    else if (command == 'S') {  // Parar
      digitalWrite(2, LOW); digitalWrite(3, LOW);
      digitalWrite(4, LOW); digitalWrite(5, LOW);
    }
  }

  delay(50); // Pequena pausa para estabilizar leituras
}
