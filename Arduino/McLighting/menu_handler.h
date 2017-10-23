#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MD_KeySwitch.h>
#include <Encoder.h>

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);


Encoder myEnc(D5, D6);
long oldPosition  = -999;
boolean isButtonPressed = false;
long lastUpdateMillis = 0;

const uint8_t SWITCH_PIN = D7;       // switch connected to this pin
const uint8_t SWITCH_ACTIVE = LOW;  // digital signal when switch is pressed 'on'

MD_KeySwitch rotaryButton(SWITCH_PIN, SWITCH_ACTIVE);

#define MENU_LENGTH 9

uint8_t currentMenu = 0;
bool menuActivated = false;

String statusMessage = "";

void initMenuHandler() {

  rotaryButton.begin();
  rotaryButton.enableDoublePress(true);
  rotaryButton.enableLongPress(true);
  rotaryButton.enableRepeat(false);
  rotaryButton.enableRepeatResult(false);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE, BLACK);
  display.clearDisplay();
  display.fillRect(0, 0, 64, 48, WHITE);
  display.display();

}

void setStripSettings() {
  strip.setMode(ws2812fx_mode);
  strip.setColor(main_color.red, main_color.green, main_color.blue);
  strip.setSpeed(ws2812fx_speed);
  strip.setBrightness(brightness);
}


const String menuItems[] = {
  "Brightness",
  "Speed",
  "Mode",
  "Red",
  "Green",
  "Blue",
  "WiFi",
  "Load conf",
  "Save conf"
};

void drawSlider(uint8_t y, int value, int min, int max) {
  display.drawRect(0, y, 64, 6, WHITE);
  if (value > min) {
    display.drawRect(2, y + 2, (int) (60.0 * (float)value / (float)(max - min)), 2, WHITE);
  }
}

void updateDisplay() {
  display.clearDisplay();
  if (!menuActivated) {
    // show main menu
    display.setTextSize(1);

    uint8_t offset = (currentMenu / 6) * 6;
    for (uint8_t i = 0; i < 7; i++) {
      uint8_t menuIndex = offset + i;
      if (menuIndex > MENU_LENGTH - 1) {
        continue;
      }
      if (currentMenu == menuIndex) {
        display.fillRect(0, i * 8, display.width(), 8, WHITE);
        display.setCursor(1, i * 8);
        display.setTextColor(BLACK, WHITE);
      } else {
        display.setCursor(0, i * 8);
        display.setTextColor(WHITE, BLACK);
      }
      display.println(menuItems[menuIndex]);
    }
  } else {
    // show the screen of the selected menu item

    // menu header
    display.fillRect(0, 0, display.width(), 8, WHITE);
    display.setCursor(1, 0);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    display.println(menuItems[currentMenu]);

    if (currentMenu >= 0 && currentMenu <= MENU_LENGTH - 1) {
      display.setTextColor(WHITE, BLACK);
      display.setCursor(0, 9);
      display.setTextSize(2);
      if (currentMenu == 0) {
        display.println(brightness);
        drawSlider(41, brightness, 0, 255);
      } else  if (currentMenu == 1) {
        display.println(ws2812fx_speed);
        drawSlider(41, ws2812fx_speed, 0, 255);
      } else  if (currentMenu == 2) {
        display.println(ws2812fx_mode);
        drawSlider(41, ws2812fx_mode, 0, strip.getModeCount() - 1);
        display.setTextSize(1);
        display.setCursor(0, 25);
        display.println(strip.getModeName(ws2812fx_mode));
      } else  if (currentMenu == 3) {
        display.println(main_color.red);
        drawSlider(41, main_color.red, 0, 255);
      } else if (currentMenu == 4) {
        display.println(main_color.green);
        drawSlider(41, main_color.green, 0, 255);
      } else if (currentMenu == 5) {
        display.println(main_color.blue);
        drawSlider(41, main_color.blue, 0, 255);
      } else if (currentMenu == 6) {
        if (wifiEnabled) {
          display.println("ON");
        } else {
          display.println("OFF");
        }

        if (wifiStarted) {
          display.setTextSize(1);
          display.setCursor(0, 25);
          display.println("IP address");
          display.println(WiFi.localIP());
        } else if (wifiConfigMode) {
          display.print(" AP");
          display.setTextSize(1);
          display.setCursor(0, 25);
          display.print("Connect to ");
          display.print(HOSTNAME);
          display.print(" IP: ");
          display.print(WiFi.softAPIP());
        }

      } else if (currentMenu == 7) {
        display.setTextSize(1);
        display.println(statusMessage);
      } else if (currentMenu == 8) {
        display.setTextSize(1);
        display.println(statusMessage);
      }
    }
  }

  display.display();
}


