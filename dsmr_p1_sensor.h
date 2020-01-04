#include "esphome.h"
#include "dsmr.h"

using namespace esphome;

#define P1_MAXTELEGRAMLENGTH 1500
#define DELAY_MS 60000 // Delay in miliseconds before reading another telegram
#define WAIT_FOR_DATA_MS 2000

// Use data structure according to: https://github.com/matthijskooijman/arduino-dsmr

using MyData = ParsedData <
  /* FixedValue */ energy_delivered_tariff1,
  /* FixedValue */ energy_delivered_tariff2,
  /* FixedValue */ energy_returned_tariff1,
  /* FixedValue */ energy_returned_tariff2,
  /* FixedValue */ power_delivered,
  /* FixedValue */ power_returned,
  /* FixedValue */ voltage_l1,
  /* FixedValue */ voltage_l2,
  /* FixedValue */ voltage_l3,
  /* FixedValue */ current_l1,
  /* FixedValue */ current_l2,
  /* FixedValue */ current_l3,
  /* FixedValue */ power_delivered_l1,
  /* FixedValue */ power_delivered_l2,
  /* FixedValue */ power_delivered_l3,
  /* FixedValue */ power_returned_l1,
  /* FixedValue */ power_returned_l2,
  /* FixedValue */ power_returned_l3,
  /* uint16_t */ gas_device_type,
  /* uint8_t */ gas_valve_position,
  /* TimestampedFixedValue */ gas_delivered
>;

class CustomP1UartComponent : public Component, public uart::UARTDevice {
 protected:
   char telegram[P1_MAXTELEGRAMLENGTH];
   char c;
   int telegramlen;
   bool headerfound;
   bool footerfound;
   unsigned long lastread;
   int bytes_read;
  
  bool data_available() {
	// See if there's data available.
	unsigned long currentMillis = millis();
	unsigned long previousMillis = currentMillis; 
  
	while (currentMillis - previousMillis < WAIT_FOR_DATA_MS) { // wait in miliseconds
		if (available()) {
			return true;
		}
	}
	return false;  
  }  
   
  bool read_message() {
	//ESP_LOGD("DmsrCustom","Read message");
	headerfound = false;
	footerfound = false;
	telegramlen = 0;
	bytes_read = 0;
	unsigned long currentMillis = millis();
	unsigned long previousMillis = currentMillis; 	
	
	if (available()) { // Check to be sure
		// Messages come in batches. Read until footer.
		while (!footerfound && currentMillis - previousMillis < 5000) { // Loop while there's no footer found with a maximum of 5 seconds
			// Loop while there's data to read
			while (available()) { // Loop while there's data 
				if (telegramlen >= P1_MAXTELEGRAMLENGTH) {  // Buffer overflow
					headerfound = false;
					footerfound = false;
					ESP_LOGD("DmsrCustom","Error: Message larger than buffer");
				}
				bytes_read++;
				c = read();
				if (c == 47) { // header: forward slash
					// ESP_LOGD("DmsrCustom","Header found");
					headerfound = true;
					telegramlen = 0;
				}
				if (headerfound) {
					telegram[telegramlen] = c;
					telegramlen++;
					if (c == 33) { // footer: exclamation mark
						ESP_LOGD("DmsrCustom","Footer found");
						footerfound = true;
					} else {
						if (footerfound && c == 10) { // last \n after footer
							// Parse message
							MyData data;
							// ESP_LOGD("DmsrCustom","Trying to parse");
							ParseResult<void> res = P1Parser::parse(&data, telegram, telegramlen, false); // Parse telegram accoring to data definition. Ignore unknown values.
							if (res.err) {
								// Parsing error, show it
								Serial.println(res.fullError(telegram, telegram + telegramlen));
							} else {
								publish_sensors(data);
								return true; // break out function
							}
						}	
					}
				} 
			} // While data available	
		} // !footerfound	
	} 	
	return false;	  
  }

  void publish_sensors(MyData data){
	if(data.energy_delivered_tariff1_present)s_energy_delivered_tariff1->publish_state(data.energy_delivered_tariff1);
	if(data.energy_delivered_tariff2_present)s_energy_delivered_tariff2->publish_state(data.energy_delivered_tariff2);
	if(data.energy_returned_tariff1_present)s_energy_returned_tariff1->publish_state(data.energy_returned_tariff1);
	if(data.energy_returned_tariff2_present)s_energy_returned_tariff2->publish_state(data.energy_returned_tariff2);
	if(data.power_delivered_present)s_power_delivered->publish_state(data.power_delivered);
	if(data.power_returned_present)s_power_returned->publish_state(data.power_returned);
	if(data.voltage_l1_present)s_voltage_l1->publish_state(data.voltage_l1);
	if(data.voltage_l2_present)s_voltage_l2->publish_state(data.voltage_l2);
	if(data.voltage_l3_present)s_voltage_l3->publish_state(data.voltage_l3);
	if(data.current_l1_present)s_current_l1->publish_state(data.current_l1);
	if(data.current_l2_present)s_current_l2->publish_state(data.current_l2);
	if(data.current_l3_present)s_current_l3->publish_state(data.current_l3);
	if(data.power_delivered_l1_present)s_power_delivered_l1->publish_state(data.power_delivered_l1);
	if(data.power_delivered_l2_present)s_power_delivered_l2->publish_state(data.power_delivered_l2);
	if(data.power_delivered_l3_present)s_power_delivered_l3->publish_state(data.power_delivered_l3);
	if(data.power_returned_l1_present)s_power_returned_l1->publish_state(data.power_returned_l1);
	if(data.power_returned_l2_present)s_power_returned_l2->publish_state(data.power_returned_l2);
	if(data.power_returned_l3_present)s_power_returned_l3->publish_state(data.power_returned_l3);
	if(data.gas_device_type_present)s_gas_device_type->publish_state(data.gas_device_type);
	if(data.gas_valve_position_present)s_gas_valve_position->publish_state(data.gas_valve_position);
	if(data.gas_delivered_present)s_gas_delivered->publish_state(data.gas_delivered);
  };  
   
 public:
  CustomP1UartComponent(UARTComponent *parent) : UARTDevice(parent) {}
  Sensor *s_energy_delivered_tariff1 = new Sensor();
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
  Sensor *s_gas_valve_position = new Sensor();
  Sensor *s_gas_delivered = new Sensor(); 

  void setup() override {
    lastread = 0;
	pinMode(D5, OUTPUT); // Set D5 as output pin
	digitalWrite(D5,LOW); // Set low, don't request message from P1 port
  }
  
  void loop() override {
	unsigned long now = millis();
	
	if (now - lastread > DELAY_MS || lastread == 0) {
		lastread = now;
		digitalWrite(D5,HIGH); // Set high, request new message from P1 port
		if (data_available()) { // Check for x seconds if there's data available
			bool have_message = read_message();
			if (have_message) { 
				digitalWrite(D5,LOW); // Set low, stop requesting messages from P1 port
			} // If No message was read, keep output port high and retry later
		} else {
				ESP_LOGD("DmsrCustom","No data available. Is P1 port connected?");
		}	
	}
  }	

};
