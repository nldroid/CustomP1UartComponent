#include "esphome.h"
#include <SoftwareSerial.h>
// https://github.com/matthijskooijman/arduino-dsmr
#include "dsmr.h"

// * Baud rate for both hardware and software serial
#define BAUD_RATE 115200
#define SWSERIAL_BAUD_RATE 115200

// * Max telegram length
#define P1_MAXLINELENGTH 75
#define P1_MAXTELEGRAMLENGTH 1500

// * P1 Meter RX pin
#define P1_SERIAL_RX D2

#define USE_7E1 0

using MyData = ParsedData<
  /* String */ p1_version,
  /* FixedValue */ energy_delivered_tariff1
>;

// * Initiate Software Serial
SoftwareSerial p1_serial(P1_SERIAL_RX, SW_SERIAL_UNUSED_PIN, true, P1_MAXLINELENGTH); // (RX, TX, inverted, buffer)

class DsmrP1CustomSensor : public Component {
 public:
   // * Set to store received telegram
  char line[P1_MAXLINELENGTH];
  char telegram[P1_MAXTELEGRAMLENGTH];
  int telegramlen;
  
  Sensor *s_p1_version = new Sensor();
/*  Sensor *s_energy_delivered_tariff1 = new Sensor();
  Sensor *s_energy_delivered_tariff2 = new Sensor();
  Sensor *s_energy_returned_tariff1 = new Sensor();
  Sensor *s_energy_returned_tariff2 = new Sensor();
  Sensor *s_electricity_tariff = new Sensor();
  Sensor *s_power_delivered = new Sensor();
  Sensor *s_power_returned = new Sensor();
  Sensor *s_electricity_threshold = new Sensor();
  Sensor *s_voltage_l1 = new Sensor();
  Sensor *s_voltage_l2 = new Sensor();
  Sensor *s_voltage_l3 = new Sensor();
  Sensor *s_current_l1 = new Sensor();
  Sensor *s_current_l2 = new Sensor();
  Sensor *s_current_l3 = new Sensor();
  Sensor *s_power_delivered_l1 = new Sensor();
  Sensor *s_power_delivered_l2 = new Sensor();
  Sensor *s_power_delivered_l3 = new Sensor();
  Sensor *s_power_returned_l1 = new Sensor();
  Sensor *s_power_returned_l2 = new Sensor();
  Sensor *s_power_returned_l3 = new Sensor();
  Sensor *s_gas_device_type = new Sensor();
  Sensor *s_gas_equipment_id = new Sensor();
  Sensor *s_gas_valve_position = new Sensor();
  Sensor *s_gas_delivered = new Sensor(); 
*/
			
  // DsmrP1CustomSensor() : PollingComponent(1000) { }
  DsmrP1CustomSensor() {}
 
	/*
	Serial.print(Item::name);
	Serial.print(F(": "));
	Serial.print(i.val());
	Serial.print(Item::unit());
	Serial.println();
	*/
 
/*	struct Publish {
		template<typename Item>
		void apply(Item &i) {
			if (i.present()) {
				switch(Item::name) {
					case "p1_version":
						s_p1_version->publish_state(i.val);
						break;
					case "energy_delivered_tariff1":
						s_energy_delivered_tariff1->publish_state(i.val);
						break;
					case "energy_delivered_tariff2":
						s_energy_delivered_tariff2->publish_state(i.val);
						break;
					case "energy_returned_tariff1":
						s_energy_returned_tariff1->publish_state(i.val);
						break;
					case "energy_returned_tariff2":
						s_energy_returned_tariff2->publish_state(i.val);
						break;
					case "electricity_tariff":
						s_electricity_tariff->publish_state(i.val);
						break;
					case "power_delivered":
						s_power_delivered->publish_state(i.val);
						break;
					case "power_returned":
						s_power_returned->publish_state(i.val);
						break;
					case "electricity_threshold":
						s_electricity_threshold->publish_state(i.val);
						break;
					case "voltage_l1":
						s_voltage_l1->publish_state(i.val);
						break;
					case "voltage_l2":
						s_voltage_l2->publish_state(i.val);
						break;
					case "voltage_l3":
						s_voltage_l3->publish_state(i.val);
						break;
					case "current_l1":
						s_current_l1->publish_state(i.val);
						break;
					case "current_l2":
						s_current_l2->publish_state(i.val);
						break;
					case "current_l3":
						s_current_l3->publish_state(i.val);
						break;
					case "power_delivered_l1":
						s_power_delivered_l1->publish_state(i.val);
						break;
					case "power_delivered_l2":
						s_power_delivered_l2->publish_state(i.val);
						break;
					case "power_delivered_l3":
						s_power_delivered_l3->publish_state(i.val);
						break;
					case "power_returned_l1":
						s_power_returned_l1->publish_state(i.val);
						break;
					case "power_returned_l2":
						s_power_returned_l2->publish_state(i.val);
						break;
					case "power_returned_l3":
						s_power_returned_l3->publish_state(i.val);
						break;
					case "gas_device_type":
						s_gas_device_type->publish_state(i.val);
						break;
					case "gas_equipment_id":
						s_gas_equipment_id->publish_state(i.val);
						break;
					case "gas_valve_position":
						s_gas_valve_position->publish_state(i.val);
						break;
					case "gas_delivered:
						s_gas_delivered->publish_state(i.val);					
						break; 
				} 
			}
		}
	};
*/

  void setup() override {
	Serial.begin(BAUD_RATE);
	
    // * Start software serial for p1 meter
    p1_serial.begin(SWSERIAL_BAUD_RATE);
  }

  void loop() override {
	
	
    if (p1_serial.available())
    {
      memset(line, 0, sizeof(line));

      while (p1_serial.available())
      {
        ESP.wdtDisable();

        int len = p1_serial.readBytesUntil('\n', line, P1_MAXLINELENGTH); // Read line for line
        
		if (USE_7E1 == 1) 
		{
			// Shift bits for parity
			for (int cnt = 0; cnt < len; cnt++)
			line[cnt] &= ~(1 << 7);	
        }
		
        ESP.wdtEnable(1);

        line[len] = '\n';
		
        yield();
		
		if (line[0] == 47) // Slash found, start of telegram
		{ 
			telegramlen = 0;
			memset(telegram, 0, sizeof(telegram)); // Make empty
		}
		
		// Copy line data to telegram
		for (int i = 0; i < len; i++)
		{
			telegram[telegramlen + i] = line[i];
		}
		telegramlen = telegramlen + len;
		
		if (line[0] == 33) // Exclamation mark found, end of telegram
		{ 
			MyData data;
			  
			ParseResult<void> res = P1Parser::parse(&data, telegram, telegramlen, true);
			if (res.err) {
				// Parsing error, show it
				Serial.println(res.fullError(telegram, telegram + telegramlen));
			} else {
				// Parsed succesfully, print all values
				//data.applyEach(Publish());
			}		
		}
      }
	}  
  }
  
 private:

  
};
