#include <Arduino.h>
#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>
#include <VL53L0X.h>

#include <stale.h>
#include <led.h>
#include <tof.h>

// Set to true to reset eeprom before to write something
#define RESET_EEPROM true

#ifndef STASSID
#define STASSID "Nodemcu-CENTRALA"
#define STAPSK "systemwykrywaniaotwieraniadrzwi"
#endif

VL53L0X tofsensor;

const char *ssid = STASSID;
const char *password = STAPSK;

const char *host = "192.168.8.5";
const uint16_t port = 80;

bool tryb_konfiguracji = false;
bool konfiguracja_zakonczona = false;

unsigned long timer1 = millis();

IPAddress local_IP(192, 168, 8, 6);
IPAddress gateway(192, 168, 8, 1);
IPAddress mask(255, 255, 255, 0);

struct
{
	uint32_t crc32;	  // 4 bytes
	uint8_t channel;  // 1 byte,   5 in total
	uint8_t bssid[6]; // 6 bytes, 11 in total
	uint8_t padding;  // 1 byte,  12 in total
} rtcData;

uint8_t bssid[6] = {0x52, 0x02, 0x91, 0xDA, 0x5F, 0x9b}; // 52:02:91:da:5f:9b

void ustawienia_pinow()
{
	pinMode(IR_OUT, INPUT);
	pinMode(LED_VCC, OUTPUT);
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
}

/*

void eeprom_conf()
{
	EEPROM.begin(2048);
	Serial.println("");

	Serial.println("");
	Serial.println("Check writing");
	String ssid;
	for (int k = addr_ssid; k < addr_ssid + 20; ++k)
	{
		ssid += char(EEPROM.read(k));
	}
	Serial.print("SSID: ");
	Serial.println(ssid);

	String password;
	for (int l = addr_password; l < addr_password + 20; ++l)
	{
		password += char(EEPROM.read(l));
	}
	Serial.print("PASSWORD: ");
	Serial.println(password);
}

void zapisz_do_eeprom(string wartosc)
{
	String a = wartosc;
}



*/

bool centrala_komunikacja()
{

	uint16_t zasieg_tof = tofsensor.readRangeSingleMillimeters();
	bool httpStan = false;

	WiFiClient client;
	HTTPClient http; //must be declared after WiFiClient for correct destruction order, because used by http.begin(client,...)

	Serial.print("connecting to ");
	Serial.print(host);
	Serial.print(':');
	Serial.println(port);

	// Odczytanie wartosci zapisanej odleflosci do obiektu
	uint16_t pamiec_wartosc = Tof::odczytaj();
	String endpoint = "http://192.168.8.5/drzwi_otwarte";
	Serial.print(String(pamiec_wartosc));
	Serial.print(" : ");
	Serial.println(String(zasieg_tof));

	if (zasieg_tof <= pamiec_wartosc + 10 && zasieg_tof >= pamiec_wartosc - 10)
	{
		// Odleglosc do obiektu zgadza sie z wartoscia zapisana
		endpoint = "http://192.168.8.5/drzwi_zamkniete";
	}

	// Wyslanie sygnalu do odbiornika
	Serial.print("[HTTP] Begin. ");
	Serial.println(endpoint);
	http.begin(client, endpoint);
	int httpCode = http.GET();

	if (httpCode > 0)
	{

		httpStan = Led::HttpStan(httpCode == HTTP_CODE_OK);
		// file found at server
		if (httpStan)
		{
			Serial.println("[HTTP] HTTP_CODE_OK.");
			Serial.println("[HTTP] zamknięto połączenie lub zakończenie pliku.");
		}
	}
	else
	{
		httpStan = Led::HttpStan(false);
		Serial.printf("[HTTP] GET... niepowodzenie, error: %s\n", http.errorToString(httpCode).c_str());
	}

	http.end();

	return httpStan;
}

// uint32_t calculateCRC32(const uint8_t *data, size_t length)
// {
// 	uint32_t crc = 0xffffffff;
// 	while (length--)
// 	{
// 		uint8_t c = *data++;
// 		for (uint32_t i = 0x80; i > 0; i >>= 1)
// 		{
// 			bool bit = crc & 0x80000000;
// 			if (c & i)
// 			{
// 				bit = !bit;
// 			}

