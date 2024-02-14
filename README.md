# ESP-Octo

Port of [Octo](https://github.com/JohnEarnest/c-octo.git) to ESP32-2432S024C

## Touch

The touch interface is a CST820, connected with I2C. This is not supported by LovyanGFX. The board file specifies a CST816S, connected with SPI. I couldn't get this to work. Instead, I use files CST820.{cpp,hpp} from https://github.com/NoosaHydro/2.4inch_ESP32-2432S024.git
