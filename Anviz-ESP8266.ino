/**
 * Emulador de Dispositivo Anviz para NodeMCU/ESP8266
 * 
 * Este código implementa un emulador de dispositivo de control de acceso Anviz
 * que puede ser utilizado con el software CrossChex. Soporta un lector RFID Wiegand 26/34
 * para control de acceso con tarjetas.
 * 
 * Autor: Oemspot
 * Fecha: 03-11-2025
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>

// ========= CONFIGURACIÓN WIFI Y RED ==========

#define SERVER_PORT 5010                           // Puerto estándar de dispositivos Anviz

// ========= CONFIGURACIÓN DEL DISPOSITIVO EMULADO ===========

#define FIRMWARE_VERSION "TC401" // Versión de firmware
#define SERIAL_NUMBER "TC403" // Número de serie (16 bytes max)

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

// ========= VARIABLES PARA MANEJO NO BLOQUEANTE ===========
LedState currentLedState = LED_IDLE;
unsigned long actionStartTime = 0;
unsigned long ledBlinkTime = 0;
int blinkCount = 0;

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
  WiFiManager wifiManager;
  String mac = WiFi.macAddress();
  String lastMac = mac.substring(12);
  lastMac.replace(":", "");
  String apName = "Anviz-ESP8266-" + lastMac;

  if (!wifiManager.autoConnect(apName.c_str())) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  Serial.println("connected...yeey :)");
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  return true;
}

// ========= SETUP ===========
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n[SETUP] Iniciando emulador de dispositivo Anviz...");

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
  loadWebAuth();
  loadUsers();
  loadRecords();
  Serial.println("[SETUP] Carga de datos finalizada.");
  
  // Inicializar hardware AHORA que tenemos la configuración de pines cargada
  Serial.println("[SETUP] Configurando pines de hardware...");
  pinMode(basicConfig.pin_relay, OUTPUT);
  pinMode(basicConfig.pin_led, OUTPUT);
  pinMode(basicConfig.pin_d0, INPUT_PULLUP);  // Wiegand Data0
  pinMode(basicConfig.pin_d1, INPUT_PULLUP);  // Wiegand Data1
  
  digitalWrite(basicConfig.pin_relay, LOW);
  digitalWrite(basicConfig.pin_led, LOW);
  
  // LED de arranque
  for (int i = 0; i < 3; i++) {
    digitalWrite(basicConfig.pin_led, HIGH);
    delay(100);
    digitalWrite(basicConfig.pin_led, LOW);
    delay(100);
  }
  
  // Configurar interrupciones para Wiegand
  Serial.println("[SETUP] Configurando interrupciones Wiegand...");
  attachInterrupt(digitalPinToInterrupt(basicConfig.pin_d0), handleD0, FALLING);
  attachInterrupt(digitalPinToInterrupt(basicConfig.pin_d1), handleD1, FALLING);
  Serial.println("[SETUP] Interrupciones Wiegand activadas.");
  
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
    digitalWrite(basicConfig.pin_led, HIGH);
    delay(1000);
    digitalWrite(basicConfig.pin_led, LOW);
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
    Serial.println("[HEARTBEAT] System OK. Uptime: " + uptime + " min. | millis: " + String(millis()) + " | lastHeartbeat: " + String(lastHeartbeat));
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
  
  // Manejar estados de LED y relé de forma no bloqueante
  handleLedAndRelay();

  // Verificar si la conexión WiFi sigue activa, reconectar si es necesario
  static unsigned long lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 30000) { // Verificar cada 30 segundos
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Conexión WiFi perdida, intentando reconectar...");
      // Parpadear LED para indicar reconexión
      digitalWrite(basicConfig.pin_led, HIGH);
      delay(200);
      digitalWrite(basicConfig.pin_led, LOW);
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

  // Comprobar si es hora de un reinicio programado
  checkScheduledReboot();

  // Guardar registros periódicamente si hay nuevos
  static unsigned long lastSaveTime = 0;
  if (newRecordCount > 0 && millis() - lastSaveTime > 300000) { // 5 minutos
    saveRecords();
    lastSaveTime = millis();
  }

  delay(10); // Pequeño delay para dar tiempo a otros procesos
}

// ========= FUNCIÓN PARA REINICIO PROGRAMADO ===========
void checkScheduledReboot() {
  static int lastCheckedMinute = -1;
  if (basicConfig.rebootEnabled && now() > 0) { // Asegurarse de que la hora esté sincronizada
    int currentMinute = minute();
    if (currentMinute != lastCheckedMinute) {
      lastCheckedMinute = currentMinute;
      if (hour() == basicConfig.rebootHour && minute() == basicConfig.rebootMinute) {
        Serial.println("[REBOOT] Reinicio programado activado. Reiniciando...");
        delay(1000);
        ESP.restart();
      }
    }
  }
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

  // Pines por defecto
  basicConfig.pin_d0 = D7;
  basicConfig.pin_d1 = D6;
  basicConfig.pin_relay = D1;
  basicConfig.pin_led = D0;
  basicConfig.relayOnDuration = 2000; // 2 segundos por defecto
  
  // Número de serie por defecto
  strcpy(serialNumber, SERIAL_NUMBER);
}

void setupWebServer() {
  // Rutas del servidor web
  webServer.on("/", HTTP_GET, handleRoot);
  webServer.on("/users", HTTP_GET, handleUsers);
  webServer.on("/records", HTTP_GET, handleRecords);
  webServer.on("/settings", HTTP_GET, handleSettings);
  webServer.on("/savesettings", HTTP_POST, handleSaveSettings);
  webServer.on("/auth", HTTP_GET, handleAuth);
  webServer.on("/saveauth", HTTP_POST, handleSaveAuth);
  webServer.on("/changewifi", HTTP_GET, handleWifiChange);
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
      if (currentLedState == LED_IDLE) { // Procesar solo si no hay otra acción en curso
        Serial.print("[WIEGAND] Acceso concedido a: ");
        Serial.println((char*)users[userIndex].name);
        
        // Iniciar estado de acceso concedido
        currentLedState = LED_ACCESS_GRANTED;
        actionStartTime = millis();
        digitalWrite(basicConfig.pin_relay, HIGH);
        digitalWrite(basicConfig.pin_led, HIGH);
        
        // Crear registro de acceso
        createAccessRecord(userIndex);
      }
    } else {
      // Usuario no encontrado
      if (currentLedState == LED_IDLE) { // Procesar solo si no hay otra acción en curso
        Serial.println("[WIEGAND] Tarjeta no autorizada");
        
        // Iniciar estado de acceso denegado
        currentLedState = LED_ACCESS_DENIED;
        actionStartTime = millis();
        ledBlinkTime = millis();
        blinkCount = 0;
      }
    }
    
    // Resetear variables Wiegand para la próxima lectura
    wiegandData = 0;
    wiegandBitCount = 0;
  }
}

// ========= FUNCIÓN PARA MANEJO DE ESTADOS (NO BLOQUEANTE) ===========
void handleLedAndRelay() {
  unsigned long currentTime = millis();

  switch (currentLedState) {
    case LED_FORCED_UNLOCK: // Cae al mismo caso que el acceso concedido
    case LED_ACCESS_GRANTED:
      // Esperar a que pase el tiempo para desactivar el relé y el LED
      if (currentTime - actionStartTime >= basicConfig.relayOnDuration) {
        digitalWrite(basicConfig.pin_relay, LOW);
        digitalWrite(basicConfig.pin_led, LOW);
        currentLedState = LED_IDLE; // Volver al estado de reposo
      }
      break;

    case LED_ACCESS_DENIED:
      // Realizar 3 parpadeos (6 cambios de estado)
      if (blinkCount < 6) {
        if (currentTime - ledBlinkTime >= 200) { // Cada 200ms
          digitalWrite(basicConfig.pin_led, !digitalRead(basicConfig.pin_led)); // Invertir LED
          ledBlinkTime = currentTime;
          blinkCount++;
        }
      } else {
        // Parpadeo finalizado, apagar LED y volver a estado de reposo
        digitalWrite(basicConfig.pin_led, LOW);
        currentLedState = LED_IDLE;
      }
      break;

    case LED_IDLE:
    default:
      // No hacer nada
      break;
  }
}

// ========= FUNCIÓN PARA CREAR REGISTROS DE ACCESO ===========
void createAccessRecord(int userIndex) {
  // Usar recordCount como índice de escritura en un búfer circular
  int writeIndex = recordCount % 500;

  AccessRecord& record = records[writeIndex]; // Obtener referencia al registro

  memcpy(record.id, users[userIndex].id, 5);
  
  // Calcular timestamp (segundos desde 2000-01-01)
  record.timestamp = now() - 946684800; // Unix timestamp - timestamp 2000-01-01
  
  record.backup = 0x08; // Indicar acceso por tarjeta
  record.recordType = 0x80; // Indicar acceso exitoso (bit 7 = 1)
  memset(record.workCode, 0, 3); // Sin código de trabajo
  
  recordCount++;
  newRecordCount++;

  // Opcional: Guardar inmediatamente si el búfer se llena, 
  // aunque el guardado periódico en el loop es más eficiente.
  if (recordCount % 500 == 0) {
    saveRecords();
  }
}
