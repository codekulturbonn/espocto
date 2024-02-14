# ESP-Octo

Port of [Octo](https://github.com/JohnEarnest/c-octo.git) to [ESP32-2432S024C](https://www.aliexpress.com/item/1005005865107357.html) with capacitive touch.

## Checkout

Use "git clone --recursive" to get the required submodules.

## Touch

The touch interface is a CST820, connected with I2C. This is not supported by LovyanGFX. The board file specifies a CST816S, connected with SPI. I couldn't get this to work. Instead, I use files CST820.{cpp,hpp} from https://github.com/NoosaHydro/2.4inch_ESP32-2432S024.git

## Games

There are ~100 games from the CHIP-8 archive in "vendor/chip8Archive/roms". Put them in a SDcard in directory "/chip8". Also, put file "chip8.txt" in the root directory of this SDcard. This file is created from "chip8Archive/programs.json" with a small Python script.
