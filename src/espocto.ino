/**
 * Play CHIP-8 ROMs on an esp32-2432s024c
 */ 
#include <string.h>

#include <SPI.h>
#include <SD.h>

#define SD_CS 5 


struct prgInfo_t {
  char* name;
  int tickrate;
  char* fillColor;
  char* backgroundColor;
  char* buzzColor;
  char* quietColor;
  bool shiftQuirks;
  bool loadStoreQuirks;
};

prgInfo_t* prg;
int prgCount = 0;
int prgSpace = 0;
int currPrg = 55; // octojam1title.ch8

char* ch8 = NULL;
int ch8Size;

bool isMonitor = false;
uint16_t monitorAddr;
uint8_t monitorNibble;

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <lgfx/v1/LGFX_Button.hpp>
#include "CST820.h"

class LGFX : public lgfx::LGFX_Device
{
lgfx::Panel_ILI9341     _panel_instance;
lgfx::Bus_SPI       _bus_instance;   // SPI bus instance
lgfx::Light_PWM     _light_instance;
//lgfx::Touch_CST816S     _touch_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();    // Gets the structure for bus configuration. 
      cfg.spi_host = HSPI_HOST;     // Select the SPI to use 
      cfg.spi_mode = 0;             // Set SPI communication mode (0 ~ 3) 
      cfg.freq_write = 55000000;    // SPI clock at the time of transmission (up to 80MHz, rounded to the value obtained by dividing 80MHz by an integer) // 送信時のSPIクロック (最大80MHz, 80MHzを整数で割った値に丸められます)
      cfg.freq_read  = 20000000;    // SPI clock when receiving 
      cfg.spi_3wire  = false;       // Set true when receiving with MOSI pin 
      cfg.use_lock   = true;        // Set to true when using transaction lock
      cfg.dma_channel = 1;          // Set the DMA channel (1 or 2. 0=disable)
      cfg.pin_sclk = 14;            // Set SPI SCLK pin number 
      cfg.pin_mosi = 13;            // Set SPI MOSI pin number
      cfg.pin_miso = 12;            // Set SPI MISO pin number (-1 = disable) 
      cfg.pin_dc   = 2;             // Set SPI D / C pin number (-1 = disable) 

      _bus_instance.config(cfg);    // The set value is reflected on the bus.
      _panel_instance.setBus(&_bus_instance);      // Set the bus on the panel.
    }

    { // Set the display panel control.
      auto cfg = _panel_instance.config();    // Gets the structure for display panel settings.
      cfg.pin_cs           =    15;  // Pin number to which CS is connected (-1 = disable) 
      cfg.pin_rst          =    -1;  // Pin number to which RST is connected (-1 = disable) 
      cfg.pin_busy         =    -1;  // Pin number to which BUSY is connected (-1 = disable) 
      cfg.memory_width     =   240;  // Maximum width supported by driver IC 
      cfg.memory_height    =   320;  // Maximum height supported by driver IC 
      cfg.panel_width      =   240;  // Actually displayable width 
      cfg.panel_height     =   320;  // Actually displayable height 
      cfg.offset_x         =     0;  // Amount of X-direction offset of the panel
      cfg.offset_y         =     0;  // Amount of Y-direction offset of the panel 
      cfg.offset_rotation  =     0;  // Offset of values in the direction of rotation 0 ~ 7 (4 ~ 7 are upside down) 
      cfg.dummy_read_pixel =     8;  // Number of dummy read bits before pixel reading 
      cfg.dummy_read_bits  =     1;  // Number of bits of dummy read before reading data other than pixels 
      cfg.readable         =  true;  // Set to true if data can be read 
      cfg.invert           = false;  // Set to true if the light and darkness of the panel is reversed 
      cfg.rgb_order        = false;  // Set to true if the red and blue of the panel are swapped 
      cfg.dlen_16bit       = false;  // Set to true for panels that send data length in 16-bit units 
      cfg.bus_shared       =  true;  // If the bus is shared with the SD card, set to true (bus control is performed with drawJpgFile etc.) 

      _panel_instance.config(cfg);
    }
    
    { // Set the backlight control. (Delete if not needed
      auto cfg = _light_instance.config();    // Gets the structure for the backlight setting. 

      cfg.pin_bl = 27;              // Pin number to which the backlight is connected 
      cfg.invert = false;           // True if you want to invert the brightness of the backlight 
      cfg.freq   = 44100;           // Backlight PWM frequency 
      cfg.pwm_channel = 7;          // PWM channel number to use 

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  // Set the backlight on the panel. 
    }

  #if 0
    { // Set the touch screen control. (Delete if not needed)
      auto cfg = _touch_instance.config();
      cfg.x_min      = 0;    // Minimum X value (raw value) obtained from touch screen 
      cfg.x_max      = 239;  // Maximum X value (raw value) obtained from the touch screen 
      cfg.y_min      = 0;    // Minimum Y value (raw value) obtained from touch screen
      cfg.y_max      = 319;  // Maximum Y value (raw value) obtained from the touch screen 
      cfg.pin_int    = 36;   // Pin number to which INT is connected 
      cfg.bus_shared = true; // Set to true if you are using the same bus as the screen 
      cfg.offset_rotation = 0;// Adjustment when the display and touch orientation do not match Set with a value from 0 to 7 
      cfg.spi_host = VSPI_HOST;// Select the SPI to use (HSPI_HOST or VSPI_HOST) 
      cfg.freq = 2500000;     // Set SPI clock 
      cfg.pin_sclk = 25;     // Pin number to which SCLK is connected 
      cfg.pin_mosi = 32;     // Pin number to which MOSI is connected 
      cfg.pin_miso = 39;     // Pin number to which MISO is connected 
      cfg.pin_cs   = 33;     // Pin number to which CS is connected 
      
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  // Set the touch screen on the panel. 
    }
  #endif
    setPanel(&_panel_instance); // Set the panel to be used. 
  }
};

