// Wlasna dodatkowa funkcja dla sensora Time of Flight
#pragma once

class Tof
{
public:
    static uint8_t eeprom_dlugosc;

    /**
     * Zapis wartosci do pamieci eeprom
     * return void
     */
    static void zapisz(uint16_t wartosc)
    {
        uint16_t dlugosc = 3;

        byte byte1 = wartosc >> 8;
        byte byte2 = wartosc & 0xFF;

        EEPROM.begin(dlugosc);

        for (uint8_t i = 0; i <= dlugosc; i++)
        {
            EEPROM.write(i, 0);
        }
        EEPROM.commit();
        delay(500);

        EEPROM.write(0, byte1);
        EEPROM.write(1, byte2);

        EEPROM.commit();
        EEPROM.end();
    }

    /**
     * Odczyt zapisanej wartosci z pamieci eeprom
     * return uint8_t
     */
    static uint16_t odczytaj()
    {
        uint8_t dlugosc = 16;
        EEPROM.begin(dlugosc);

        byte rbyte1 = EEPROM.read(0);
        byte rbyte2 = EEPROM.read(0 + 1);
        uint16_t wartosc = (rbyte1 << 8) + rbyte2;

        return wartosc;
    }
};

uint8_t Tof::eeprom_dlugosc = 4;