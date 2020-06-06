#include <stdio.h>
#include <string.h>
unsigned long time;
unsigned long prevMicros;
byte DB[4] = {50, 51, 52, 53};
//pinE:37
//pinRS:11

#define FASTADC 1

int cellsNumber = 2;
float current = 0.7;
float voltage = 8.04;
char processMode = 'D';//C - charge, B - balance, F - fast charge, S - storage, D - discharge.

int pin0 = 2;//StopButton
int pin1 = 3;//LeftButton
int pin2 = 4;//RightButton
int pin3 = 5;//StartButton


// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

//Massives of pins' vuluses for fast getting screen info
byte massE[300];
byte massRS[300];
byte massDB4[300];
byte massDB5[300];
byte massDB6[300];
byte massDB7[300];
byte pinb[300];
int res[300];
unsigned long ttime[300];
int c = 0;

//Screen massives.
char screen[2][16];
char screenPrev1[2][16];
char screenPrev2[2][16];
char resultScreen[2][16];
//cursor's coord
int cursorX = 0;
int cursorY = 0;

void setup() {

#if FASTADC
  // set prescale to 16
  sbi(ADCSRA, ADPS2) ;
  cbi(ADCSRA, ADPS1) ;
  cbi(ADCSRA, ADPS0) ;
#endif

  Serial.begin(115200);
  //Buttons' pins setup
  pinMode(pin0, OUTPUT);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
}

void loop() {

  delay(1000);
  switch (processMode)
  {
    case 'C':
      charge(cellsNumber, current, voltage);
      break;
    case 'B':
      balance(cellsNumber, current, voltage);
      break;
    case 'F':
      fastCHG(cellsNumber, current, voltage);
      break;
    case 'S':
      storage(cellsNumber, current, voltage);
      break;
    case 'D':
      discharge(cellsNumber, current, voltage);
      break;
  }
  delay(1000000);//Possible to change for a while, waiting of something.
}


//Refreshes global screen massive
void getScreen()
{
  prevMicros = micros();
  bool flag2 = 1;
  c = 0;
  while (flag2) {

    if (c < 300) {//Fast reading of 300 strings from LCD

      massE[c] = PINC;
      if (massE[c] == 1) {
        pinb[c] = PINB;
        ttime[c] = (unsigned long)(micros() - prevMicros);      c++;
      }
    }
    else {//Parsing of 300 strings from LCD
      bool flag = 0;
      int counter = 0;
      int PrevRS = 1;
      for (int i = 0; i < 300; i++)
      {
        if ((ttime[i] - ttime[i - 1] > 21)) {
          int lost = ((int)pinb[i] - 128);
          int RS = (int)(lost / 32);

          if (((RS == 0 && PrevRS == 1) || flag) & (counter < 80)) {
            flag = 1;
            massRS[counter] = RS;
            lost -= massRS[counter] * 32;
            massDB4[counter] = (int)(lost / 8);
            lost -= massDB4[counter] * 8;
            massDB5[counter] = (int)(lost / 4);
            lost -= massDB5[counter] * 4;
            massDB6[counter] = (int)(lost / 2);
            lost -= massDB6[counter] * 2;
            massDB7[counter] = lost;
            res[counter] = massDB4[counter] + massDB5[counter] * 2 + massDB6[counter] * 4 + massDB7[counter] * 8;
            counter++;
          }
          PrevRS = RS;
        }
      }

      //Clearing of the screen's massive.
      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 16; j++)
        {
          screen[i][j] = ' ';
        }
      }

      //Filling of the screen's massive by new info.
      int j = 1;
      while (j < 80)
      {
        switch (massRS[j]) {
          case 0:
            cursorY = massDB6[j - 1];
            res[j] = massDB4[j] + massDB5[j] * 2 + massDB6[j] * 4 + massDB7[j] * 8;
            cursorX = res[j];
            break;
          case 1:
            res[j - 1] = massDB4[j - 1] + massDB5[j - 1] * 2 + massDB6[j - 1] * 4 + massDB7[j - 1] * 8;
            res[j] = massDB4[j] + massDB5[j] * 2 + massDB6[j] * 4 + massDB7[j] * 8;
            res[j - 1] = res[j - 1] << 4;
            screen[cursorY][cursorX] = static_cast<char>(res[j - 1] + res[j]);
            cursorX++;
            break;
        }
        j += 2;
      }




      for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 16; j++)
        {
          /* //Info filtration.
          if (screen[i][j] == screenPrev1[i][j] && screenPrev1[i][j] == screenPrev2[i][j]) {
            resultScreen[i][j] = screen[i][j];
          }
          else
          {
            if (screen[i][j] != ' ')
              resultScreen[i][j] = screen[i][j];
            if (screen[i][j] != screenPrev2[i][j] && screenPrev2[i][j] != ' ' && ((screenPrev2[i][j] == screenPrev1[i][j]) || ( screenPrev2[i][j] != screenPrev1[i][j]) && screenPrev1[i][j] == ' '))
              resultScreen[i][j] = screenPrev2[i][j];
            if (screen[i][j] != screenPrev1[i][j] && screenPrev1[i][j] != ' ' && ((screenPrev1[i][j] == screenPrev2[i][j]) || ( screenPrev1[i][j] != screenPrev2[i][j]) && screenPrev2[i][j] == ' '))
              resultScreen[i][j] = screenPrev1[i][j];

            if (screenPrev2[i][j] == screenPrev1[i][j] && screenPrev1[i][j] != ' ')
              resultScreen[i][j] = screenPrev1[i][j];
          }

          screenPrev2[i][j] = screenPrev1[i][j];
          screenPrev1[i][j] = screen[i][j];*/
          resultScreen[i][j] = screen[i][j];
        }
      }


      flag2 = 0;
    }
  }
}