#define I2C_SDA 33
#define I2C_SCL 32
#define TP_RST 25
#define TP_INT 21
CST820 touch(I2C_SDA, I2C_SCL, TP_RST, TP_INT);

#include "console.h"
#include "octo_emulator.h"

const int WIDTH = 320;
const int HEIGHT = 240;

#if defined ( SDL_h_ )
static LGFX lcd(WIDTH, HEIGHT, 2);
#else
static LGFX lcd;
static LGFX_Sprite sprite(&lcd);
#endif

static char lbl[20][2] = { 
  "1", "2", "3", "C", "<",
  "4", "5", "6", "D", ">",
  "7", "8", "9", "E", "G",
  "A", "0", "B", "F", "M",
};

static LGFX_Button btn[20];

static LGFX_Button btnFlag;

#ifdef TARGET_NATIVE
#include <chrono>
// Implement millis() for non-Arduino environments
unsigned long millis() {
  static auto start = std::chrono::steady_clock::now();
  auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}
#endif

octo_emulator emu;
octo_options options;

const std::int8_t KEY_NONE = -1;

const std::int8_t KEY_LEFT = -2;
const std::int8_t KEY_RIGHT = -3;
const std::int8_t KEY_GO = -4;
const std::int8_t KEY_MONITOR = -5;

std::int8_t hexButton(std::uint8_t i) {
  char c = lbl[i][0];

  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  else
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 0xA;
  }
  else
  if (c == '<') return KEY_LEFT;
  else
  if (c == '>') return KEY_RIGHT;
  else
  if (c == 'G') return KEY_GO;
  else
  if (c == 'M') return KEY_MONITOR;
  else return KEY_NONE;
}

void drawButtons(void) {
  for (int col = 0; col <= 4; col++) {
    for (int row = 0; row <= 3; row++) {
      int n = 5 * row + col;

      btn[n].initButton(&lcd,
        36 + col * 42,      // x
        150 + row * 42,     // y
        36,                 // w
        36,                 // h
        0xFFFFCC00u,        // outline
        0xFF996600u,        // fill
        col == 4 ? 0xFFFF6600u : 0xFFFFCC00u,
                            // textcolor
        lbl[n],             // label
        1.0,                // textsize x
        1.0                 // textsize y
      );
      btn[n].drawButton();
    }
  }
}

