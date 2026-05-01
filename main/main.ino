// Infant Warmer - Continuous Load Cell Display on LCD
#include <HX711_ADC.h>
#include <LiquidCrystal_I2C.h>

// HX711 pins (adjust if needed)
const int HX711_DOUT = D6;
const int HX711_SCK  = D7;

// LCD I2C address and size
LiquidCrystal_I2C lcd(0x27, 16, 2);

HX711_ADC LoadCell(HX711_DOUT, HX711_SCK);

// === CALIBRATION VALUES (update with your measured values) ===
float calibrationFactor = -94.22;   // Replace with your actual calibration factor
long tareOffset = 8347099;          // Replace with your actual tare offset value

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Starting...");

  // Initialise scale
  LoadCell.begin();
  
  unsigned long stabilizingtime = 2000;
  boolean _tare = false;  // Don't tare on startup, we'll use saved offset
  LoadCell.start(stabilizingtime, _tare);
  
  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  } else {
    Serial.println("Startup complete");
  }

  // Apply saved calibration factor and tare offset
  LoadCell.setCalFactor(calibrationFactor);
  LoadCell.setTareOffset(tareOffset);
  
  Serial.println("Calibration and tare offset applied.");
  Serial.print("Calibration Factor: ");
  Serial.println(calibrationFactor);
  Serial.print("Tare Offset: ");
  Serial.println(tareOffset);

  // Initialise LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Infant Warmer");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Weight (g):");
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 500; // Print every 500ms
  static unsigned long t = 0;

  // Check for new data/start next conversion
  if (LoadCell.update()) {
    newDataReady = true;
  }

  // Get smoothed value from the dataset
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float weight = LoadCell.getData();
      
      // Display on LCD
      lcd.setCursor(0, 1);
      lcd.print("                ");  // clear line
      lcd.setCursor(0, 1);
      lcd.print(weight, 1);            // one decimal place
      lcd.print(" g");

      // Print to Serial Monitor
      Serial.print("Load_cell output val: ");
      Serial.println(weight);
      
      newDataReady = 0;
      t = millis();
    }
  }
  
  // Check for errors
  if (LoadCell.getTareTimeoutFlag() || LoadCell.getSignalTimeoutFlag()) {
    lcd.setCursor(0, 1);
    lcd.print("Sensor Error!   ");
    Serial.println("HX711 error detected.");
  }
}