//Prints screen to the Serial.
void printScreen()
{
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++)
    {

      if (resultScreen[i][j] != 0) {
        Serial.print(resultScreen[i][j]);

      }
      else
      {
        Serial.print(" ");
      }
    }
    Serial.println();
  }
  Serial.println();
}

//Pushes stop button
void pushStop(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(pin0, 1);
    delay(100);
    digitalWrite(pin0, 0);
    delay(100);
  }
}

//Pushes left button n times
void pushLeft(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(pin1, 1);
    delay(100);
    digitalWrite(pin1, 0);
    delay(100);
  }
}

//Pushes right button n times
void pushRight(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(pin2, 1);
    delay(100);
    digitalWrite(pin2, 0);
    delay(100);
  }
}

//Pushes start button n times
void pushStart(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(pin3, 1);
    delay(100);
    digitalWrite(pin3, 0);
    delay(200);
  }
}

//Pushes start button for 5 sec n times
void pushStartLong(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(pin3, 1);
    delay(5000);
    digitalWrite(pin3, 0);
    delay(100);
  }
}

//Sets LiPo programs.
void setLiPo()
{
  getScreen();
  while (!(resultScreen[1][7] == 'L' & resultScreen[1][8] == 'i' &
           resultScreen[1][9] == 'P' & resultScreen[1][10] == 'o')) {
    pushStop(1);
    delay(100);
    printScreen();
    getScreen();
  }
  pushStart(1);
}

//Sets parametrs of process to the Charger.
void setParam(int cellsNum, float curr, float volt)
{
  pushStart(1);
  getScreen();
  char cur1 = (int)curr;
  char cur2 = curr * 10 - (int)curr * 10;
  while (!(resultScreen[1][0] == cur1 + 48
           & resultScreen[1][2] == cur2 + 48)) {
    if (((int)resultScreen[1][0] - 48 + (float)((int)resultScreen[1][2] - 48) / 10) < curr) {
      pushRight(1);
    }
    else {
      pushLeft(1);
    }
    delay(100);
    printScreen();
    getScreen();
  }

  pushStart(1);
  char cNum = cellsNum;
  while (!(resultScreen[1][13] == cNum + 48)) {
    if (resultScreen[1][13] < cNum + 48) {
      pushRight(1);
    }
    else {
      pushLeft(1);
    }
    delay(100);
    printScreen();
    getScreen();
  }
  pushStart(1);

  pushStartLong(1);
  pushStart(1);

}