void showCurrPrg() {
  lcd.fillRect(0, 0, 240, 18, 0xFFFFCC00u);

  lcd.setTextColor(0xFF996600u, 0xFFFFCC00u);
  lcd.drawNumber(options.tickrate, 2, 0, &fonts::FreeMonoBold9pt7b);
  lcd.drawCenterString(prg[currPrg].name, 120, 0, &fonts::FreeMonoBold9pt7b);
  lcd.setTextColor(0xFFFFCC00u, 0xFF996600u);
}

void loadCurrPrg() {
  char* path = (char*) malloc(12 + strlen(prg[currPrg].name));
  strcpy(path, "/chip8/");
  strcat(path, prg[currPrg].name);
  strcat(path, ".ch8");
  File f = SD.open(path);
  ch8Size = f.size();
  if (ch8) {
    ch8 = (char*)realloc(ch8, ch8Size);
  }
  else {
    ch8 = (char*)malloc(ch8Size);
  }
  f.read((uint8_t*)ch8, ch8Size);
  f.close();
  free(path);

  octo_default_options(&options);
  options.tickrate = prg[currPrg].tickrate;
  options.q_shift = prg[currPrg].shiftQuirks;
  options.q_loadstore = prg[currPrg].loadStoreQuirks;
  octo_emulator_init(&emu, ch8, ch8Size, &options, NULL);

  monitorAddr = 0x200;
  monitorNibble = 0;

  showCurrPrg();
}

void loadPrgInfo(void) {
  File f = SD.open("/chip8.txt");
  if (!f) {
    console_printf("Can't read info\r\n");
    return;
  }
  int size = f.size();
  char* info = (char*)malloc(size + 1);
  f.read((uint8_t*)info, size);
  f.close();
  info[size] = '\0';

  prg = (prgInfo_t*)malloc(30 * sizeof(prgInfo_t));
  prgSpace = 30;

  while (true) {
    prgInfo_t prgInfo;

    prgInfo.name = strtok(info, ",");
    info = NULL;
    if (!prgInfo.name) {
      break;
    }
    prgInfo.tickrate = atoi(strtok(NULL, ","));
    prgInfo.fillColor = strtok(NULL, ",");
    prgInfo.backgroundColor = strtok(NULL, ",");
    prgInfo.buzzColor = strtok(NULL, ",");
    prgInfo.quietColor = strtok(NULL, ",");
    char* s = strtok(NULL, ",");
    prgInfo.shiftQuirks = s[0] == 1;
    s = strtok(NULL, "\n");
    prgInfo.loadStoreQuirks = s[0] == 1;

    if (prgSpace < ++prgCount) {
      prgSpace += 30;
      //console_printf("cnt=%d. More space -> %d\r\n", prgCount, prgSpace);
      prg = (prgInfo_t*)realloc(prg, prgSpace * sizeof(prgInfo_t));
    }
    //console_printf("%d: %s\r\n", prgCount -1, prgInfo.name);
    memcpy(&prg[prgCount - 1], &prgInfo, sizeof(prgInfo_t));
  }
  console_printf("%d files read.\r\n", prgCount);
}

void setup(void)
{
  Serial.begin(115200);

  if (!SD.begin(SD_CS)) {
    console_printf("SD.begin failed!\r\n");
    while (1) delay(0);
  }

  loadPrgInfo();

  lcd.init();
  lcd.setRotation(2);

  touch.begin();

//  lcd.setBrightness(128);
  lcd.setColorDepth(16);
  lcd.fillScreen(0xFF996600u);
  lcd.setFont(&fonts::FreeMonoBold12pt7b);

  loadCurrPrg();
  drawButtons();
}

