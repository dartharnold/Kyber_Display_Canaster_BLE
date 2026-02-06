#include <Arduino.h>
#include <bluefruit.h>
#include <Adafruit_NeoPixel.h>

// NeoPixel declarations
// Which pin on the Arduino is connected to the NeoPixels?
Adafruit_NeoPixel strip[2];
uint8_t pins[2] = {7, 9};
#define PIXELCOUNT 3  // How many NeoPixels are attached to the Arduino?
#define NEOBRIGHT 75
#define PIXELFORMAT NEO_GRB + NEO_KHZ800  // NeoPixel color format & data rate. See the strandtest example for information on possible values.

// Device Info
const char* DEVNAME = "KyberVault";

// Counters and Timers
#define CHANGEDELY 15*1000

// Location IDs (in Int)
#define NOBEACON      0
#define MARKETPLACE   1
#define BEHINDDEPOT   2
#define RESISTANCE    3
#define UNKNOWN       4
#define DROIDDEPOT    5
#define DOKONDARS     6
#define FIRSTORDER    7

// Crystal Colors
#define BLACK         0x00000000
#define WHITE         0x00FFFFFF
#define RED           0x00FF0000
#define CRIMSONRED    0x00DC143C
#define PURPLE        0x00FF00FF
#define DARKPURPLE    0x00301934
#define BLUE          0x000000FF
#define CYAN          0x0000FFFF
#define GREEN         0x0000FF00
#define YELLOW        0x00FFBF00
#define ORANGE        0x00FF7E00
//#define ORANGE        0x00FFA500

// Filters
#define RSSI -75
#define BLE_DISNEY 0x0183
const String IGNOREHOST = "SITH-TLBX";

// Timing
uint32_t last_activity;

// Scan Values to store
int8_t scan_rssi;
uint16_t area_num;
String beacon_name = "";

// Function Declaration
void scan_callback(ble_gap_evt_adv_report_t* report);

void setup() {
  Serial.begin(115200);
  // while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("SWGE_Beacon_Scan");
  Serial.println("----------------\n");
  last_activity = CHANGEDELY;

  // Create a new NeoPixel object dynamically with these values:
  for (int x = 0; x <= 1; x++) {
    strip[x].updateLength(PIXELCOUNT);
    strip[x].updateType(PIXELFORMAT);
    strip[x].setPin(pins[x]);
  }
  
  // Initialize the NeoPixels and turn them all off (black) at startup:
  for (int x = 0; x <= 1 ; x++) {
    strip[x].begin();
    strip[x].show();
    for (int p = 0; p <= 2; p++) {
      strip[x].setPixelColor(p, BLACK);
    }
    strip[x].setBrightness(25);
    strip[x].show();
  }
  randomSeed(analogRead(0));

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(0, 1);
  Bluefruit.setTxPower(4);  // Check bluefruit.h for supported values

  /* Set the device name */
  Bluefruit.setName(DEVNAME);

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Filter out packet with a min rssi
   * - Interval = 100 ms, window = 50 ms
   * - Use active scan (used to retrieve the optional scan response adv packet)
   * - Start(0) = will scan forever since no timeout is given
   */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.filterRssi(RSSI);
  Bluefruit.Scanner.filterMSD(BLE_DISNEY);
  Bluefruit.Scanner.setInterval(160, 80);  // in units of 0.625 ms
  Bluefruit.Scanner.useActiveScan(true);   // Request scan response data
  Bluefruit.Scanner.start(0);              // 0 = Don't stop scanning after n seconds

  Serial.println("Starting SWGE Location Scanning ...");
}

void scan_callback(ble_gap_evt_adv_report_t* report) {
  PRINT_LOCATION();
  //uint8_t len = 0;
  uint8_t beacon_name_buff[32];
  uint8_t md_len = 0;
  uint8_t md_buffer[32];

  memset(beacon_name_buff, 0, sizeof(beacon_name_buff));
  memset(md_buffer, 0, sizeof(md_buffer));

  //uint8_t mfg_size;
  //uint8_t mfg_data[32];

  /* Check for Manufacturer Specific Data is it a Location?*/
  md_len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, md_buffer, sizeof(md_buffer));
  if ((md_len) && (md_buffer[2] == 0x0A))  // Process beacon only if it is a Location
  {
    /* Complete Local Name */
    Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, beacon_name_buff, sizeof(beacon_name_buff));

    // Convert beacon_name_buff to string beacon_name
    beacon_name = "";  // clear name to rebuild
    for (byte i = 0; i < sizeof(beacon_name); i++) {
      if (beacon_name_buff[i] > 0) {
        beacon_name.concat(char(beacon_name_buff[i]));
      }
    }

    if ((beacon_name != IGNOREHOST) && ((millis() - last_activity) >= CHANGEDELY))  // Proces beacon data only if it is from a host that is not being ignored and the delay has been met.
    {
      // Reset Delay Timer
      last_activity = millis();

      // What is the location ID?
      area_num = md_buffer[4];

      /* RSSI value */
      scan_rssi = report->rssi;

      // Serial.println();
      // Serial.print("Beacon Host: ");
      // Serial.println(beacon_name);
      // Serial.print("RSSI: ");
      // Serial.print(scan_rssi);
      // Serial.println(" dBm");
      // Serial.print("Area #: ");
      // Serial.println(area_num);
      // Serial.println("----------------\n");
    }
  }

  // For Softdevice v6: after received a report, scanner will be paused
  // We need to call Scanner resume() to continue scanning
  Bluefruit.Scanner.resume();
}