//Charging process, until the voltage is lower than volt.
void charge(int cellsNum, float curr, float volt)
{
  setLiPo();
  getScreen();
  while (!(resultScreen[0][5] == 'C' & resultScreen[0][6] == 'H' &
           resultScreen[0][7] == 'A' & resultScreen[0][8] == 'R')) {
    pushRight(1);
    delay(100);
    printScreen();
    getScreen();
  }

  setParam(cellsNum, curr, volt);

  getScreen();
  float curVolt = ((int)resultScreen[0][11] - 48) + ((float)((int)resultScreen[0][13] - 48) /
                  10 + (float)((int)resultScreen[0][14] - 48) / 100);
  Serial.println(curVolt);
  Serial.println(11111);
  while (curVolt < volt) {
    curVolt = ((int)resultScreen[0][11] - 48) + ((float)((int)resultScreen[0][13] - 48) / 10
              + (float)((int)resultScreen[0][14] - 48) / 100);
    Serial.println(curVolt);
    delay(100);
    printScreen();
    getScreen();
  }
  pushStop(1);

}

//Smart charging with balancing of cells, until the voltage is lower than volt.
void balance(int cellsNum, float curr, float volt)
{
  setLiPo();
  getScreen();
  while (!(resultScreen[0][5] == 'B' & resultScreen[0][6] == 'A' &
           resultScreen[0][7] == 'L' & resultScreen[0][8] == 'A')) {
    pushRight(1);
    delay(100);
    printScreen();
    getScreen();
  }

  setParam(cellsNum, curr, volt);

  getScreen();
  float curVolt = ((int)resultScreen[0][11] - 48) + ((float)((int)resultScreen[0][13] - 48) / 10
                  + (float)((int)resultScreen[0][14] - 48) / 100);
  Serial.println(curVolt);
  Serial.println(11111);
  while (curVolt < volt) {
    curVolt = ((int)resultScreen[0][11] - 48) + ((float)((int)resultScreen[0][13] - 48) / 10
              + (float)((int)resultScreen[0][14] - 48) / 100);
    Serial.println(curVolt);
    delay(100);
    printScreen();
    getScreen();
  }
  pushStop(1);

}


//Fast charging, until the voltage is lower than volt.
void fastCHG(int cellsNum, float curr, float volt)
{
  setLiPo();
  getScreen();
  while (!(resultScreen[0][5] == 'F' & resultScreen[0][6] == 'A' &
           resultScreen[0][7] == 'S' & resultScreen[0][8] == 'T')) {
    pushRight(1);
    delay(100);
    printScreen();
    getScreen();
  }

  setParam(cellsNum, curr, volt);

  getScreen();
  float curVolt = ((int)resultScreen[0][11] - 48) + ((float)((int)resultScreen[0][13] - 48) / 10
                  + (float)((int)resultScreen[0][14] - 48) / 100);
  Serial.println(curVolt);
  Serial.println(11111);
  while (curVolt < volt) {
    curVolt = ((int)resultScreen[0][11] - 48) + ((float)((int)resultScreen[0][13] - 48) / 10
              + (float)((int)resultScreen[0][14] - 48) / 100);
    Serial.println(curVolt);
    delay(100);
    printScreen();
    getScreen();
  }
  pushStop(1);
}

//Starting storage process
void storage(int cellsNum, float curr, float volt)
{
  setLiPo();
  getScreen();
  while (!(resultScreen[0][5] == 'S' & resultScreen[0][6] == 'T' &
           resultScreen[0][7] == 'O' & resultScreen[0][8] == 'R')) {
    pushRight(1);
    delay(100);
    printScreen();
    getScreen();
  }

  setParam(cellsNum, curr, volt);

}


//Discharging, until the voltage is under than volt.
void discharge(int cellsNum, float curr, float volt)
{
  setLiPo();
  getScreen();
  while (!(resultScreen[0][5] == 'D' & resultScreen[0][6] == 'I' &
           resultScreen[0][7] == 'S' & resultScreen[0][8] == 'C')) {
    pushRight(1);
    delay(100);
    printScreen();
    getScreen();
  }

  setParam(cellsNum, curr, volt);

  getScreen();
  float curVolt = ((int)resultScreen[0][11] - 48) + ((float)((int)resultScreen[0][13] - 48) / 10
                  + (float)((int)resultScreen[0][14] - 48) / 100);
  Serial.println(curVolt);
  Serial.println(11111);
  while (curVolt > volt) {
    curVolt = ((int)resultScreen[0][11] - 48) + ((float)((int)resultScreen[0][13] - 48) / 10
              + (float)((int)resultScreen[0][14] - 48) / 100);
    Serial.println(curVolt);
    delay(100);
    printScreen();
    getScreen();
  }
  pushStop(1);

}