void ui_run(octo_emulator* emu) {
  // drop repaints if the display hasn't changed
  int dirty = memcmp(emu->px, emu->ppx, sizeof(emu->px)) != 0;

  if (!dirty) return;
  memcpy(emu->ppx,emu->px,sizeof(emu->ppx));

  // render chip8 display
  int w = emu->hires ? 128 : 64, h = emu->hires ? 64 : 32;
  float scale = emu->hires ? 1.5 : 3;

  static char lastRes = emu->hires;
  if (emu->hires != lastRes) {
    lastRes = emu->hires;
    lcd.fillCircle(10, 32, 4, emu->hires ? 0xFFFF6600u : 0xFF996600u);
    console_printf("%sres rot=%d w=%d h=%d scale=%f\r\n", emu->hires ? "hi" : "lo", emu->options.rotation, w, h, scale);
  }

  sprite.createSprite(w, h);
  sprite.setPivot(w / 2, 0);
  sprite.setColorDepth(4);
  sprite.setPaletteColor(1, 0xFFFFCC00u);
  sprite.setPaletteColor(2, 0xFFFF6600u);
  sprite.setPaletteColor(3, 0xFF662200u);

  for(int y=0; y<h; y++) {
    for(int x=0; x<w; x++) {
      int c = emu->px[x + (y*w)];
      //console_printf("%d", c);
      sprite.drawPixel(x, y, c);
    }
    //console_printf("\n");
  }
  sprite.pushRotateZoom(lcd.width() / 2, 74, 0, scale, scale);
  sprite.deleteSprite();
}

void emu_step(octo_emulator* emu) {
  static bool flagged = false;
  if (emu->halt) {
    if (!flagged) {
        flagged = true;
        console_printf("halted");
    }
    return;
  }
  for (int z=0; z<emu->options.tickrate && !emu->halt; z++) {
    if (emu->options.q_vblank && (emu->ram[emu->pc]&0xF0) == 0xD0) {
        z=emu->options.tickrate;
    }
    //console_printf("pc=%0x", emu->pc);
    octo_emulator_instruction(emu);
  }
  if (emu->dt>0) emu->dt--;
  if (emu->st>0) emu->st--, emu->had_sound=1;
}

