
//#define LAYOUT_JAPANESE
//#include <HID_Keyboard.h>



// BLEを有効にするか否か
//#define __ENABLE_BLE__

// BLE有効か無効かで読み込むヘッダを切り替え
#ifdef __ENABLE_BLE__
  #include <KeyboardBLE.h>
#else
  #include <Keyboard.h>
  //#include <Keyboard_jp_JP.h>
#endif

// LED
#include <Adafruit_NeoPixel.h>
#define LED_PIN 16
// create a pixel strand with 1 pixel on PIN_NEOPIXEL
Adafruit_NeoPixel pixels(1, LED_PIN);

// I2C
#include <Wire.h>


// pin(row = out / col = in)
int pin_row[] = {28, 27, 26, 15, 14};
int row_len = sizeof(pin_row)/sizeof(pin_row[0]);
int pin_col[] = {3, 4, 5, 6, 7, 8};
int col_len = sizeof(pin_col)/sizeof(pin_col[0]);
int col_l_len = 6;
int col_r_len = 9;

// key_map
unsigned char key_map_layer = 0;
unsigned char key_map[2][sizeof(pin_row)/sizeof(pin_row[0])][15] = {
  {
    {0x60, 0x1E, 0x1F, 0x20, 0x21, 0x22,  0x23, 0x24, 0x25, 0x26, 0x27, 0x2D, 0x2E, 0x89, 0x2A},
    {0x2B, 0x14, 0x1A, 0x08, 0x15, 0x17,  0x1C, 0x18, 0x0C, 0x12, 0x13, 0x2F, 0x30, 0x28, 0x00},
    {0xE0, 0x04, 0x16, 0x07, 0x09, 0x0A,  0x0B, 0x0D, 0x0E, 0x0F, 0x33, 0x34, 0x32, 0x00, 0x00},
    {0xE1, 0x1D, 0x1B, 0x06, 0x19, 0x05,  0x11, 0x10, 0x36, 0x37, 0x38, 0x87, 0x52, 0xE5, 0x00},
    {0xFB, 0xFB, 0xE3, 0xE2, 0x2C, 0x00,  0xFB, 0x00, 0x00, 0x00, 0x00, 0x50, 0x51, 0x4F, 0x00}
  },
  {
    {0xB1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xFB, 0xFB, 0x00, 0x00, 0x00, 0x00,  0xFB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
  }
};

int chattering = 3;
bool key_state[sizeof(pin_row)/sizeof(pin_row[0])][15] = {0};
bool key_flag[sizeof(pin_row)/sizeof(pin_row[0])][15] = {false};
unsigned char key_input_count[sizeof(pin_row)/sizeof(pin_row[0])][15] = {0};

//
class KeyboardFunc{
private:
  
  //bool key_state[sizeof(pin_row)/sizeof(pin_row[0])][sizeof(pin_col)/sizeof(pin_col[0])] = {0};
  //bool key_flag[sizeof(pin_row)/sizeof(pin_row[0])][sizeof(pin_col)/sizeof(pin_col[0])] = {false};
  //unsigned char key_input_count[sizeof(pin_row)/sizeof(pin_row[0])][sizeof(pin_col)/sizeof(pin_col[0])] = {0};

public:
  // コンストラクタ 
  KeyboardFunc(){
  }

  // キーの状態更新
  void update(){
    for(int r = 0; r < row_len; r++){
      // 1行有効
      digitalWrite(pin_row[r], LOW);
      delay(1);

      // 各列確認
      for(int c = 0; c < col_len; c++){
        // 該当キーが押されているか？
        if(digitalRead(pin_col[c]) == LOW){
          // 該当キーのカウントアップ
          if(key_input_count[r][c] < chattering) key_input_count[r][c] += 1;
        } else {
          // 該当キーのカウントダウン
          if(key_input_count[r][c] > 0) key_input_count[r][c] = 0;
        }
        // カウントが規定値を超えているか？
        if(key_input_count[r][c] >= chattering){
          key_state[r][c] = true;
        } else {
          key_state[r][c] = false;
        }
      }

      // 1行無効
      digitalWrite(pin_row[r], HIGH);
      delay(1);
    }
  }

  
  // キーの送信
  void write(){
    for(int r = 0; r < row_len; r++){
      for(int c = 0; c < 15; c++){
        if(key_state[r][c] == true && key_flag[r][c] == false){
          //Keyboard.press(key_map[key_map_layer][r][c]);
          Keyboard.pressRaw(key_map[key_map_layer][r][c]);
          key_flag[r][c] = true;            // ボタンを押した
          delay(1);
          // 特殊キー以外の場合は初期化
          if(key_map[key_map_layer][r][c] != 0x81 && key_map[key_map_layer][r][c] != 0x82 && key_map[key_map_layer][r][c] != 0x83 
            && key_map[key_map_layer][r][c] != 0x84 && key_map[key_map_layer][r][c] != 0xFB){
            key_input_count[r][c] = -5;      // キー押下の初期化（連続防止）  
          }
          // レイヤー切り替え？
          if(key_map[key_map_layer][r][c] == 0xFB){
            key_map_layer = 1;
            // LED点灯
            pixels.setPixelColor(0, pixels.Color(125, 0, 0, 1));
            pixels.show();
          }
        } else if(key_state[r][c] == false) {
          // リリース
          if(key_flag[r][c] == true){
            //Keyboard.release(key_map[key_map_layer][r][c]);
            Keyboard.releaseRaw(key_map[key_map_layer][r][c]);
            delay(1);
            key_flag[r][c] = false;
            // レイヤー切り替え？
            if(key_map[key_map_layer][r][c] == 0xFB){
              key_map_layer = 0;
              // LED消灯
              pixels.clear();
              pixels.show();
            }
          }

        }
      }
    }
  }
  

};


void setup() {
  // BLE有効か無効かで初期化切り替え
  #ifdef __ENABLE_BLE__
    KeyboardBLE.begin();
    //Serial.begin(9600);
  #else
    Serial.begin(9600);
    Keyboard.begin();
  #endif

  // I2C設定
  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();

  // LED消灯
  pixels.clear();
  pixels.show();
  
  // 共通
  analogReadResolution(12);
  
  // 出力(row)
  for(int i = 0; i < row_len; i++){
    pinMode(pin_row[i], OUTPUT);
    digitalWrite(pin_row[i], HIGH);
  }
  // 入力(col)
  for(int i = 0; i < col_len; i++){
    pinMode(pin_col[i], INPUT_PULLUP);
  }
}


uint16_t raw;
float v;
int persent = 0, val = 0, count = 0, len = 0;
int read_size = 45;
KeyboardFunc func;

void loop() {
  func.update();

  
  // もう片方のキーボード入力状況を取得する
  len = Wire.requestFrom(0x30, read_size); //データ通信を要求
  Serial.println(len);
  if (len == read_size) {
    for(int r = 0; r < row_len; r++){
      for(int c = 0; c < 9; c++){
        val = Wire.read();
        if(val == 1) {
          key_state[r][col_l_len + c] = true;
          Serial.println(c);
        } else {
          key_state[r][col_l_len + c] = false;
        }    
      }
    }
  } else {
    while (Wire.available() > 0){
      Wire.read();
    }
  }  

  func.write();

}
