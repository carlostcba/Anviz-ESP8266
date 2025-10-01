# Anviz-ESP8266: Emulador de Control de Acceso Anviz para ESP8266

Este proyecto convierte un microcontrolador ESP8266 en un emulador de dispositivo de control de acceso Anviz, compatible con el software **Anviz CrossChex**. Permite gestionar usuarios y registrar accesos mediante un lector de tarjetas RFID con interfaz Wiegand.

## Características

-   **Emulación de Protocolo Anviz:** Se comunica vía TCP (puerto 5010) para ser gestionado por CrossChex.
-   **Lector RFID Wiegand:** Compatible con lectores Wiegand 26/34.
-   **Interfaz Web:** Servidor web para monitorización de estado, usuarios y registros.
-   **Persistencia de Datos:** Almacena configuración, usuarios y registros en la memoria SPIFFS.
-   **Control de Acceso:** Activa un relé para abrir una puerta.
-   **Sincronización de Hora:** Cliente NTP para mantener la hora actualizada.
-   **Configuración WiFi Sencilla:** Usa WiFiManager para una fácil configuración inicial de la red.

## Requisitos de Hardware

-   **Microcontrolador:** Placa de desarrollo ESP8266 (ej. NodeMCU, Wemos D1 Mini).
-   **Lector RFID:** Lector de tarjetas con salida Wiegand (26 o 34 bits).
-   **Módulo de Relé:** Relé de 5V compatible con microcontroladores.
-   **Fuente de Alimentación:** 5V para alimentar el ESP8266 y periféricos.

## Conexiones de Hardware

| Componente      | Pin ESP8266 |
| --------------- | ----------- |
| Lector Wiegand D0 | D7          |
| Lector Wiegand D1 | D6          |
| Módulo de Relé    | D1          |
| LED de Estado     | D0          |

## Dependencias de Software

Instala las siguientes librerías desde el Gestor de Librerías del Arduino IDE:

-   `ArduinoJson` by Benoit Blanchon (v6 recomendada)
-   `NTPClient` by Fabrice Weinberg
-   `WiFiManager` by tzapu

## Instalación y Uso

1.  **Hardware:** Conecta el lector Wiegand y el relé a los pines GPIO especificados.
2.  **Software:**
    *   Abre `Anviz-ESP8266.ino` en el Arduino IDE.
    *   Instala las librerías requeridas.
    *   Selecciona tu placa ESP8266 y el puerto COM.
3.  **Carga:** Sube el código a tu placa.
4.  **Configuración WiFi (Primera Vez):**
    *   El dispositivo creará un punto de acceso WiFi llamado **"Anviz-ESP8266-XXYY"**.
    *   Conéctate a esta red. Se abrirá un portal cautivo.
    *   Selecciona tu red WiFi, introduce la contraseña y guarda.
    *   El dispositivo se reiniciará y conectará a tu red.
5.  **Uso:**
    *   Abre el Monitor Serie (115200 baudios) para ver la IP asignada.
    *   Accede a la interfaz web con la IP del dispositivo.
    *   Usa **Anviz CrossChex** para gestionar usuarios, añadiendo el dispositivo con su IP y el puerto `5010`.

## Interfaz Web

La interfaz web proporciona:

-   **Inicio:** Estado del sistema (IP, WiFi, contadores, hora).
-   **Usuarios:** Lista de usuarios registrados.
-   **Registros:** Últimos 50 registros de acceso.
-   **Configuración:** Información de configuración y opción de reinicio.

## Estructura del Proyecto

-   `Anviz-ESP8266.ino`: Lógica principal (`setup()` y `loop()`).
-   `protocolo.h`: Implementación del protocolo Anviz.
-   `web.h`: Código de la interfaz web.
-   `almacenamiento.h`: Funciones de guardado y carga en SPIFFS.
-   `estructuras.h`: Definiciones de estructuras de datos.
-   `variables.h`: Variables globales.
-   `utilidades.h`: Funciones auxiliares.

## Mejoras Futuras

-   [ ] Soporte para gestión de huellas dactilares.
-   [ ] Subida de usuarios y registros desde la interfaz web.
-   [ ] Autenticación para la interfaz web.
-   [ ] Configuración de pines GPIO desde la interfaz web.

## Autor

-   **Autor:** Oemspot
-   **Fecha:** 31-03-2025