char* instr(octo_emulator* emu, uint16_t addr) {
  uint8_t hi = emu->ram[addr], lo = emu->ram[addr+1], op = hi >> 4;  
  uint16_t wd = hi;
  wd <<= 8; wd |= lo;
  static char buf[13]; 
  switch (op) {
    case 0x0:
      switch (wd) {
        case 0x00E0:
          return "cls";
        case 0x00EE:
          return "ret";
        case 0x00FF:
          return "hires";
        case 0x00FE:
          return "lores";
        case 0x00FD:
          return "exit";
        case 0x00FB:
          return "scr";
        case 0x00FC:
          return "scl";
        default:
          if (lo & 0xF0 == 0xC0) {
            snprintf(buf, 8, "scd d %X", lo & 0x0F);
          }
          else {
            snprintf(buf, 5, "%04X", wd);
          }
          return buf;
      }
      break;
    case 0x1:
      snprintf(buf, 7, "jp %03X", wd & 0xFFF);
      return buf;
    case 0x2:
      snprintf(buf, 9, "call %03X", wd & 0xFFF);
      return buf;
    case 0x3:
      snprintf(buf, 9, "se v%X,%02X", hi & 0xF, lo);
      return buf;
    case 0x4:
      snprintf(buf, 10, "sne v%X,%02X", hi & 0xF, lo);
      return buf;
    case 0x5:
      snprintf(buf, 9, "se v%X,v%X", hi & 0xF, lo & 0xF0 >> 4);
      return buf;
    case 0x6:
      snprintf(buf, 9, "ld v%X,%02X", hi & 0xF, lo);
      return buf;
    case 0x7:
      snprintf(buf, 10, "add v%X,%02X", hi & 0xF, lo);
      return buf;
    case 0x8:
      switch (lo & 0xF) {
        case 0x0:
          snprintf(buf, 9, "ld v%X,v%X", hi & 0xF, lo & 0xF0 >> 4);
          return buf;
        case 0x1:
          snprintf(buf, 9, "or v%X,v%X", hi & 0xF, lo & 0xF0 >> 4);
          return buf;
        case 0x2:
          snprintf(buf, 10, "and v%X,v%X", hi & 0xF, lo & 0xF0 >> 4);
          return buf;
        case 0x3:
          snprintf(buf, 10, "xor v%X,v%X", hi & 0xF, lo & 0xF0 >> 4);
          return buf;
        case 0x4:
          snprintf(buf, 10, "add v%X,v%X", hi & 0xF, lo & 0xF0 >> 4);
          return buf;
        case 0x5:
          snprintf(buf, 10, "sub v%X,v%X", hi & 0xF, lo & 0xF0 >> 4);
          return buf;
        case 0x6:
          snprintf(buf, 10, "shr v%X,v%X", hi & 0xF, lo & 0xF0 >> 4);
          return buf;
        case 0x7:
          snprintf(buf, 11, "subn v%X,v%X", hi & 0xF, lo & 0xF0 >> 4);
          return buf;
        case 0xE:
          snprintf(buf, 10, "shl v%X,v%X", hi & 0xF, lo & 0xF0 >> 4);
          return buf;
        default: 
          snprintf(buf, 5, "%04X", wd);
          return buf;
      }
    case 0x9:
      snprintf(buf, 10, "sne v%X,v%X", hi & 0xF, lo & 0xF0 >> 4);
      return buf;
    case 0xa:
      snprintf(buf, 9, "ld i %03X", wd & 0xFFF);
      return buf;
    case 0xb:
      snprintf(buf, 9, "jp v0,%03X", wd & 0xFFF);
      return buf;
    case 0xc:
      snprintf(buf, 9, "rnd v%X,%02X", hi & 0xF, lo);
      return buf;
    case 0xd:
      snprintf(buf, 12, "drw v%X,v%X,%X", hi & 0xF, lo >> 4, lo & 0xF);
      return buf;
    case 0xe:
      switch (lo) {
        case 0x9E:
          snprintf(buf, 7, "skp v%X", hi & 0xF);
          return buf;
        case 0xA1:
          snprintf(buf, 7, "sknp v%X", hi & 0xF);
          return buf;
        default: 
          snprintf(buf, 5, "%04X", wd);
          return buf;
      }
    case 0xf:
      switch (lo) {
        case 0x07:
          snprintf(buf, 9, "ld v%X,dt", hi & 0xF);
          return buf;
        case 0x0A:
          snprintf(buf, 8, "ld v%X,k", hi & 0xF);
          return buf;
        case 0x15:
          snprintf(buf, 9, "ld dt,v%X", hi & 0xF);
          return buf;
        case 0x18:
          snprintf(buf, 9, "ld st,v%X", hi & 0xF);
          return buf;
        case 0x1E:
          snprintf(buf, 9, "add i,v%X", hi & 0xF);
          return buf;
        case 0x29:
          snprintf(buf, 8, "ld f,v%X", hi & 0xF);
          return buf;
        case 0x33:
          snprintf(buf, 8, "ld b,v%X", hi & 0xF);
          return buf;
        case 0x55:
          snprintf(buf, 10, "ld [i],v%X", hi & 0xF);
          return buf;
        case 0x65:
          snprintf(buf, 10, "ld v%X,[i]", hi & 0xF);
          return buf;
        default: 
          snprintf(buf, 5, "%04X", wd);
          return buf;
      }
    default: 
      snprintf(buf, 5, "%04X", wd);
      return buf;
  }
}

void showMonitor(octo_emulator* emu) {
  console_printf("Monitor\r\n");

  lcd.fillRect(0, 20, 240, 108, 0xFF996600u);

  uint16_t addr = monitorAddr - 2;

  char buf[25];
  for (int i = 0; i < 5; i++) {
    snprintf(buf, 24, "%04X:       %s", addr, instr(emu, addr));
    lcd.drawString(buf, 20, 24 + i*20, &fonts::AsciiFont8x16);

    snprintf(buf, 5, "%02X%02X", emu->ram[addr], emu->ram[addr+1]);
    char c[2];
    c[1] = '\0';
    for (int n = 0; n < 4; n++) {
      if (i == 1 && n == monitorNibble) {
        lcd.setTextColor(0xFF996600u, 0xFFFFCC00u);
      }
      else {
        lcd.setTextColor(0xFFFFCC00u, 0xFF996600u);
      }
      c[0] = buf[n];
      lcd.drawString(c, 20 + 48 + n*8, 24 + i*20, &fonts::AsciiFont8x16);
      lcd.setTextColor(0xFFFFCC00u, 0xFF996600u);
    }
    addr += 2;
  }
}

