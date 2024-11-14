// 使用 4.6.0  版本的库 RadioLib 在文件夹： C:\Users\maker\Documents\Arduino\libraries\RadioLib
// 使用 2.0.0  版本的库 SPI 在文件夹： C:\Users\maker\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.14\libraries\SPI
// 使用 2.0.0  版本的库 WiFi 在文件夹： C:\Users\maker\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.14\libraries\WiFi

#include <RadioLib.h> //LORA库
#include <SPI.h>

#include "Arduino.h"
#include "WiFiMulti.h"
#include "driver/i2s.h"
#include "config.h"
#include "web_server.h"

SX1276 radio = new Module(LORA_CS, DIO0, LORA_RST, DIO1, SPI, SPISettings());


// Sensor Config
char sensor_id[NVS_DATA_LENGTH];
char sleep_time[NVS_DATA_LENGTH];

int count=0;

unsigned char resp[80] = {0};

float analogValue = 0.0;
//float analogVolts = 0.0;
float bat_vol = 0.0;

RTC_DATA_ATTR int bootCount = 0;

void setup()
{
    pin_init();

    Serial.begin(115200);

    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));
    print_wakeup_reason();

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    lora_init();
    I2S_INIT();
    analogReadResolution(12); //设置ADC分辨率为12位，输入电压映射到0-4095的值
    // Button Check
    read_bat();
    wifi_init();
    delay(5000);
}

void loop()
{
    // Send three time and sleep
    for (int i = 0; i < 15; i++)
    {
        // Serial.println(lora_msg_create());

        // String temp = "";

        // temp = sensor_read();
        sensor_read();
        read_bat();
        value_log();
        if (i > 13)
        {
            // temp = lora_msg_create(temp);
            // lora_send_task(temp);
            check_count(&count);
            record_count(++count);
            lora_send_task(json_create());
        }

        delay(2000);
    }

    int sleep_s = atoi(sleep_time); //将数组转化为整数值
    esp_sleep_enable_timer_wakeup(sleep_s * uS_TO_S_FACTOR); //设置睡眠模式为时间唤醒
    Serial.println("Setup ESP32 to sleep for every " + String(sleep_s) + " Seconds");

    // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    // Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
    
    Serial.println("Going to sleep now");
    Serial.flush();//确保深度睡眠时数据发送完毕
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
}

// Hardware init

void pin_init()
{
    pinMode(POWER_LORA, OUTPUT);
    pinMode(LORA_CS, OUTPUT);
    pinMode(BAT_PIN, INPUT);
    digitalWrite(POWER_LORA, HIGH);
    delay(100);
}


void lora_init()
{
    Serial.print(F("[SX1276] Initializing ... "));

    int state = radio.begin(FREQUENCY, BANDWIDTH, SPREADING_FACTOR, CODING_RATE, SX127X_SYNC_WORD, OUTPUT_POWER, PREAMBLE_LEN, GAIN);
    if (state == ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true)
            ;
    }
}

// Lora send task
String lora_msg_create(String sensor_data)
{
    String temp = "ID:";
    temp = temp + sensor_id + ",SLEEP:" + sleep_time + "," + sensor_data;

    return temp;
}

String json_create()
{
    String temp = "{";
    temp = temp + "\"ID\":\"" + sensor_id + "\",\"COUNT\":" + count + ",\"SLEEP\":" + sleep_time + ",";
    temp = temp + "\"bat\":" + bat_vol + ",";
    temp = temp + "\"ADC\":" + analogValue + "}";
    //temp = temp + "\"ADC_millivolts_value\":" + analogVolts + "}";

    return temp;
}

void lora_send_task(String data)
{
    Serial.println(data);
    int state = radio.transmit(data.c_str());
    if (state == ERR_NONE)
    {
        // the packet was successfully transmitted
        Serial.println(F(" success!"));

        // print measured data rate
        Serial.print(F("[SX1276] Datarate:\t"));
        Serial.print(radio.getDataRate());
        Serial.println(F(" bps"));
    }
    else if (state == ERR_PACKET_TOO_LONG)
    {
        // the supplied packet was longer than 256 bytes
        Serial.println(F("too long!"));
    }
    else if (state == ERR_TX_TIMEOUT)
    {
        // timeout occurred while transmitting packet
        Serial.println(F("timeout!"));
    }
    else
    {
        // some other error occurred
        Serial.print(F("failed, code "));
        Serial.println(state);
    }
}

// Soil Sensor
String sensor_read()
{
    analogValue = analogRead(sensorADCPin);
    //analogVolts = analogReadMilliVolts(sensorADCPin);

    String str = "ADC_analog_value:";
    str = str + analogValue;
    //str = str + analogValue + ",ADC_millivolts_value:" + analogVolts;

    return str;
}


void value_log()
{
    Serial.print("ADC: ");
    Serial.println(analogValue);
    //Serial.print("ADC millivolts value =");
    //Serial.println(analogVolts);
    Serial.print("Bat: ");
    Serial.print(bat_vol);
    Serial.println(" V");
}

int CaculateValue(int x, int y)
{
    int t = 0;
    t = x * 256;
    t = t + y;
    return t;
}

void print_wakeup_reason()
{
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
}

float c2f(float c_temp)
{
    return c_temp * 9.0 / 5.0 + 32;
}

void read_bat()
{
    bat_vol = 3.3 * analogRead(BAT_PIN) / 4096 * 2;
}

void I2S_INIT(void) 
{
    //i2s configuration
    uint8_t m_i2s_num = I2S_NUM_0; // i2s port number
    i2s_config_t i2s_config = {
         .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
         .sample_rate = 48000,
         .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,//I2S_BITS_PER_SAMPLE_16BIT,//
         .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
         .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
         .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
         .dma_buf_count = 8,  // max buffers
         .dma_buf_len = 1024, // max value
         .use_apll = 0,//APLL_DISABLE,
         .tx_desc_auto_clear= true,  // new in V1.0.1
         .fixed_mclk=-1
    };
    i2s_driver_install((i2s_port_t)m_i2s_num, &i2s_config, 0, NULL);
    
    i2s_pin_config_t pins = {
      .bck_io_num = I2S_BCLK,           // Bit Clock
      .ws_io_num =  I2S_LRC,            //  wclk,// Left/Right Clock
      .data_out_num = I2S_DOUT,         // Data Out
      .data_in_num = -1
    };
    i2s_set_pin((i2s_port_t)m_i2s_num, &pins);
}

esp_err_t I2Sstart(uint8_t i2s_num) {
    return i2s_start((i2s_port_t) i2s_num);
}

esp_err_t I2Sstop(uint8_t i2s_num) {
    return i2s_stop((i2s_port_t) i2s_num);
}
