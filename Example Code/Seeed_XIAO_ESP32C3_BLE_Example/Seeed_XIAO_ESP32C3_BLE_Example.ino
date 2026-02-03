#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Define BLE service and characteristic UUIDs
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

void setup() {
Serial.begin(115200);

// Initialize BLE
BLEDevice::init("XIAO_ESP32C3_BLE");
BLEServer *pServer = BLEDevice::createServer();

// Callback for connection events
pServer->setCallbacks(new BLEServerCallbacks());

// Create BLE service
BLEService *pService = pServer->createService(SERVICE_UUID);

// Create BLE characteristic
pCharacteristic = pService->createCharacteristic(
CHARACTERISTIC_UUID,
BLECharacteristic::PROPERTY_READ |
BLECharacteristic::PROPERTY_WRITE |
BLECharacteristic::PROPERTY_NOTIFY
);

// Add descriptor for notifications
pCharacteristic->addDescriptor(new BLE2902());

// Start the service
pService->start();

// Start advertising
BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
pAdvertising->start();
Serial.println("Waiting for a client to connect...");
}

void loop() {
// Check if a device is connected
if (deviceConnected) {
// Send data to the connected device
pCharacteristic->setValue("Hello from XIAO ESP32C3!");
pCharacteristic->notify();
delay(1000);
}
}

// Callback class for connection events
class BLEServerCallbacks : public BLEServerCallbacks {
void onConnect(BLEServer* pServer) {
deviceConnected = true;
Serial.println("Device connected");
}

void onDisconnect(BLEServer* pServer) {
deviceConnected = false;
Serial.println("Device disconnected");
}
};