unsigned long previousMillis = 0; // will store last time the function was called
const long interval = 22;// 33; // interval at which to call function (milliseconds)

void loop(void)
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time the function was called
    previousMillis = currentMillis;

    bool touched;
    uint8_t gesture;
    uint16_t touchX, touchY;

    touched = touch.getTouch(&touchX, &touchY, &gesture);

    if (touched) {
      for (int i = 0; i < 20; i++) {
        if (btn[i].contains(touchX, touchY)) {
          btn[i].press(true);
          btn[i].drawButton(true);

          //lcd.fillRect(218, 0, 22, 14, 0xFF996600u);
          //lcd.setTextColor(0xFFFFCC00u);
          //lcd.drawString(lbl[i], 218, 0, &fonts::FreeMono9pt7b);

          //lcd.fillRect(230, 0, 10, 18, 0xFFFFCC00u);
          lcd.setTextColor(0xFF996600u, 0xFFFFCC00u);
          lcd.drawString(lbl[i], 230, 0, &fonts::FreeMonoBold9pt7b);
          lcd.setTextColor(0xFFFFCC00u, 0xFF996600u);

          std::int8_t b = hexButton(i);
          //console_printf("btn %d\r\n", b);

          if (btn[i].justPressed()) {
            if (isMonitor) {
              if (b == KEY_LEFT) {
                if (monitorAddr >= 0x202) {
                  monitorAddr -= 2;
                  monitorNibble = 0;
                  showMonitor(&emu);
                }
              }
              else
              if (b == KEY_RIGHT) {
                if (monitorAddr < 4 * 1024 - 2) {
                  monitorAddr += 2;
                  monitorNibble = 0;
                  showMonitor(&emu);
                }
              }
              else
              if (b == KEY_GO) {
              }
              else
              if (b == KEY_MONITOR) {
                isMonitor = false;
                lcd.fillRect(0, 15, 240, 102, 0xFF996600u);
              }
              else {
                uint8_t* m = &emu.ram[monitorAddr];
                switch (monitorNibble) {
                  case 0:
                    *m = (*m & 0xF) | (b << 4);
                    break;
                  case 1:
                    *m = (*m & 0xF0) | b;
                    break;
                  case 2:
                    *(m+1) = (*(m+1) & 0xF) | (b << 4);
                    break;
                  case 3:
                    *(m+1) = (*(m+1) & 0xF0) | b;
                    break;
                }
                monitorNibble += 1;
                if (monitorNibble == 4) {
                  monitorNibble = 0;
                  monitorAddr += 2;
                }
                showMonitor(&emu);
              }
            }
            else {
              // not isMonitor
              if (b == KEY_LEFT) {
                if (currPrg > 0) {
                  currPrg -= 1;
                  showCurrPrg();
                }
              }
              else
              if (b == KEY_RIGHT) {
                if (currPrg < prgCount - 1) {
                  currPrg += 1;
                  showCurrPrg();
                }
              }
              else
              if (b == KEY_GO) {
                loadCurrPrg();
              }
              else
              if (b == KEY_MONITOR) {
                isMonitor = true;
                showMonitor(&emu);
              }
            }
          }
          if (b >= 0) {
            emu.keys[b] = true;
          }
        }
      }
    }
    else {
      // not touched
      for (int i = 0; i < 20; i++) {
        if (btn[i].isPressed()) {
          btn[i].press(false);
          btn[i].drawButton(false);

          std::int8_t b = hexButton(i);
          if (b >= 0) {
            emu.keys[b] = false;
          }
          lcd.fillRect(230, 0, 10, 18, 0xFFFFCC00u);
        }
      }
    }

    if (!isMonitor) { 
      emu_step(&emu);
      ui_run(&emu);
    }
  }
}

#if defined ( ESP_PLATFORM ) && !defined ( ARDUINO )
extern "C" {
int app_main(int, char**)
{
    setup();
    for (;;) {
      loop();
    }
    return 0;
}
}
#endif