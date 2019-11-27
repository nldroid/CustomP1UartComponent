#include "esphome.h"
#include <SoftwareSerial.h>

// * Baud rate for both hardware and software serial
#define BAUD_RATE 115200

// * Max telegram length
#define P1_MAXLINELENGTH 64

// * P1 Meter RX pin
#define P1_SERIAL_RX D2

// * Initiate Software Serial
SoftwareSerial p1_serial(P1_SERIAL_RX, -1, true, P1_MAXLINELENGTH); // (RX, TX. inverted, buffer)

class DsmrP1CustomSensor : public PollingComponent {
 public:
   // * Set to store received telegram
  char telegram[P1_MAXLINELENGTH];
  
  // * Set to store the data values read
  long CONSUMPTION_LOW_TARIF;
  long CONSUMPTION_HIGH_TARIF;
  long ACTUAL_CONSUMPTION;
  long INSTANT_POWER_CURRENT;
  long INSTANT_POWER_USAGE;
  long GAS_METER_M3;
  
  // Set to store data counters read
  long ACTUAL_TARIF;
  long SHORT_POWER_OUTAGES;
  long LONG_POWER_OUTAGES;
  long SHORT_POWER_DROPS;
  long SHORT_POWER_PEAKS;
  
  // * Set during CRC checking
  unsigned int currentCRC = 0; 
  
  Sensor *consumption_low_tarif_sensor = new Sensor();
  Sensor *consumption_high_tarif_sensor = new Sensor();
  Sensor *actual_consumption_sensor = new Sensor();
  Sensor *instant_power_current_sensor = new Sensor();
  Sensor *instant_power_usage_sensor = new Sensor();
  Sensor *gas_meter_m3_sensor = new Sensor();
  Sensor *actual_tarif_sensor = new Sensor();
  Sensor *short_power_outages_sensor = new Sensor();
  Sensor *long_power_outages_sensor = new Sensor();
  Sensor *short_power_drops_sensor = new Sensor();
  Sensor *short_power_peaks_sensor = new Sensor();

  DsmrP1CustomSensor() : PollingComponent(15000) { }

  void setup() override {
	Serial.begin(BAUD_RATE);
	
    // * Start software serial for p1 meter
    p1_serial.begin(BAUD_RATE);
	
	ESP_LOGD("DmsrCustom","Init baud rate %f", BAUD_RATE);
  }

  void update() override {
	
	ESP_LOGD("DmsrCustom","Updating..");	
	
    if (p1_serial.available())
    {
	  ESP_LOGD("DmsrCustom","Data ready..");	
	  
      memset(telegram, 0, sizeof(telegram));

      while (p1_serial.available())
      {
        ESP.wdtDisable();
        int len = p1_serial.readBytesUntil('\n', telegram, P1_MAXLINELENGTH);
        ESP.wdtEnable(1);

        telegram[len] = '\n';
        telegram[len + 1] = 0;
        yield();    

        bool result = decode_telegram(len + 1);
        if (result)
        {  
		  ESP_LOGD("DmsrCustom","CRC Ok");	
		  
          consumption_low_tarif_sensor->publish_state(CONSUMPTION_LOW_TARIF);
          consumption_high_tarif_sensor->publish_state(CONSUMPTION_HIGH_TARIF);
          actual_consumption_sensor->publish_state(ACTUAL_CONSUMPTION);
          instant_power_current_sensor->publish_state(INSTANT_POWER_CURRENT);
          instant_power_usage_sensor->publish_state(INSTANT_POWER_USAGE);
          gas_meter_m3_sensor->publish_state(GAS_METER_M3);
		  
          actual_tarif_sensor->publish_state(ACTUAL_TARIF);
          short_power_outages_sensor->publish_state(SHORT_POWER_OUTAGES);
          long_power_outages_sensor->publish_state(LONG_POWER_OUTAGES);
          short_power_drops_sensor->publish_state(SHORT_POWER_DROPS);
          short_power_peaks_sensor->publish_state(SHORT_POWER_PEAKS);
        } else {
			ESP_LOGD("DmsrCustom","CRC Not Ok");	
		}
      }
	} else {
		ESP_LOGD("DmsrCustom","No data ready..");
	}  
  }
  
 private:
  unsigned int CRC16(unsigned int crc, unsigned char *buf, int len)
  {
  	for (int pos = 0; pos < len; pos++)
    {
  		crc ^= (unsigned int)buf[pos];  // * XOR byte into least sig. byte of crc
                        // * Loop over each bit
      for (int i = 8; i != 0; i--)
      {
        // * If the LSB is set
        if ((crc & 0x0001) != 0)
        {
          // * Shift right and XOR 0xA001
          crc >>= 1;
  				crc ^= 0xA001;
  			}
        // * Else LSB is not set
        else
          // * Just shift right
          crc >>= 1;
  		}
  	}
  	return crc;
  }
  
  bool isNumber(char *res, int len)
  {
    for (int i = 0; i < len; i++)
    {
      if (((res[i] < '0') || (res[i] > '9')) && (res[i] != '.' && res[i] != 0))
        return false;
    }
    return true;
  }
  
  int FindCharInArrayRev(char array[], char c, int len)
  {
    for (int i = len - 1; i >= 0; i--)
    {
      if (array[i] == c)
        return i;
    }
    return -1;
  }
  
