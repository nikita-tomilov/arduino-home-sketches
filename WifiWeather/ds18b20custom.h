void find_ds_sensors() {
    byte addr[8];
    while (ds.search(addr)) {
        Serial.print("\n\r\n\rFound \'1-Wire\' device with address:\n\r");
        for( int i = 0; i < 8; i++) {
            Serial.print("0x");
            if (addr[i] < 16) {
                Serial.print('0');
            }
            Serial.print(addr[i], HEX);
            if (i < 7) {
                Serial.print(", ");
            }
        }
    }
        
    Serial.println("No more addresses.");
}

void setup_ds_temp(byte* addr) {     
    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("DS CRC is not valid!");
        return;
    }
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);        // start conversion, without parasite power on at the end
    
    //delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.
    ds.reset();
    //ds.reset_search();
    //Serial.println("Setup DS18B20 done.");
}

float get_ds_temp(byte* addr) {
    setup_ds_temp(addr);
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    float celsius, fahrenheit;
    
    present = ds.reset();
    ds.select(addr);    
    ds.write(0xBE);         // Read Scratchpad
    
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds.read();
    }
    
    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10) {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - data[6];
        }
    } else {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    fahrenheit = celsius * 1.8 + 32.0;
    return celsius;
}