// 			crc <<= 1;
// 			if (bit)
// 			{
// 				crc ^= 0x04c11db7;
// 			}
// 		}
// 	}

// 	return crc;
// }

void setup()
{

	// put your setup code here, to run once:
	Serial.begin(115200);

	// byte a = 100;
	// byte b = 100 >> 8;

	// Serial.println(a);
	// Serial.println(b);

	ustawienia_pinow();

	Wire.begin(TOF_SDA, TOF_SCL);

	digitalWrite(LED_RED, 1);
	digitalWrite(LED_GREEN, 1);
	digitalWrite(LED_BLUE, 1);
	digitalWrite(LED_VCC, 1);

	while (!tofsensor.init())
	{
		Serial.println("Nie mozna zaladowac VL53L0X");
		Led::ToF_nieznaleziono(false);
		delay(500);
	}

	if (analogRead(A0) > 100)
	{
		Serial.print("Analog read: ");
		Serial.println(String(analogRead(A0)));
		tryb_konfiguracji = true;
		return;
	}

	WiFi.disconnect();

	// Serial.println();
	// Serial.println();
	// Serial.print("Connecting to ");
	// Serial.println(ssid);

	/* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
	// WiFi.mode(WIFI_STA);
	// WiFi.config(local_IP, gateway, mask);

	// bool rtcValid = false;
	// if (ESP.rtcUserMemoryRead(0, (uint32_t *)&rtcData, sizeof(rtcData)))
	// {
	// 	// Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
	// 	uint32_t crc = calculateCRC32(((uint8_t *)&rtcData) + 4, sizeof(rtcData) - 4);
	// 	if (crc == rtcData.crc32)
	// 	{
	// 		rtcValid = true;
	// 	}
	// }

	// if (rtcValid)
	// {

	WiFi.begin(ssid, password, 1, bssid);

	// }
	// else
	// {
	// 	WiFi.begin(ssid, password);
	// }

	// WiFi.persistent(true);
	// WiFi.setAutoConnect(true);

	// int retries = 0;
	while (WiFi.status() != WL_CONNECTED)
	{
		// retries++;
		// if (retries == 100)
		// {
		// 	// Quick connect is not working, reset WiFi and try regular connection
		// 	WiFi.disconnect();
		// 	delay(10);
		// 	WiFi.forceSleepBegin();
		// 	delay(10);
		// 	WiFi.forceSleepWake();
		// 	delay(10);
		// 	WiFi.begin(ssid, password);
		// }

		Led::LaczenieWiFi(false);
	}

	// rtcData.channel = WiFi.channel();
	// memcpy(rtcData.bssid, WiFi.BSSID(), 6); // Copy 6 bytes of BSSID (AP's MAC address)
	// rtcData.crc32 = calculateCRC32(((uint8_t *)&rtcData) + 4, sizeof(rtcData) - 4);
	// ESP.rtcUserMemoryWrite(0, (uint32_t *)&rtcData, sizeof(rtcData));
}

void loop()
{

	// Praca z opźnieniem czasowym 50ms
	if (millis() - timer1 < 1000)
		return;

	timer1 = millis();

	uint16_t zasieg_tof = tofsensor.readRangeSingleMillimeters();
	//Serial.println(String(zasieg_tof));
	//delay(50);
	//return;

	if (tryb_konfiguracji)
	{
		Led::Konfiguracja(konfiguracja_zakonczona);

		if (konfiguracja_zakonczona)
		{
			//uint8_t tof_val = Tof::odczytaj();
			return;
		}

		//zapisz_do_eeprom(String(zasieg_tof));
		Tof::zapisz(zasieg_tof);

		Serial.print("Zapis: ");
		Serial.println(String(zasieg_tof));

		konfiguracja_zakonczona = true;

		return;
	}

	bool httpStan = false;

	if (WiFi.status() == WL_CONNECTED)
	{
		httpStan = centrala_komunikacja();
		Led::LaczenieWiFi(true);

		// Przejście w stan deepSleep
		// jeżeli odpowiedź serwera ma status 200
		if (httpStan)
		{
			digitalWrite(LED_VCC, 0);
			ESP.deepSleep(0, WAKE_RF_DEFAULT);
		}
	}

	//Led::LaczenieWiFi(false);
}