  long getValue(char *buffer, int maxlen, char startchar, char endchar)
  {
    int s = FindCharInArrayRev(buffer, startchar, maxlen - 2);
    int l = FindCharInArrayRev(buffer, endchar, maxlen - 2) - s - 1;
  
    char res[16];
    memset(res, 0, sizeof(res));
  
    if (strncpy(res, buffer + s + 1, l))
    {
      if (endchar == '*')
      {
        if (isNumber(res, l))
          // * Lazy convert float to long
          return (1000 * atof(res));
      }
      else if (endchar == ')')
      {
        if (isNumber(res, l))
          return atof(res);
      }
    }
    return 0;
  }
  
  bool decode_telegram(int len)
  {
    int startChar = FindCharInArrayRev(telegram, '/', len);
    int endChar = FindCharInArrayRev(telegram, '!', len);
    bool validCRCFound = false;
  
    for (int cnt = 0; cnt < len; cnt++)
      Serial.print(telegram[cnt]);
  
    if (startChar >= 0)
    {
      // * Start found. Reset CRC calculation
      currentCRC = CRC16(0x0000,(unsigned char *) telegram+startChar, len-startChar);
    }
    else if (endChar >= 0)
    {
      // * Add to crc calc
      currentCRC = CRC16(currentCRC,(unsigned char*)telegram+endChar, 1);
  
      char messageCRC[5];
      strncpy(messageCRC, telegram + endChar + 1, 4);
  
      messageCRC[4] = 0;   // * Thanks to HarmOtten (issue 5)
      validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);
  
      if (validCRCFound)
        Serial.println(F("CRC Valid!"));
      else
        Serial.println(F("CRC Invalid!"));
  
      currentCRC = 0;
    }
    else
    {
      currentCRC = CRC16(currentCRC, (unsigned char*) telegram, len);
    }
  
    // 1-0:1.8.1(000992.992*kWh)
    // 1-0:1.8.1 = Elektra verbruik laag tarief (DSMR v4.0)
    if (strncmp(telegram, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0)
    {
      CONSUMPTION_LOW_TARIF = getValue(telegram, len, '(', '*');
    }
  
    // 1-0:1.8.2(000560.157*kWh)
    // 1-0:1.8.2 = Elektra verbruik hoog tarief (DSMR v4.0)
    if (strncmp(telegram, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0)
    {
      CONSUMPTION_HIGH_TARIF = getValue(telegram, len, '(', '*');
    }
  
    // 1-0:1.7.0(00.424*kW) Actueel verbruik
    // 1-0:2.7.0(00.000*kW) Actuele teruglevering
    // 1-0:1.7.x = Electricity consumption actual usage (DSMR v4.0)
    if (strncmp(telegram, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0)
    {
      ACTUAL_CONSUMPTION = getValue(telegram, len, '(', '*');
    }
  
    // 1-0:21.7.0(00.378*kW)
    // 1-0:21.7.0 = Instantaan vermogen Elektriciteit levering
    if (strncmp(telegram, "1-0:21.7.0", strlen("1-0:21.7.0")) == 0)
    {
      INSTANT_POWER_USAGE = getValue(telegram, len, '(', '*');
    }
  
    // 1-0:31.7.0(002*A)
    // 1-0:31.7.0 = Instantane stroom Elektriciteit
    if (strncmp(telegram, "1-0:31.7.0", strlen("1-0:31.7.0")) == 0)
    {
      INSTANT_POWER_CURRENT = getValue(telegram, len, '(', '*');
    }
  
    // 0-1:24.2.1(150531200000S)(00811.923*m3)
    // 0-1:24.2.1 = Gas (DSMR v4.0) on Kaifa MA105 meter
    if (strncmp(telegram, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0)
    {
      GAS_METER_M3 = getValue(telegram, len, '(', '*');
    }
  
    // 0-0:96.14.0(0001)
    // 0-0:96.14.0 = Actual Tarif
    if (strncmp(telegram, "0-0:96.14.0", strlen("0-0:96.14.0")) == 0)
    {
      ACTUAL_TARIF = getValue(telegram, len, '(', ')');
    }
  
    // 0-0:96.7.21(00003)
    // 0-0:96.7.21 = Aantal onderbrekingen Elektriciteit
    if (strncmp(telegram, "0-0:96.7.21", strlen("0-0:96.7.21")) == 0)
    {
      SHORT_POWER_OUTAGES = getValue(telegram, len, '(', ')');
    }
  
    // 0-0:96.7.9(00001)
    // 0-0:96.7.9 = Aantal lange onderbrekingen Elektriciteit
    if (strncmp(telegram, "0-0:96.7.9", strlen("0-0:96.7.9")) == 0)
    {
      LONG_POWER_OUTAGES = getValue(telegram, len, '(', ')');
    }
  
    // 1-0:32.32.0(00000)
    // 1-0:32.32.0 = Aantal korte spanningsdalingen Elektriciteit in fase 1
    if (strncmp(telegram, "1-0:32.32.0", strlen("1-0:32.32.0")) == 0)
    {
      SHORT_POWER_DROPS = getValue(telegram, len, '(', ')');
    }
  
    // 1-0:32.36.0(00000)
    // 1-0:32.36.0 = Aantal korte spanningsstijgingen Elektriciteit in fase 1
    if (strncmp(telegram, "1-0:32.36.0", strlen("1-0:32.36.0")) == 0)
    {
      SHORT_POWER_PEAKS = getValue(telegram, len, '(', ')');
    }
  
    return validCRCFound;
  } 
  
};
