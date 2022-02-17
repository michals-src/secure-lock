#pragma once

class Led
{
public:
    static unsigned long zapisCzas;
    static bool led_ostatni_stan;
    static uint8_t itor;

    // Dioda konfiguracji
    static void Konfiguracja(bool status)
    {
        //Stały kolor w trakcie trwania konfiguracji
        if (!status)
        {
            digitalWrite(LED_RED, 1);
            digitalWrite(LED_GREEN, 0);
            digitalWrite(LED_BLUE, 0);

            return;
        }

        //Miganie diody co 150ms, potwierdzajace zakończenie konfiguracji
        if (millis() - Led::zapisCzas > 300)
        {
            digitalWrite(LED_GREEN, led_ostatni_stan);
            digitalWrite(LED_BLUE, led_ostatni_stan);
            led_ostatni_stan = !led_ostatni_stan;

            Led::zapisCzas = millis();
        }
    }

    // Dioda łączenia WiFi
    static void LaczenieWiFi(bool statusWiFi)
    {
        if (statusWiFi == false)
        {

            if (millis() - Led::zapisCzas < 5)
                return;

            digitalWrite(LED_RED, 1);
            digitalWrite(LED_GREEN, 1);

            analogWrite(LED_BLUE, Led::itor);

            if (!Led::led_ostatni_stan)
                Led::itor += 5;

            if (Led::led_ostatni_stan)
                Led::itor -= 5;

            if (Led::itor >= 255 && !Led::led_ostatni_stan)
                Led::led_ostatni_stan = true;

            if (Led::itor <= 0 && Led::led_ostatni_stan)
                Led::led_ostatni_stan = false;

            Led::zapisCzas = millis();

            return;
        }
    }

    //Dioda statusu połączenia http
    static bool HttpStan(bool server_stan)
    {

        if (server_stan)
        {
            digitalWrite(LED_RED, 1);
            digitalWrite(LED_GREEN, 1);

            if (millis() - Led::zapisCzas < 1500)
            {
                digitalWrite(LED_BLUE, 0);
                return false;
            }

            digitalWrite(LED_BLUE, 1);
            return true;
        }

        if (millis() - Led::zapisCzas < 450)
            return false;

        digitalWrite(LED_BLUE, 1);
        digitalWrite(LED_GREEN, 1);
        digitalWrite(LED_RED, led_ostatni_stan);
        led_ostatni_stan = !led_ostatni_stan;
        Led::zapisCzas = millis();

        return false;
    }

    // Dioda awarii ToF
    static void ToF_nieznaleziono(bool status)
    {
        if (status)
        {
            digitalWrite(LED_RED, 1);
            digitalWrite(LED_GREEN, 1);
            digitalWrite(LED_BLUE, 1);

            return;
        }

        if (millis() - Led::zapisCzas < 150)
            return;

        digitalWrite(LED_GREEN, 1);
        digitalWrite(LED_BLUE, 1);

        digitalWrite(LED_RED, led_ostatni_stan);
        led_ostatni_stan = !led_ostatni_stan;
        Led::zapisCzas = millis();
    }
};

unsigned long Led::zapisCzas = millis();
bool Led::led_ostatni_stan = false;
uint8_t Led::itor = 0;