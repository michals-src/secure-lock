// Wlasna dodatkowa funkcja dla sensora Time of Flight
#pragma once

class ToF{
    private:
        static uint8_t eeprom_dlugosc;

public:

    uint8_t eeprom_dlugosc = 4;

    static void wartosc(){

    }

    /**
     * Zapis wartosci do pamieci eeprom
     * return void
     */
    static void zapisz( string wartosc){
        	uint16_t dlugosc = 4;

        EEPROM.begin(dlugosc);

        for (uint8_t i = 0; i <= dlugosc; i++) {
        EEPROM.write(i, 0);
        }
        EEPROM.commit();
        delay(500);

        // Zapisanie odleglosci w pamieci eeprom
        // od 4 bitu do 0
        for(uint8_t j = 4; j >= 0; j--){
            if( 4 - j > wartosc.length() ) break;
            uint8_t diff = 4 - j;
            EEPROM.write(j, wartosc[(wartosc.length() - 1) - diff ]);
        }

        EEPROM.commit();
        EEPROM.end();
    }

    /**
     * Odczyt zapisanej wartosci z pamieci eeprom
     * return uint8_t
     */
    static uint8_t odczytaj(){
        uint16_t dlugosc = 4;
        String wartosc = "";

        EEPROM.begin(dlugosc);

        for(uint8_t i = 0; i <= dlugosc; i++){
            wartosc += char(EEPROM.read(i));
        }

        return wartosc.toInt();
    }

}