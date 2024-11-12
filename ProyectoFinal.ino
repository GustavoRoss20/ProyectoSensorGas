#include <OneWire.h>
#include <DallasTemperature.h>

// Sensores
OneWire nuestraWire(2);                             // Pin para el sensor de temperatura
DallasTemperature sensorTemperatura(&nuestraWire);  // Inicializar el sensor de temperatura
const int pinMQ2 = A0;                              // Pin para el sensor MQ-2
const int pinPoten = A1;                            // Pin para el potenciómetro
const int pinLedVerde = 5;                          // Indicador visual verde
const int pinLedAmarillo = 6;                       // Indicador visual amarillo
const int pinLedRojo = 7;                           // Indicador visual rojo
const int pinReley = 8;                             // Pin para el relé
const int pinBuzzer = 9;                            // Buzzer

// Variables
float valorBaseMQ2 = 0;          // Valor base del sensor MQ-2 en aire limpio
unsigned long previoMillis = 0;  // Almacena el último tiempo de lectura
const long intervalo = 200;      // Intervalo de tiempo entre lecturas (100 ms)
float concentracionGas = 0;      // Variable para almacenar la concentración de gas
float temperatura = 0;           // Variable para almacenar la temperatura
int valorPotenciometro = 0;      // Variable para almacenar el valor del potenciómetro

const long interval = 1000;      // Intervalo de 1 segundo (1000 milisegundos)
bool buzzerState = false;        // Estado del buzzer (encendido/apagado)
bool buzzerEnabled = false;      // Controla si el buzzer está habilitado o no

float umbralGas = 20.0;          // Umbral de concentración de gas
float umbralTemperatura = 35.0;  // Umbral de temperatura
int umbralPresion = 300;         // Umbral de presión

void setup() {
  Serial.begin(9600);

  pinMode(pinMQ2, INPUT);
  pinMode(pinPoten, INPUT);
  pinMode(pinReley, OUTPUT);
  pinMode(pinLedVerde, OUTPUT);
  pinMode(pinLedAmarillo, OUTPUT);
  pinMode(pinLedRojo, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);

  valorBaseMQ2 = calibrarSensor(20000);  // Calibrar durante 20 segundos
  sensorTemperatura.begin();
  
  digitalWrite(pinReley, HIGH);  // Configurar el relé en alto para iniciar abierto
}

void loop() {
  unsigned long tiempoActual = millis();

  if (tiempoActual - previoMillis >= intervalo) {
    previoMillis = tiempoActual;

    concentracionGas = leerMQ2();
    temperatura = leerTemperatura();
    valorPotenciometro = leerPotenciometro();

    Serial.println("----------------------------------------------");
    Serial.print("Concentración de gas MQ-2: ");
    Serial.println(concentracionGas);
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");
    Serial.print("Valor del Potenciómetro: ");
    Serial.println(valorPotenciometro);
    Serial.println("----------------------------------------------");

    if (valorPotenciometro < umbralPresion) {
      if (concentracionGas < umbralGas) {
        digitalWrite(pinReley, HIGH);
        digitalWrite(pinLedVerde, LOW);
        digitalWrite(pinLedAmarillo, HIGH);
        digitalWrite(pinLedRojo, LOW);        
        buzzerEnabled = false;
        Serial.println("Estado: Se está acabando el gas.");
      } else if (concentracionGas >= umbralGas) {
        if (temperatura < umbralTemperatura) {
          digitalWrite(pinReley, LOW); // Cerrar el relé
          digitalWrite(pinLedVerde, LOW);
          digitalWrite(pinLedAmarillo, LOW);
          digitalWrite(pinLedRojo, HIGH);   
          buzzerEnabled = false;       
          Serial.println("Estado: Fuga detectada, pero no hay calor.");
        } else {
          digitalWrite(pinReley, HIGH); // Mantener el relé abierto
          digitalWrite(pinLedVerde, LOW);
          digitalWrite(pinLedAmarillo, LOW);
          digitalWrite(pinLedRojo, HIGH);
          buzzerEnabled = true; 
          Serial.println("Estado: Fuga detectada y calor presente.");
        }
      }
    } else {
      if (concentracionGas >= umbralGas) {
        digitalWrite(pinReley, HIGH);
        digitalWrite(pinLedVerde, HIGH);
        digitalWrite(pinLedAmarillo, LOW);
        digitalWrite(pinLedRojo, LOW);
        buzzerEnabled = false;
        Serial.println("Estado: Estufa abierta.");
      } else if (concentracionGas < umbralGas) {
        digitalWrite(pinReley, HIGH);
        digitalWrite(pinLedVerde, HIGH);
        digitalWrite(pinLedAmarillo, LOW);
        digitalWrite(pinLedRojo, LOW);
        buzzerEnabled = false;
        Serial.println("Estado: Todo normal.");
      }
    }
  }
  activarBuzzer();
}

float calibrarSensor(int tiempoCalibracion) {
  float suma = 0;
  int contador = 0;
  unsigned long tiempoInicio = millis();

  while (millis() - tiempoInicio < tiempoCalibracion) {
    suma += analogRead(pinMQ2);
    contador++;
  }

  float valorBase = suma / contador;
  Serial.print("Calibración completada. Valor base MQ-2: ");
  Serial.println(valorBase);
  return valorBase;
}

float leerMQ2() {
  int lecturaSensor = analogRead(pinMQ2);
  float concentracionGas = 0;

  if (valorBaseMQ2 != 0) {
    concentracionGas = (lecturaSensor / valorBaseMQ2) * 100;
  }

  return concentracionGas;
}

float leerTemperatura() {
  sensorTemperatura.requestTemperatures();
  return sensorTemperatura.getTempCByIndex(0);
}

int leerPotenciometro() {
  return analogRead(pinPoten);
}

// Función para alternar el estado del buzzer usando millis()
void activarBuzzer() {
  static unsigned long previousMillis = 0; // Variable estática para almacenar el último tiempo
  unsigned long currentMillis = millis();  // Obtiene el tiempo actual

  // Verifica si el buzzer está habilitado
  if (buzzerEnabled) {
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis; // Actualiza el tiempo para el próximo cambio

      // Alterna el estado del buzzer
      buzzerState = !buzzerState;
      if (buzzerState) {
        tone(pinBuzzer, 4000); // Enciende el buzzer con una frecuencia de 4KHz
      } else {
        noTone(pinBuzzer);     // Apaga el buzzer
      }
    }
  } else {
    noTone(pinBuzzer);          // Asegura que el buzzer esté apagado si no está habilitado
    previousMillis = currentMillis; // Reinicia el temporizador cuando se desactiva el buzzer
  }
}