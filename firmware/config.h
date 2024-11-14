#ifndef CONFIG_H
#define CONFIG_H

#define AP_SSID "Makerfabs-"
#define AP_PWD "12345678"

#define DEFAULT_SENSOR_ID "SoilM01"
#define DEFAULT_SLEEP_TIME "3600"

#define SUCCESS 1
#define ERROR 0

#define NVS_DATA_LENGTH 20

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

#define BUTTON_PIN 1
#define BAT_PIN 14

// Lora
#define LORA_RST 5
#define LORA_CS 4

#define DIO0 6
#define DIO1 7

#define SPI_MOSI 11
#define SPI_MISO 13
#define SPI_SCK 12

#define FREQUENCY 868.0
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 12
#define CODING_RATE 6
#define OUTPUT_POWER 20
#define PREAMBLE_LEN 8
#define GAIN 0


// Power Control
#define POWER_LORA 21

#define I2S_DOUT  47
#define I2S_BCLK  48
#define I2S_LRC   45

#define  sensorADCPin 9

#endif
