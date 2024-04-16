
// USB関連
#include <Keyboard.h>
#include <Mouse.h>

// LED
#include <Adafruit_NeoPixel.h>
#define LED_PIN 16
Adafruit_NeoPixel pixels(1, LED_PIN);

// I2C
#include <Wire.h>

// pin(row = out / col = in)
#define KEY_ROW 5
#define L_KEY_COL 6
#define R_KEY_COL 9
#define KEY_STOP_NUM 20
int pin_row[] = {28, 27, 26, 15, 14};
int row_len = sizeof(pin_row)/sizeof(pin_row[0]);
int pin_col[] = {3, 4, 5, 6, 7, 8};
int col_len = sizeof(pin_col)/sizeof(pin_col[0]);
int col_l_len = 6;
int col_r_len = 9;

// key_map
// 独自割り当て : 0xFB/レイヤ切り替え　0xFC/左クリック 0xFD/右クリック
//               0xF0/マウス↑ 0xF1/マウス↓ 0xF2/マウス← 0xF3:マウス→
unsigned char key_map_layer = 0;
unsigned char key_map[2][sizeof(pin_row)/sizeof(pin_row[0])][15] = {
  {
    {0x35, 0x1E, 0x1F, 0x20, 0x21, 0x22,  0x23, 0x24, 0x25, 0x26, 0x27, 0x2D, 0x2E, 0x89, 0x2A},
    {0x2B, 0x14, 0x1A, 0x08, 0x15, 0x17,  0x1C, 0x18, 0x0C, 0x12, 0x13, 0x2F, 0x30, 0x28, 0x00},
    {0xE0, 0x04, 0x16, 0x07, 0x09, 0x0A,  0x0B, 0x0D, 0x0E, 0x0F, 0x33, 0x34, 0x32, 0x00, 0x00},
    {0xE1, 0x1D, 0x1B, 0x06, 0x19, 0x05,  0x11, 0x10, 0x36, 0x37, 0x38, 0x87, 0x52, 0xE5, 0x00},
    {0xE0, 0xFB, 0xE3, 0xE2, 0x2C, 0x00,  0xFB, 0x3B, 0x3C, 0x3D, 0x3E, 0x50, 0x51, 0x4F, 0x00}
  },
  {
    {0x29, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E,  0x3F, 0x40, 0x41, 0x42, 0x00, 0x00, 0x00, 0x00, 0x4C},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xE0, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xE1, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xE0, 0xFB, 0xE3, 0x2A, 0x28, 0x00,  0xFB, 0x3B, 0x3C, 0x3D, 0x3E, 0x00, 0x00, 0x00, 0x00}
  }
};
int chattering = 3;
bool key_state[sizeof(pin_row)/sizeof(pin_row[0])][15] = {0};
bool key_flag[sizeof(pin_row)/sizeof(pin_row[0])][15] = {false};
int key_input_count[sizeof(pin_row)/sizeof(pin_row[0])][15] = {0};
int key_stop[sizeof(pin_row)/sizeof(pin_row[0])][15] = {0};

// マウス移動量
#define MOUSE_SPEED_MAX 8
#define MOUSE_SPEED_MIN 0
int mouse_move[4] = {MOUSE_SPEED_MIN, MOUSE_SPEED_MIN, MOUSE_SPEED_MIN, MOUSE_SPEED_MIN};

//
class KeyboardFunc{
private:
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

        /////////////////////////////////////////////////////////////////  
        // ボタンを押したと判断
        if(key_state[r][c] == true && key_flag[r][c] == false){
          delay(1);
          key_flag[r][c] = true;            // ボタンを押した

          // レイヤー切り替え
          if(key_map[key_map_layer][r][c] == 0xFB){
            key_map_layer = 1;
          }
          // 押しっぱなしのキー
          else if(key_map[key_map_layer][r][c] == 0xE0 || key_map[key_map_layer][r][c] == 0xE1 
            || key_map[key_map_layer][r][c] == 0xE2 || key_map[key_map_layer][r][c] == 0xE3){
            Keyboard.pressRaw(key_map[key_map_layer][r][c]);
          }
          // キーボード入力
          else if(key_map[key_map_layer][r][c] != 0xFB){
            Keyboard.writeRaw(key_map[key_map_layer][r][c]);
          }

        /////////////////////////////////////////////////////////////////
        // ボタン押しっぱなしと判断
        } else if(key_state[r][c] == true && key_flag[r][c] == true){
          // 特殊キー以外はキーを放していない場合に連打させる
          if(key_map[key_map_layer][r][c] != 0xFB && key_map[key_map_layer][r][c] != 0xE0 && 
            key_map[key_map_layer][r][c] != 0xE1 && key_map[key_map_layer][r][c] != 0xE2 && key_map[key_map_layer][r][c] != 0xE3){
            // 連打する前に少し待つ
            key_stop[r][c]++;
            if(key_stop[r][c] > KEY_STOP_NUM){
              Keyboard.writeRaw(key_map[key_map_layer][r][c]);
            }
            
          }

        /////////////////////////////////////////////////////////////////
        // ボタンを離したと判断
        } else if(key_state[r][c] == false) {
          // リリース
          if(key_flag[r][c] == true){
            delay(1);
            // 初期化
            key_flag[r][c] = false;
            key_stop[r][c] = 0;

            // レイヤー切り替え
            if(key_map[key_map_layer][r][c] == 0xFB){
              key_map_layer = 0;
            }
            // キーボード解放
            else if(key_map[key_map_layer][r][c] == 0xE0 || key_map[key_map_layer][r][c] == 0xE1 
              || key_map[key_map_layer][r][c] == 0xE2 || key_map[key_map_layer][r][c] == 0xE3){
              //Serial.printf("r: %x\n", key_map[key_map_layer][r][c]);
              Keyboard.releaseRaw(key_map[key_map_layer][r][c]);
            }                  
          }
        }


      }
    }
  }
};




//------------------------------------------------------------------------------------------------
// Core0 : I2C処理
int val = 0, count = 0, len = 0, r = 0, c = 0, read_size = 7;

void setup() {
  //Serial.begin(9600);
  // I2C設定
  Wire.setClock(400000);
  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin();
}

void loop() {
  len = Wire.requestFrom(0x30, read_size); //データ通信を要求
  //Serial.println(len);

  if (len == read_size) {
    count = 0; r = 0; c = 0;

    for(int i = 0; i < read_size; i++) {
      val = Wire.read();
      //Serial.printf("%x\n", val);

      // 先頭バイトを確認
      if(i == 0 && val != 0xAA) break;
      if(i == 0) continue;

      // 受信バイトを解析
      for(int s = 0; s < 8; s++){
        if((val & (byte)(1 << 7 - s)) > 0) {
          //Serial.printf("r:%d, c:%d\n", r, c);
          key_state[r][L_KEY_COL + c] = true;
        } else {
          key_state[r][L_KEY_COL + c] = false;
        }

        count++; c++;
        // 片方の列数分実行したら、行を進めて列を戻す
        if(count % R_KEY_COL == 0) {
          r++;
          c = 0;
        }
        // 終了判定
        if(count >= (R_KEY_COL * KEY_ROW)) break;
      }
    }
    //Serial.println("-------------------");
  }

  // ゴミ処理
  while (Wire.available() > 0){
    Wire.read();
  } 

  delay(10);
}



//------------------------------------------------------------------------------------------------
// Core1 : キーボード入力処理
void setup1() {
  Serial.begin(9600);
  Keyboard.begin();
  Mouse.begin();

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

KeyboardFunc func;
void loop1() {
  func.update();
  func.write();
}