void handleRotation(int delta) {
  if (!menuActivated) {

    currentMenu = constrain(currentMenu + delta, 0, MENU_LENGTH - 1);

  } else if (currentMenu == 0) {

    brightness = constrain(brightness + 10 * delta, 0, 255);
    strip.setBrightness(brightness);

  } else if (currentMenu == 1) {

    ws2812fx_speed = constrain(ws2812fx_speed + 10 * delta, 0, 255);
    strip.setSpeed(ws2812fx_speed);

  } else if (currentMenu == 2) {

    ws2812fx_mode = constrain(ws2812fx_mode + delta, 0, strip.getModeCount() - 1);
    strip.setColor(main_color.red, main_color.green, main_color.blue);
    strip.setMode(ws2812fx_mode);

  } else if (currentMenu == 3) {

    main_color.red = constrain(main_color.red + 10 * delta, 0, 255);
    strip.setColor(main_color.red, main_color.green, main_color.blue);

  } else if (currentMenu == 4) {

    main_color.green = constrain(main_color.green + 10 * delta, 0, 255);
    strip.setColor(main_color.red, main_color.green, main_color.blue);

  } else if (currentMenu == 5) {

    main_color.blue = constrain(main_color.blue + 10 * delta, 0, 255);
    strip.setColor(main_color.red, main_color.green, main_color.blue);

  } else if (currentMenu == 6) {

    if (delta > 0 && !wifiEnabled) {
      wifiEnabled = true;
      saveWifiSettings();
      initWIFI();
    }
    if (delta < 0 && wifiEnabled) {
      wifiEnabled = false;
      saveWifiSettings();
      stopWIFI();
    }

  }
}

void handleSinglePress() {
  if (!menuActivated) {
    if (currentMenu == 7) {
      if (loadConfig()) {
        setStripSettings();
        statusMessage = "Loaded";
      } else {
        statusMessage = "Error";
      }
    } else if (currentMenu == 8) {
      if (saveConfig()) {
        statusMessage = "Saved";
      } else {
        statusMessage = "Error";
      }
    }
  }
  menuActivated = !menuActivated;
}

long lastPressTime = millis();
bool screenIsOff = false;

void screenOff() {
  display.clearDisplay();
  display.display();
  screenIsOff = true;
}
void keepScreenOn() {
  lastPressTime = millis();
  screenIsOff = false;
}

void handleMenu() {

  // display auto off after 30 secs
  if (!screenIsOff && millis() - lastPressTime > 30000) {
    screenOff();
  }

  // handle rotation

  int newPosition = myEnc.read() / 4; // read value increases/decreases by 4 for one tick with this encoder
  bool needUpdate = false;
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    myEnc.write(0);
    if (oldPosition == -1 || oldPosition == 1) {
      if (!screenIsOff) {
        handleRotation(oldPosition);
      }
      keepScreenOn();

      needUpdate = true;
    }
  }

  // handle button

  switch (rotaryButton.read()) {
    case MD_KeySwitch::KS_NULL:
      /* DBG_OUTPUT_PORT.println("NULL"); */
      break;
    case MD_KeySwitch::KS_PRESS:
      //DBG_OUTPUT_PORT.print("\nSINGLE PRESS");
      if (!screenIsOff) {
        handleSinglePress();
      }
      keepScreenOn();
      needUpdate = true;
      break;
    case MD_KeySwitch::KS_DPRESS:
      //DBG_OUTPUT_PORT.print("\nDOUBLE PRESS");
      break;
    case MD_KeySwitch::KS_LONGPRESS:
      //DBG_OUTPUT_PORT.print("\nLONG PRESS");
      if (blackOut) {
        //loadConfig();
        setStripSettings();
        blackOut = false;
        needUpdate = true;
        keepScreenOn();
      } else {
        strip.setMode(0);
        strip.setColor(0, 0, 0);
        strip.setBrightness(0);
        display.clearDisplay();
        display.display();
        blackOut = true;
      }
      break;
    default:
      DBG_OUTPUT_PORT.print("\nUNKNOWN");
      break;
  }
  if (needUpdate) {
    updateDisplay();
  }
}



