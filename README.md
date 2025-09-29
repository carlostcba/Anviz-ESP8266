# Anviz-ESP8266: Emulador de Dispositivo de Control de Acceso

Este proyecto convierte un microcontrolador ESP8266 (como un NodeMCU) en un emulador de un dispositivo de control de acceso Anviz. Es compatible con el software **Anviz CrossChex** y permite gestionar usuarios y registrar accesos utilizando un lector de tarjetas RFID con interfaz Wiegand.

## Características Principales

-   **Emulación de Protocolo Anviz:** Se comunica a través de TCP en el puerto 5010, permitiendo que el software CrossChex lo detecte y gestione como si fuera un dispositivo Anviz real.
-   **Lector RFID Wiegand:** Lee tarjetas RFID utilizando lectores con protocolo Wiegand 26 o 34.
-   **Interfaz Web:** Incluye un servidor web para monitorizar el estado del dispositivo, ver la lista de usuarios, consultar los registros de acceso y realizar operaciones básicas.
-   **Persistencia de Datos:** Guarda la configuración, los usuarios y los registros de acceso en la memoria SPIFFS del ESP8266.
-   **Control de Acceso:** Activa un relé para abrir una puerta cuando se presenta una tarjeta autorizada.
-   **Sincronización de Hora:** Mantiene la hora actualizada mediante un cliente NTP.

## Flujo de la Aplicación

1.  **Inicio:** El ESP8266 se inicializa, conecta a la red WiFi configurada y carga los datos (usuarios, registros) desde la memoria SPIFFS.
2.  **Servidores:** Inicia dos servidores:
    *   Un **servidor TCP** en el puerto `5010` para escuchar los comandos del protocolo Anviz enviados por CrossChex.
    *   Un **servidor Web** en el puerto `80` para la interfaz de usuario web.
3.  **Operación Normal:**
    *   **Lectura de Tarjeta:** Cuando un usuario pasa una tarjeta por el lector Wiegand, el ESP8266 lee el ID.
    *   **Verificación:** Busca el ID de la tarjeta en la lista de usuarios almacenada.
    *   **Acceso:** Si el usuario está autorizado, activa un relé (para abrir una puerta) durante unos segundos y guarda un registro del acceso. Si no, el acceso es denegado.
    *   **Gestión con CrossChex:** Es posible añadir, modificar o eliminar usuarios desde el software CrossChex, que enviará los comandos correspondientes al emulador.
    *   **Monitorización Web:** Se puede acceder a la IP del dispositivo desde un navegador para ver el estado del sistema y los registros en tiempo real.

## Diagrama de Conexiones

A continuación se describe cómo conectar los componentes a una placa NodeMCU (ESP8266).

```
+---------------------+      +---------------------+      +---------------------+
|   Lector Wiegand    |      |    NodeMCU (ESP8266)  |      |    Módulo Relé (5V) |
|---------------------|      |---------------------|      |---------------------|
|        VCC (5V) ----|----&gt; |         VIN (5V)    | &lt;----|---- VCC            |
|             GND ----|----&gt; |         GND         | &lt;----|---- GND            |
|              D0 ----|----&gt; |         D7          |      |                     |
|              D1 ----|----&gt; |         D6          |      |              IN ----|----&gt; D1                   |
+---------------------+      +---------------------+      +---------------------+
                               |                     |
                               |                     |
+---------------------+      |                     |
|         LED         |      |                     |
|---------------------|      |                     |
| ánodo (+) --[R:220Ω]-|----&gt; |         D0          |
| cátodo (-) ---------|----&gt; |         GND         |
+---------------------+      +---------------------+
```

**Notas:**
-   El lector Wiegand y el relé pueden requerir 5V. Conéctalos al pin `VIN` del NodeMCU si lo alimentas por USB.
-   Se recomienda usar una resistencia de 220Ω o 330Ω en serie con el LED para limitar la corriente.

## Dependencias

Para compilar este proyecto, necesitas tener instalado el soporte para placas ESP8266 en tu Arduino IDE y las siguientes bibliotecas:

-   `ArduinoJson` by Benoit Blanchon (se recomienda la versión 6)
-   `NTPClient` by Fabrice Weinberg

Puedes instalarlas desde el **Gestor de Librerías** del Arduino IDE.

## Configuración e Inicialización

1.  **Hardware:** Monta el circuito como se muestra en el diagrama de conexiones.
2.  **Software:**
    *   Abre el archivo `Anviz-ESP8266.ino` en el Arduino IDE.
    *   Modifica las credenciales de tu red WiFi en las siguientes líneas:
        ```cpp
        const char* wifi_ssid = "TU_SSID";
        const char* wifi_password = "TU_PASSWORD";
        ```
    *   Instala las bibliotecas mencionadas en la sección de "Dependencias".
    *   Selecciona tu placa (ej. "NodeMCU 1.0 (ESP-12E Module)") y el puerto COM correcto.
3.  **Carga:** Sube el código a tu placa ESP8266.
4.  **Uso:**
    *   Abre el **Monitor Serie** (velocidad 115200) para ver los mensajes de inicio y la dirección IP asignada al dispositivo.
    *   Accede a la interfaz web escribiendo la dirección IP en tu navegador.
    *   Para gestionar usuarios, utiliza el software **Anviz CrossChex**. Añade un nuevo dispositivo introduciendo la dirección IP del emulador y el puerto `5010`.
