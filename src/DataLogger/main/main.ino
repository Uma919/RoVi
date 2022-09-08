#define M5STACK_MPU6886
#include <M5Stack.h>
#include <Ticker.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;
Ticker tk;

/*/ CSVファイル /*/
int csv_save = 0;
char csv_sens[100]; // 慣性計測ファイル
char csv_repo[100]; // 道路異常ファイル

/*/ データ格納用 /*/
char dat_sens[100]; // 慣性計測データ
char dat_repo[100]; // 道路異常データ

/*/ GPS /*/
static const uint32_t GPSBaud = 9600;
HardwareSerial ss(2);

/*/ IMU /*/
float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;
float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

/*/ 割込み処理 /*/
void onTimer(){
  if (gps.location.isValid() && csv_save == 1){
    File file = SD.open(csv_sens, FILE_APPEND);
    sprintf(dat_sens, "%f,%f,%f,%f,%f,%f,%f,%f\n", gps.location.lat(), gps.location.lng(), accX, accY, accZ, gyroX, gyroY, gyroZ);
    file.print(dat_sens);
    file.close(); 
  }
}

/*/ メイン処理 /*/
void setup(){
  M5.begin();
  M5.IMU.Init();
  ss.begin(GPSBaud);
  
  /** タイマー割込み処理 **/
  tk.attach_ms(100, onTimer);
  
  /** 衛星受信機数 **/
  M5.Lcd.setCursor(20, 30, 2);
  M5.Lcd.println("Sats [-]");
  
  /** 緯度・経度・高度 **/
  M5.Lcd.setCursor(20, 50, 2);
  M5.Lcd.println("Latitude [deg]");
  M5.Lcd.setCursor(20, 70, 2);
  M5.Lcd.println("Longitude [deg]");
  M5.Lcd.setCursor(20, 90, 2);
  M5.Lcd.println("Altitude [m]");

  /** 移動速度・移動方向 **/
  M5.Lcd.setCursor(227, 125, 4);
  M5.Lcd.println("[km/h]");
  M5.Lcd.setCursor(227, 175, 4);
  M5.Lcd.println("[deg]");

  /** ボタン説明 **/
  M5.Lcd.setCursor(25, 215, 2);
  M5.Lcd.println("[Report]Step");
  M5.Lcd.setCursor(125, 215, 2);
  M5.Lcd.println("Start/Stop");
  M5.Lcd.setCursor(210, 215, 2);
  M5.Lcd.println("[Report]Grass");

  /* アイコン表示 */
  M5.Lcd.fillRect(255, 60, 40, 40, RED);
  M5.Lcd.drawRect(250, 55, 50, 50, RED);
  M5.Lcd.fillRect(268, 55, 14, 25, BLACK);
  M5.Lcd.fillTriangle(260, 80, 290, 80, 275, 95, BLACK);
}

void loop(){
  M5.update();
  
  if(gps.location.isValid()){
    /** CSV保存切替 **/
    if(M5.BtnB.wasPressed()){
      csv_save = 1 - csv_save;
      if(csv_save == 1){
        /* CSV新規作成 */
        int hr = gps.time.hour() + 9;
        if(hr >= 24)hr -= 24;
        sprintf(csv_sens, "/sens_%02d%02d%02d.csv", hr, gps.time.minute(), gps.time.second());
        sprintf(csv_repo, "/repo_%02d%02d%02d.csv", hr, gps.time.minute(), gps.time.second());
        File fle_sens = SD.open(csv_sens, FILE_WRITE);
        fle_sens.close(); 
        File fle_repo = SD.open(csv_repo, FILE_WRITE);
        fle_repo.close();
        
        /* アイコン表示 */
        M5.Lcd.setCursor(180, 10, 2);
        M5.Lcd.print(csv_sens);
        M5.Lcd.setCursor(180, 30, 2);
        M5.Lcd.print(csv_repo);
        M5.Lcd.fillRect(255, 60, 40, 40, GREEN);
        M5.Lcd.drawRect(250, 55, 50, 50, GREEN);
        M5.Lcd.fillRect(268, 55, 14, 25, BLACK);
        M5.Lcd.fillTriangle(260, 80, 290, 80, 275, 95, BLACK);
      }else{
        /* アイコン表示 */
        M5.Lcd.fillRect(255, 60, 40, 40, RED);
        M5.Lcd.drawRect(250, 55, 50, 50, RED);
        M5.Lcd.fillRect(268, 55, 14, 25, BLACK);
        M5.Lcd.fillTriangle(260, 80, 290, 80, 275, 95, BLACK);
      }
    
    /** 道路異常入力 **/
    }else if(M5.BtnA.wasPressed() && csv_save == 1){
      File file = SD.open(csv_repo, FILE_APPEND);
      sprintf(dat_repo, "%f,%f,%s\n", gps.location.lat(), gps.location.lng(), "step");
      file.print(dat_repo);
      file.close();
    }else if(M5.BtnC.wasPressed() && csv_save == 1){
      File file = SD.open(csv_repo, FILE_APPEND);
      sprintf(dat_repo, "%f,%f,%s\n", gps.location.lat(), gps.location.lng(), "grass");
      file.print(dat_repo);
      file.close();
    }
  }

  /** 姿勢角 **/
  M5.IMU.getAccelData(&accX,&accY,&accZ);
  M5.IMU.getGyroData(&gyroX,&gyroY,&gyroZ);
  
  /** 時刻 **/
  M5.Lcd.setCursor(20, 10, 2);
  printDateTime(gps.date, gps.time);
  M5.Lcd.println();

  /** 衛星受信機数 **/
  M5.Lcd.setCursor(150, 30, 2);
  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  M5.Lcd.println();

  /** 緯度・経度・高度 **/
  M5.Lcd.setCursor(150, 50, 2);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  M5.Lcd.println();
  M5.Lcd.setCursor(150, 70, 2);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  M5.Lcd.println();
  M5.Lcd.setCursor(150, 90, 2);
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  M5.Lcd.println();

  /** 移動速度・移動方向 **/
  M5.Lcd.setCursor(40, 115, 6);
  printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  M5.Lcd.setCursor(40, 165, 6);
  printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
  M5.Lcd.println();
  
  smartDelay(100);
  if (millis() > 5000 && gps.charsProcessed() < 10){
    M5.Lcd.println(F("No GPS data received: check wiring"));
  }
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
  if (d.isValid() && t.isValid()){
    int dy = d.day();
    int hr = t.hour() + 9;
    if(hr >= 24){
      dy ++;
      hr -= 24;
    }  
    char sz[64];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d", d.year(), d.month(), dy, hr, t.minute(), t.second());
    M5.Lcd.print(sz);
  }else{
    M5.Lcd.print(F("********** ******"));
  }
  smartDelay(0);
}