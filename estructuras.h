/**
 * estructuras.h
 * Definición de estructuras de datos para el emulador Anviz
 */

#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

// Estructura de datos de usuario
typedef struct {
  uint8_t id[5];        // ID de empleado (5 bytes)
  uint8_t password[3];  // Contraseña (no utilizada en este proyecto)
  uint32_t cardId;      // ID de tarjeta RFID
  char name[11];        // Nombre (10 caracteres + terminador)
  uint8_t department;   // Departamento
  uint8_t group;        // Grupo
  uint8_t mode;         // Modo de asistencia
  uint8_t fpStatus[2];  // Estado de huella digital (no utilizada)
  uint8_t special;      // Información especial
  bool isActive;        // Estado activo/inactivo
} User;

// Estructura de registros de acceso
typedef struct {
  uint8_t id[5];        // ID de usuario
  uint32_t timestamp;   // Tiempo (segundos desde 2000-01-01)
  uint8_t backup;       // Código de backup (tipo de verificación)
  uint8_t recordType;   // Tipo de registro
  uint8_t workCode[3];  // Código de trabajo (no utilizado)
} AccessRecord;

// Configuración básica del dispositivo
typedef struct {
  char firmwareVersion[9];  // Versión firmware (8 char + null)
  uint8_t password[3];      // Contraseña de comunicación
  uint8_t sleepTime;        // Tiempo de reposo
  uint8_t volume;           // Volumen
  uint8_t language;         // Idioma
  uint8_t dateFormat;       // Formato de fecha/hora
  uint8_t machineStatus;    // Estado de la máquina
  uint8_t languageFlag;     // Banderas de idioma
  uint8_t cmdVersion;       // Versión de comandos
} BasicConfig;

#endif // ESTRUCTURAS_H