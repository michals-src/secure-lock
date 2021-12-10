#include <Arduino.h>
#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>
#include <VL53L0X.h>

#include <stale.h>
#include <led.h>

int addr_ssid = 0;						// ssid index
int addr_password = 20;					// password index
String ssid = "wifi_ssid";				// wifi ssid
String password = "wifi_password_demo"; // and password

// Set to true to reset eeprom before to write something
#define RESET_EEPROM true

#ifndef STASSID
#define STASSID "ESPOdbiornik"
#define STAPSK "systemwykrywaniaotwieraniadrzwi"
#endif

VL53L0X tofsensor;

const char *ssid = STASSID;
const char *password = STAPSK;

const char *host = "192.168.8.5";
const uint16_t port = 80;

void ustawienia_pinow()
{
	pinMode(IR_OUT, INPUT);
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
}

void eeprom_conf()
{
	EEPROM.begin(512);
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

void setup()
{

	// put your setup code here, to run once:
	Serial.begin(115200);
	ustawienia_pinow();

	Wire.begin(TOF_SDA, TOF_SCL);

	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	/* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	if (!tofsensor.init())
		Serial.println("Nie mozna zaladowac VL53L0X");

	if (analogRead(A0) < 100)
	{

		for (int i = 0; i < 512; i++)
		{
			EEPROM.write(i, 0);
		}
		EEPROM.commit();
		delay(500);

		Serial.println("");
		Serial.print("Write WiFi SSID at address ");
		Serial.println(addr_ssid);
		Serial.print("");
		for (int i = 0; i < ssid.length(); ++i)
		{
			EEPROM.write(addr_ssid + i, ssid[i]);
			Serial.print(ssid[i]);
			Serial.print("");
		}

		Serial.println("");
		Serial.print("Write WiFi Password at address ");
		Serial.println(addr_password);
		Serial.print("");
		for (int j = 0; j < password.length(); j++)
		{
			EEPROM.write(addr_password + j, password[j]);
			Serial.print(password[j]);
			Serial.print("");
		}

		Serial.println("");
		if (EEPROM.commit())
		{
			Serial.println("Data successfully committed");
		}
		else
		{
			Serial.println("ERROR! Data commit failed");
		}

		return;
	}
}

void loop()
{

	uint16_t zasieg_tof = tofsensor.readRangeSingleMillimeters();

	static bool wait = false;

	Serial.print("connecting to ");
	Serial.print(host);
	Serial.print(':');
	Serial.println(port);

	WiFiClient client;
	HTTPClient http; //must be declared after WiFiClient for correct destruction order, because used by http.begin(client,...)

	Serial.print("[HTTP] begin...\n");

	// configure server and url
	http.begin(client, "http://192.168.8.5/");
	//http.begin(client, "jigsaw.w3.org", 80, "/HTTP/connection.html");

	Serial.print("[HTTP] GET...\n");
	// start connection and send HTTP header
	int httpCode = http.GET();
	if (httpCode > 0)
	{
		// HTTP header has been send and Server response header has been handled
		Serial.printf("[HTTP] GET... code: %d\n", httpCode);

		// file found at server
		if (httpCode == HTTP_CODE_OK)
		{

			// get length of document (is -1 when Server sends no Content-Length header)
			int len = http.getSize();

			// create buffer for read
			uint8_t buff[128] = {0};

#if 0
        // with API
        Serial.println(http.getString());
#else
			// or "by hand"

			// get tcp stream
			WiFiClient *stream = &client;

			// read all data from server
			while (http.connected() && (len > 0 || len == -1))
			{
				// read up to 128 byte
				int c = stream->readBytes(buff, std::min((size_t)len, sizeof(buff)));
				Serial.printf("readBytes: %d\n", c);
				if (!c)
				{
					Serial.println("read timeout");
				}

				// write it to Serial
				Serial.write(buff, c);

				if (len > 0)
				{
					len -= c;
				}
			}
#endif

			Serial.println();
			Serial.print("[HTTP] connection closed or file end.\n");
		}
	}
	else
	{
		Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
	}

	http.end();
}