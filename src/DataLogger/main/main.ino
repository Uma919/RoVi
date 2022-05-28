#define M5STACK_MPU6886 
#include <M5Stack.h>
#include <Ticker.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;
Ticker tk;


float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;


char gps_data[100];
static const uint32_t GPSBaud = 9600;
HardwareSerial ss(2);


/* 割込み処理 */
void onTimer(){
  if (gps.location.isValid()){
    File file = SD.open("/data.csv", FILE_APPEND);
    sprintf(gps_data, "%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", gps.satellites.value(), gps.location.lat(), gps.location.lng(), gps.altitude.meters(), gps.speed.kmph(), gps.course.deg(), accX, accY, accZ, gyroX, gyroY, gyroZ);
    file.print(gps_data);
    file.close(); 
  }
}

void setup(){
  M5.begin();
  M5.IMU.Init();
  ss.begin(GPSBaud);
  
  /* タイマー割込み処理 */
  tk.attach_ms(100, onTimer);
  
  /* 衛星受信機数 */
  M5.Lcd.setCursor(20, 40, 2);
  M5.Lcd.println("Sats [-]");
  
  /* 緯度・経度・高度 */
  M5.Lcd.setCursor(20, 60, 2);
  M5.Lcd.println("Latitude [deg]");
  M5.Lcd.setCursor(20, 80, 2);
  M5.Lcd.println("Longitude [deg]");
  M5.Lcd.setCursor(20, 100, 2);
  M5.Lcd.println("Altitude [m]");

  /* 移動速度・移動方向 */
  M5.Lcd.setCursor(227, 140, 4);
  M5.Lcd.println("[km/h]");
  M5.Lcd.setCursor(227, 190, 4);
  M5.Lcd.println("[deg]");
}

void loop(){
  M5.update();
  
  /* 姿勢角 */
  M5.IMU.getAccelData(&accX,&accY,&accZ);
  M5.IMU.getGyroData(&gyroX,&gyroY,&gyroZ);
  
  /* 時刻 */
  M5.Lcd.setCursor(20, 20, 2);
  printDateTime(gps.date, gps.time);
  M5.Lcd.println();

  /* 衛星受信機数 */
  M5.Lcd.setCursor(150, 40, 2);
  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  M5.Lcd.println();

  /* 緯度・経度・高度 */
  M5.Lcd.setCursor(150, 60, 2);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  M5.Lcd.println();
  M5.Lcd.setCursor(150, 80, 2);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  M5.Lcd.println();
  M5.Lcd.setCursor(150, 100, 2);
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  M5.Lcd.println();

  /* 移動速度・移動方向 */
  M5.Lcd.setCursor(40, 130, 6);
  printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  M5.Lcd.setCursor(40, 180, 6);
  printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
  M5.Lcd.println();
  
  smartDelay(100);
  if (millis() > 5000 && gps.charsProcessed() < 10)
    M5.Lcd.println(F("No GPS data received: check wiring"));
}


// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms){
  unsigned long start = millis();
  do{
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}


static void printFloat(float val, bool valid, int len, int prec){
  if (!valid){
    while (len-- > 1)
      M5.Lcd.print('*');
    M5.Lcd.print(' ');
  }else{
    M5.Lcd.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      M5.Lcd.print(' ');
  }
  smartDelay(0);
}


static void printInt(unsigned long val, bool valid, int len){
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  M5.Lcd.print(sz);
  smartDelay(0);
}


static void printDateTime(TinyGPSDate &d, TinyGPSTime &t){
  if (!d.isValid()){
    M5.Lcd.print(F("********** "));
  }else{
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.year(), d.month(), d.day());
    M5.Lcd.print(sz);
  }
  
  if (!t.isValid()){
    M5.Lcd.print(F("******"));
  }else{
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d", t.hour()+9, t.minute(), t.second());
    M5.Lcd.print(sz);
  }

  smartDelay(0);
}
