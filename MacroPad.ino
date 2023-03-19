#include <BleKeyboard.h>
BleKeyboard ble("Macro Pad", "Viverino1", 100);
int buttonPins[] = {13, 12, 14, 26, 33, 15};
int longPressTime = 300;

#define dataPin 32
#define latchPin 25
#define clockPin 27
void shiftBytes(byte shiftByte1, byte shiftByte2, byte shiftByte3)
{
   digitalWrite(latchPin, LOW);
   shiftOut(dataPin, clockPin, LSBFIRST, shiftByte3);
   shiftOut(dataPin, clockPin, LSBFIRST, shiftByte2);
   shiftOut(dataPin, clockPin, LSBFIRST, shiftByte1);
   digitalWrite(latchPin, HIGH);
}

void SwitchStatements(int key){
  switch(key){
    case 111101:
      ble.press(KEY_LEFT_ALT);
      ble.press(KEY_TAB);
      delay(100);
      ble.releaseAll();
      break;
    case 101111:
      ble.write(KEY_MEDIA_PLAY_PAUSE);
      break;
    case 11111:
      ble.press(KEY_LEFT_CTRL);
      ble.press(KEY_LEFT_SHIFT);
      ble.press(KEY_TAB);
      ble.releaseAll();
      delay(100);
      break;
    case 111110:
      ble.press(KEY_LEFT_CTRL);
      ble.press(KEY_TAB);
      delay(100);
      ble.releaseAll();
      break;
    case 110111:
      ble.write(KEY_MEDIA_VOLUME_UP);
      break;
    case 111011:
      ble.write(KEY_MEDIA_VOLUME_DOWN);
      break;
    default: break;
  }
}

void SwitchStatementsLong(int key){
  switch(key){
    case 111101:
      ble.press(KEY_LEFT_GUI);
      ble.press(KEY_TAB);
      delay(100);
      ble.releaseAll();
      break;
    case 101111:
      ble.write(KEY_MEDIA_MUTE);
      break;
    case 110111:
      ble.write(KEY_MEDIA_NEXT_TRACK);
      break;
    case 111011:
      ble.write(KEY_MEDIA_PREVIOUS_TRACK);
      break;
    case 10101:
      shiftBytes(0b11111111, 0b11111111, 0b11111111);
      Serial.println("shifted");
      break;
    case 101010:
      shiftBytes(0b10000000, 0b00000000, 0b00000000);
      break;
    default: break;
  }
}

void setup() {
  // put your setup code here, to run once:
  setCpuFrequencyMhz(80);
  Serial.begin(9600);
  //Serial.println(getXtalFrequencyMhz());  
  ble.begin();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_12,0);
  
  pinMode(2, OUTPUT);

  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  
  pinMode(13, INPUT_PULLDOWN);
  pinMode(12, INPUT_PULLDOWN);
  pinMode(14, INPUT_PULLDOWN);
  pinMode(26, INPUT_PULLUP);
  pinMode(33, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
}

bool bleActive = false;
void loop() {
  // put your main code here, to run repeatedly:
  if(ble.isConnected()){
    if(bleActive == false){
      bleActive = true;
      shiftBytes(0b11111111, 0b11111111, 0b11111111);
    }
    digitalWrite(2, LOW);
    macro(getbuttonInput());
  }else{
    //shiftBytes(0b10000000, 0b00000000, 0b00000000);
    digitalWrite(2, HIGH);
  }
}

int getbuttonInput(){
  int input[6];
  for(int i = 0; i < 3; i++){
    input[i] = !digitalRead(buttonPins[i]);
  }
  for(int i = 3; i < 6; i++){
    input[i] = digitalRead(buttonPins[i]);
  }

  int multiplier = 1;
  int output = 0;
  for(int i = 0; i < 6; i++){
    output += input[i] * multiplier;
    multiplier *= 10;
  }
  return output;
}

bool keyDown;
int64_t keyDownTime;
int lastKeyPressed;
bool longPressed;
bool keyCombo;
int lastButtonSum = 0;
void macro(int input){
  if(input != 111111){
    if(keyDown == false){
      keyDown = true;
      lastKeyPressed = input;
      keyDownTime = esp_timer_get_time();
      Serial.print("Key Down: ");
      Serial.println(lastKeyPressed);
      //digitalWrite(2, HIGH);
    }

    if(input != lastKeyPressed && lastButtonSum > findSum(input)){
      lastButtonSum = findSum(input);
      lastKeyPressed = input;
      keyDownTime = esp_timer_get_time();
      Serial.print("Combo Down: ");
      Serial.println(lastKeyPressed);
//      digitalWrite(2, HIGH);
    }

    if(esp_timer_get_time() - keyDownTime > longPressTime * 1000 && longPressed == false){
      longPressed = true;
      keyDown = true;
      SwitchStatementsLong(lastKeyPressed);
      Serial.print("Long Press: ");
      Serial.println(lastKeyPressed);
//      digitalWrite(2, LOW);
    }
  }else{
    if(keyDown == true){
      keyDown = false;
      lastButtonSum = 7;
      //digitalWrite(2, LOW);
      Serial.print("Key Up: ");
      Serial.println(lastKeyPressed);
      if(longPressed == false){
        SwitchStatements(lastKeyPressed);
      }else{
        longPressed = false;
      }
    }
  }
}

int findSum(int num){
  int sum = 0, remainder;
  while(num != 0){
    remainder = num % 10;
    sum = sum + remainder;
    num = num / 10;
  }
  return sum;
}
