/**
 * Emulador de Dispositivo Anviz para NodeMCU/ESP8266
 * 
 * Este código implementa un emulador de dispositivo de control de acceso Anviz
 * que puede ser utilizado con el software CrossChex. Soporta un lector RFID Wiegand 26/34
 * para control de acceso con tarjetas.
 * 
 * Autor: Claude
 * Fecha: 31-03-2025
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ========= CONFIGURACIÓN WIFI Y RED ==========
const char* wifi_ssid = "SYSTEMS";  // Tu SSID WiFi con espacios
const char* wifi_password = "LASALLE300";          // Tu contraseña WiFi
#define SERVER_PORT 5010                           // Puerto estándar de dispositivos Anviz

// ========= CONFIGURACIÓN DE PINES ===========
#define PIN_D0 D7     // GPIO para Data0 del lector Wiegand
#define PIN_D1 D6     // GPIO para Data1 del lector Wiegand
#define RELAY_PIN D1  // GPIO para control del relé
#define LED_PIN D0    // GPIO para LED de estado

// ========= CONFIGURACIÓN DEL DISPOSITIVO EMULADO ===========
#define DEVICE_ID 0x00010001        // ID del dispositivo (4 bytes)
#define FIRMWARE_VERSION "TC400001" // Versión de firmware
#define SERIAL_NUMBER "TC40000000001" // Número de serie (16 bytes max)

// ========= DEFINICIONES DEL PROTOCOLO ===========
#define STX 0xA5           // Inicio de trama
#define ACK_SUCCESS 0x00   // Comando ejecutado con éxito
#define ACK_FAIL 0x01      // Error al ejecutar el comando
#define ACK_FULL 0x04      // Memoria llena
#define ACK_EMPTY 0x05     // Sin datos
#define ACK_NO_USER 0x06   // Usuario no encontrado
#define ACK_TIME_OUT 0x08  // Tiempo de espera agotado

// ========= VARIABLES WIEGAND ===========
volatile unsigned long wiegandData = 0;      // Datos Wiegand recibidos
volatile byte wiegandBitCount = 0;           // Contador de bits recibidos
volatile unsigned long wiegandTimeoutStart = 0; // Tiempo de inicio para timeout
#define WIEGAND_TIMEOUT 25  // Timeout en milisegundos

// Incluir los archivos de cabecera
#include "estructuras.h"
#include "variables.h"
#include "protocolo.h"
#include "web.h"
#include "utilidades.h"
#include "almacenamiento.h"

// ========= MANEJADORES DE INTERRUPCIÓN PARA WIEGAND ===========
ICACHE_RAM_ATTR void handleD0() {
  wiegandTimeoutStart = millis();
  if (wiegandBitCount < 34) {
    wiegandBitCount++;
    wiegandData = wiegandData << 1;  // Shift left y agregar 0
  }
}

ICACHE_RAM_ATTR void handleD1() {
  wiegandTimeoutStart = millis();
  if (wiegandBitCount < 34) {
    wiegandBitCount++;
    wiegandData = (wiegandData << 1) | 1;  // Shift left y agregar 1
  }
}

// Función mejorada para conectar al WiFi
bool connectWiFi() {
  Serial.print("Conectando a WiFi: ");
  Serial.println(wifi_ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.begin(wifi_ssid, wifi_password);
  
  unsigned long startTime = millis();
  uint8_t timeout = 30; // 30 segundos para timeout (más tiempo que antes)
  
  // Bucle mientras intentamos conectar
  while (millis() - startTime < timeout * 1000) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConectado a WiFi");
      Serial.print("Dirección IP: ");
      Serial.println(WiFi.localIP());
      return true;
    }
    
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nError al conectar a WiFi - Timeout");
  return false;
}

// ========= SETUP ===========
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n[SETUP] Iniciando emulador de dispositivo Anviz...");
  
  // Inicializar hardware
  Serial.println("[SETUP] Configurando pines de hardware...");
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIN_D0, INPUT_PULLUP);  // Wiegand Data0
  pinMode(PIN_D1, INPUT_PULLUP);  // Wiegand Data1
  
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  // LED de arranque
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  
  // Configurar interrupciones para Wiegand
  Serial.println("[SETUP] Configurando interrupciones Wiegand...");
  attachInterrupt(digitalPinToInterrupt(PIN_D0), handleD0, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_D1), handleD1, FALLING);
  Serial.println("[SETUP] Interrupciones Wiegand activadas.");

  // Inicializar sistema de archivos
  if (!SPIFFS.begin()) {
    Serial.println("Error al montar el sistema de archivos");
    // Indicar error con LED
    blinkError(3);
  } else {
    Serial.println("Sistema de archivos SPIFFS montado");
  }
  
  // Inicializar configuración por defecto
  Serial.println("[SETUP] Cargando configuración, usuarios y registros...");
  initializeDefaultConfig();
  
  // Cargar datos almacenados
  loadConfig();
  loadUsers();
  loadRecords();
  Serial.println("[SETUP] Carga de datos finalizada.");
  
  // Conectar a WiFi con el método mejorado
  if (connectWiFi()) {
    // Configurar y arrancar servidor web
    setupWebServer();
    webServer.begin();
    Serial.println("Servidor web iniciado en puerto 80");
    
    // Iniciar cliente NTP
    timeClient.begin();
    timeClient.setTimeOffset(-3 * 3600); // Ajustar según zona horaria
    timeClient.update();
    setInternalTime();
    Serial.println("Reloj sincronizado con NTP");
    
    // Iniciar servidor TCP
    server.begin();
    Serial.println("Servidor TCP iniciado en puerto 5010");
    
    // Indicar éxito con LED
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
  } else {
    Serial.println("\nError al conectar a WiFi");
    // Indicar error con LED
    blinkError(5);
  }
  Serial.println("=== [SETUP] INICIALIZACIÓN COMPLETADA ===");
}

// ========= LOOP PRINCIPAL ===========
void loop() {
  // Heartbeat para saber que el loop está corriendo
  static unsigned long lastHeartbeat = 0;
  const unsigned long heartbeatInterval = 10000; // 10 segundos
  if (millis() - lastHeartbeat > heartbeatInterval) {
    lastHeartbeat = millis();
    String uptime = String(millis() / 60000);
    Serial.println("[HEARTBEAT] System OK. Uptime: " + uptime + " min.");
  }

  // Atender servidor web
  webServer.handleClient();
  
  // Revisar si hay cliente TCP nuevo
  WiFiClient newClient = server.available();
  if (newClient) {
    if (!client || !client.connected()) {
      client = newClient;
      Serial.println("Nuevo cliente TCP conectado");
    }
  }
  
  // Procesar comandos TCP si hay cliente conectado
  if (client && client.connected() && client.available() > 0) {
    processAnvizCommand();
  }
  
  // Revisar si hay tarjeta Wiegand
  checkWiegandCard();
  
  // Verificar si la conexión WiFi sigue activa, reconectar si es necesario
  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 30000) { // Verificar cada 30 segundos
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Conexión WiFi perdida, intentando reconectar...");
      // Parpadear LED para indicar reconexión
      digitalWrite(LED_PIN, HIGH);
      delay(200);
      digitalWrite(LED_PIN, LOW);
      // Intentar reconectar
      connectWiFi();
    }
  }
  
  // Actualizar tiempo NTP ocasionalmente
  static unsigned long lastNtpUpdate = 0;
  if (millis() - lastNtpUpdate > 3600000) { // Cada hora
    timeClient.update();
    setInternalTime();
    lastNtpUpdate = millis();
  }
  delay(10); // Pequeño delay para dar tiempo a otros procesos
}

// ========= FUNCIONES DE INICIALIZACIÓN ===========
void initializeDefaultConfig() {
  // Configuración básica por defecto
  strcpy(basicConfig.firmwareVersion, FIRMWARE_VERSION);
  memset(basicConfig.password, 0, 3);
  basicConfig.sleepTime = 0;
  basicConfig.volume = 5;
  basicConfig.language = 2; // Inglés
  basicConfig.dateFormat = 0x02; // Formato inglés, 24 horas
  basicConfig.machineStatus = 0;
  basicConfig.languageFlag = 0x10;
  basicConfig.cmdVersion = 0x02;
  
  // Número de serie por defecto
  strcpy(serialNumber, SERIAL_NUMBER);
}

void setupWebServer() {
  // Rutas del servidor web
  webServer.on("/", HTTP_GET, handleRoot);
  webServer.on("/users", HTTP_GET, handleUsers);
  webServer.on("/records", HTTP_GET, handleRecords);
  webServer.on("/settings", HTTP_GET, handleSettings);
  webServer.on("/clearlogs", HTTP_GET, handleClearLogs);
  webServer.on("/reset", HTTP_GET, handleReset);
  
  // Rutas para operaciones de mantenimiento
  webServer.on("/upload", HTTP_POST, []() {
    webServer.send(200, "text/plain", "Datos recibidos");
  }, handleFileUpload);
}

// ========= FUNCIÓN PARA SINCRONIZACIÓN DE TIEMPO ===========
void setInternalTime() {
  // Convertir tiempo NTP a estructura time_t
  time_t epochTime = timeClient.getEpochTime();
  setTime(epochTime);
}

// ========= FUNCIÓN PARA VERIFICAR TARJETAS WIEGAND ===========
void checkWiegandCard() {
  // Verificar si se ha recibido una tarjeta completa (timeout después del último bit)
  // Esta condición se evalúa en cada ciclo del loop()
  if (wiegandBitCount > 0 && millis() - wiegandTimeoutStart > WIEGAND_TIMEOUT) {
    uint32_t cardId = 0;
    
    if (wiegandBitCount == 26) { // Wiegand 26
      // Extraer el número de tarjeta (ignorando bits de paridad)
      cardId = (wiegandData >> 1) & 0xFFFFFF;
    } 
    else if (wiegandBitCount == 34) { // Wiegand 34
      // Extraer el número de tarjeta (ignorando bits de paridad)
      cardId = (wiegandData >> 1) & 0xFFFFFFFF;
    }
    
    Serial.print("\n[WIEGAND] Tarjeta detectada: 0x");
    Serial.println(cardId, HEX);
    
    // Buscar usuario por tarjeta
    int userIndex = findUserByCardId(cardId);
    
    if (userIndex >= 0) {
      // Usuario encontrado
      Serial.print("[WIEGAND] Acceso concedido a: ");
      Serial.println((char*)users[userIndex].name);
      
      // Activar relé y LED
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      
      // Crear registro de acceso
      createAccessRecord(userIndex);
      
      delay(2000);
      
      // Desactivar relé y LED
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
    } else {
      // Usuario no encontrado
      Serial.println("[WIEGAND] Tarjeta no autorizada");
      
      // Parpadeo de LED para indicar rechazo
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
      }
    }
    
    // Resetear variables Wiegand para la próxima lectura
    wiegandData = 0;
    wiegandBitCount = 0;
  }
}

// ========= FUNCIÓN PARA CREAR REGISTROS DE ACCESO ===========
void createAccessRecord(int userIndex) {
  if (recordCount >= 500) {
    // Si el buffer está lleno, sobrescribir los registros más antiguos
    // Implementar una lógica circular
    for (int i = 0; i < 499; i++) {
      records[i] = records[i+1];
    }
    recordCount = 499;
  }
  
  AccessRecord record;
  memcpy(record.id, users[userIndex].id, 5);
  
  // Calcular timestamp (segundos desde 2000-01-01)
  record.timestamp = now() - 946684800; // Unix timestamp - timestamp 2000-01-01
  
  record.backup = 0x08; // Indicar acceso por tarjeta
  record.recordType = 0x80; // Indicar acceso exitoso (bit 7 = 1)
  memset(record.workCode, 0, 3); // Sin código de trabajo
  
  records[recordCount++] = record;
  newRecordCount++;
  
  saveRecords();
}
