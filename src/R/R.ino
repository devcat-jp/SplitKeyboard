
#include <Wire.h>         // I2C
#include <Keyboard.h>     // キーボード

// LED
#include <Adafruit_NeoPixel.h>
#define LED_PIN 16
// create a pixel strand with 1 pixel on PIN_NEOPIXEL
Adafruit_NeoPixel pixels(1, LED_PIN);

// i2c用のキーボード入力状態
byte send_key_state[45] = {0};

// キーボードのピンアサイン
// pin(row = out / col = in)
int pin_row[] = {28, 27, 26, 15, 14};
int row_len = sizeof(pin_row)/sizeof(pin_row[0]);
int pin_col[] = {3, 4, 5, 6, 7, 8, 9, 10, 11};
int col_len = sizeof(pin_col)/sizeof(pin_col[0]);
#define KEY_ROW 6
#define KEY_COL 9

//
int chattering = 3;
bool key_state[sizeof(pin_row)/sizeof(pin_row[0])][sizeof(pin_col)/sizeof(pin_col[0])] = {0};
bool key_flag[sizeof(pin_row)/sizeof(pin_row[0])][sizeof(pin_col)/sizeof(pin_col[0])] = {false};
unsigned char key_input_count[sizeof(pin_row)/sizeof(pin_row[0])][sizeof(pin_col)/sizeof(pin_col[0])] = {0};


//------------------------------------------------------------------------------------------------
// I2C
void receiveEvent(int num) {
}

/*
void requestEvent() {
  int count = 0;
  for(int r = 0; r < row_len; r++){
    for(int c = 0; c < col_len; c++){
      if(key_state[r][c] == true)
      {
        send_key_state[count] = 1;
        //key_state[r][c] = false;
        //key_input_count[r][c] = 0;
      } else {
        send_key_state[count] = 0;
        //key_state[r][c] = false;
      }
      count++;
    }
  }
  Wire.write(send_key_state, 45);
}
*/

void requestEvent() {
  byte send_key_serial[6] = {0};
  int count = 0; int num = 0;
  for(int r = 0; r < KEY_ROW; r++){
    for(int c = 0; c < KEY_COL; c++){
      if(key_state[r][c] == true)
      {
        send_key_serial[num] |= (byte)(1 << 7 - count);
      }
      count++;

      // 1byte分終了
      if(count == 8) {
        count = 0;
        num++;
      }
    }
  }
  Wire.write(send_key_serial, 6);

  //for(int i = 0; i < 6; i++) {
  //  Serial.printf("%x\n", send_key_serial[i]);
  //}
  //Serial.println("---------------------\n");
}



//------------------------------------------------------------------------------------------------
class KeyboardFunc{
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
          // LED点灯
          pixels.setPixelColor(0, pixels.Color(125, 0, 0, 1));
          pixels.show();
        } else {
          key_state[r][c] = false;
          // LED消灯
          pixels.clear();
          pixels.show();
        }
      }

      // 1行無効
      digitalWrite(pin_row[r], HIGH);
      delay(1);
    }
  }

};


//------------------------------------------------------------------------------------------------
void setup() {

  // I2C設定
  Wire.setSDA(0);
  Wire.setSCL(1);
  Wire.begin(0x30);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  // 出力(row)
  for(int i = 0; i < row_len; i++){
    pinMode(pin_row[i], OUTPUT);
    digitalWrite(pin_row[i], HIGH);
  }
  // 入力(col)
  for(int i = 0; i < col_len; i++){
    pinMode(pin_col[i], INPUT_PULLUP);
  }

  // シリアル通信
  Serial.begin(9600);
}


//------------------------------------------------------------------------------------------------
KeyboardFunc func;
void loop() {
  func.update();
  //func.write();

  //digitalWrite(16, HIGH);
  //delay(250);
  //digitalWrite(16, LOW);
  //delay(250);
  
  //delay(1);
}
