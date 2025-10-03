/**
 * variables.h
 * Definición de variables globales para el emulador Anviz
 */

#ifndef VARIABLES_H
#define VARIABLES_H

// ========= VARIABLES GLOBALES ===========
WiFiServer server(SERVER_PORT);        // Servidor TCP
WiFiClient client;                     // Cliente conectado actual
User users[100];                       // Máximo 100 usuarios
int userCount = 0;                     // Contador de usuarios
AccessRecord records[500];             // Buffer para registros de acceso
int recordCount = 0;                   // Contador total de registros
int newRecordCount = 0;                // Contador de nuevos registros
BasicConfig basicConfig;               // Configuración básica
char serialNumber[17] = {0};           // SN del dispositivo (16 bytes máximo)
uint32_t deviceId = 0x00010001;        // ID del dispositivo (4 bytes)

// Cliente NTP para sincronización de tiempo
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Estado para seguimiento de descargas
int lastDownloadUserIndex = 0;         // Último índice de usuario descargado
int lastDownloadRecordIndex = 0;       // Último índice de registro descargado

// Servidor web para configuración
ESP8266WebServer webServer(80);        // Servidor web en puerto 80

// Credenciales para la interfaz web
char web_user[33] = "admin";
char web_pass[33] = "admin";

#endif // VARIABLES_H