// void updatePixels() {
//   uint32_t DelayOff = random(200, 500);
//   uint32_t DelayOn = random(200, 800);
//   uint8_t pixel;

//   for (int x = 0; x <= 1; x++) {
//     strip[x]->clear();
//     strip[x]->show();
//   }

//   delay(DelayOff);  // Pause Pixel Off for this pass through loop

//   // Setup to turn Pixels On
//   //Serial.println(area_num);
//   switch (area_num) {
//     case MARKETPLACE:
//       //Serial.println("Marketplace, Change to Green");
//       for (pixel = 0; pixel <= PIXELCOUNT; pixel++) {
//         odd_pixels->setPixelColor(pixel, GREEN);
//       }
//       break;
//     case BEHINDDEPOT:
//       //Serial.println("Behind Depot, Change to Yellow");
//       for (pixel = 0; pixel <= PIXELCOUNT; pixel++) {
//         odd_pixels->setPixelColor(pixel, YELLOW);
//       }
//       break;
//     case RESISTANCE:
//       //Serial.println("Resistance, Change to Blue");
//       for (pixel = 0; pixel <= PIXELCOUNT; pixel++) {
//         odd_pixels->setPixelColor(pixel, BLUE);
//       }
//       break;
//     case UNKNOWN:
//       //Serial.println("Unknown, Change to Cyan");
//       for (pixel = 0; pixel <= PIXELCOUNT; pixel++) {
//         odd_pixels->setPixelColor(pixel, CYAN);
//       }
//       break;
//     case DROIDDEPOT:
//       //Serial.println("Droid Depot, Change to Purple");
//       for (pixel = 0; pixel <= PIXELCOUNT; pixel++) {
//         odd_pixels->setPixelColor(pixel, PURPLE);
//       }
//       break;
//     case DOKONDARS:
//       //Serial.println("Dok Ondars, Change to Cyan");
//       for (pixel = 0; pixel <= PIXELCOUNT; pixel++) {
//         odd_pixels->setPixelColor(pixel, ORANGE);
//       }
//       break;
//     case FIRSTORDER:
//       //Serial.println("First Order, Change to Red");
//       for (pixel = 0; pixel <= PIXELCOUNT; pixel++) {
//         odd_pixels->setPixelColor(pixel, RED);
//       }
//       break;
//     case NOBEACON:
//       //Serial.println("No Scan Detected, Change to White");
//       for (pixel = 0; pixel <= PIXELCOUNT; pixel++) {
//         odd_pixels->setPixelColor(pixel, WHITE);
//       }
//       break;
//   }
//   odd_pixels->setBrightness(NEOBRIGHT);
//   odd_pixels->show();  // Send the updated pixel colors to the hardware.
//   delay(DelayOn);  // Pause Pixel On before next pass through loop
// }

void colorPulse() {
   int stripNumber = random(0,2);  
   int pixelNumber = random(0,3);
   int iDelay = random(50,201);
   int iBright = random(150,256);

//   if ((stripNumber == 0) && (pixelNumber == 0)) {
//     strip[stripNumber].setPixelColor(pixelNumber, strip[stripNumber].Color(255,0,0));
//   } else if ((stripNumber == 0) && (pixelNumber == 1)) {
//     strip[stripNumber].setPixelColor(pixelNumber, strip[stripNumber].Color(0,0,255));
//   } else if ((stripNumber == 0) && (pixelNumber == 2)) {
//     strip[stripNumber].setPixelColor(pixelNumber, strip[stripNumber].Color(178,132,190));
//   } else if ((stripNumber == 1) && (pixelNumber == 0)) {
//     strip[stripNumber].setPixelColor(pixelNumber, strip[stripNumber].Color(0,0,255));
//   } else if ((stripNumber == 1) && (pixelNumber == 1)) {
//     strip[stripNumber].setPixelColor(pixelNumber, strip[stripNumber].Color(255,255,0));
//   } else if ((stripNumber == 1) && (pixelNumber == 2)) {
//     strip[stripNumber].setPixelColor(pixelNumber, strip[stripNumber].Color(0,255,0));
//   }
//   delay(iDelay);
//   strip[stripNumber].setBrightness(iBright);
//   strip[stripNumber].show();
// }

void loop() {
  //  updatePixels();
  strip[0].clear();
  strip[1].clear();
  strip[0].setPixelColor(0,DARKPURPLE);
  strip[0].setPixelColor(1,RED);
  strip[0].setPixelColor(2,ORANGE);
  strip[1].setPixelColor(0,BLUE);
  strip[1].setPixelColor(1,GREEN);
  strip[1].setPixelColor(2,YELLOW);
  strip[0].show();
  strip[1].show();
  delay(1000);  
}
