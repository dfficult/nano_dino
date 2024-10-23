// Nano dino , play on the go!
// By Dfficult 2024.10.19

// ===== Hardware Setup =====
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD I2C address 0x27
const int buttonPin = 2;  // Pin where the button is connected
const int buzzerPin = 4;  // Pin where the passive buzzer is connected
int buttonState = 0;      // Variable for reading the button status
int lastButtonState = 1;  // Store the last button state
int pressCount = 0;       // Count the number of button presses


// ===== Characters =====
// designed in minecraft
byte dino_1[8] = {
  B00111,
  B00101,
  B00111,
  B10110,
  B11111,
  B01110,
  B00110,
  B00100
};
byte dino_2[8] = {
  B00111,
  B00101,
  B00111,
  B10110,
  B11111,
  B01110,
  B00110,
  B00010
};
byte cacti_1[8] = {
  B00000,
  B00000,
  B00100,
  B00101,
  B10101,
  B10110,
  B01100,
  B00100
};
byte cacti_2[8] = {
  B00000,
  B00100,
  B00101,
  B10101,
  B10110,
  B01100,
  B00100,
  B00100
};
byte cacti_3[8] = {
  B00100,
  B10100,
  B10101,
  B01101,
  B00110,
  B00100,
  B00100,
  B00100
};
byte bird[8] = {
  B00000,
  B00100,
  B00110,
  B11111,
  B01111,
  B00000,
  B00000,
  B00000
};


// ===== Sounds =====
void playDeadSound(){
  // plays dead sound
  tone(buzzerPin, 100, 100);
  delay(125);
  tone(buzzerPin, 100, 100);
}
void playJumpSound(){
  // plays jump sound
  tone(buzzerPin, 500, 200);
}
void playLevelUpSound(){
  // plays level up sound
  tone(buzzerPin, 500, 200);
  delay(125);
  tone(buzzerPin, 825, 175);
}


// ===== Game settings and variables =====
int score = 0;
bool gameOver = true;
int dinoPosition = 0; // 0 = ground, 1 = sky
int dinoLeg = 0; // 0 = left, 1 = right
char game[32] = {32}; // 32 = air(ascii ' '), 0~1 = dino, 2~4 = cacti, 5 = bird
int obstacle = 0; // obstacle counter
int gap = 0;  // gap counter
const int minGapBetween = 6;  // min gap between obstacles
const int maxConsecObstacles = 3;  // max consecutive obstacles
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 100; // update interval in milliseconds
bool genNextConsec = false;  // determine if next consecutive obstacle should be generated

bool pressed() {
  // check if the button is pressed
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && lastButtonState == LOW) {
    playJumpSound();
    lastButtonState = buttonState;
    return true;
  }
  lastButtonState = buttonState;
  return false;
}


void scroll() {
  // check collision
  if (dinoPosition == 0 && game[17] != 32) {
    // die on ground
    gameOver = true;
    lcd.setCursor(0, 0);
    lcd.print("GAME OVER");
    playDeadSound();
    return;
  }
  if (dinoPosition == 1 && game[1] != 32)  {
    // die in air
    gameOver = true;
    lcd.setCursor(0, 1);
    lcd.print("GAME OVER");
    playDeadSound();
    return;
  }
  
  // next obstacle
  if (genNextConsec) {
    if (++obstacle <= maxConsecObstacles) {
      genNextConsec = (random(obstacle + 3) == 0);
      // generate a cacti (2,3,4)
      game[15] = 32;
      game[31] = (int)random(2) + 2;
      gap = 0;
      obstacle++;
    } else {
      genNextConsec = false;
      // generate a gap
      game[15] = 32;
      game[31] = 32;
      gap++;
      obstacle = 0;
    }

  } else if (random(3) == 0 && gap >= minGapBetween) {
    gap = 0;
    obstacle++;
    if (random(4) == 0) {
      // 1 in 4 generate bird (5)
      game[15] = 5;
      game[31] = 32;
      genNextConsec = false;
    } else {
      // 3 in 4 generate cacti (2,3,4)
      game[15] = 32;
      game[31] = (int)random(2) + 2;
      genNextConsec = (random(obstacle + 3) == 0); // 1 in 3 generate a consecutive cacti
    }
  
  } else {
  // generate a gap
    game[15] = 32;
    game[31] = 32;
    gap++;
    obstacle = 0;
    genNextConsec = false;
  }

  // scroll left
  for (int i=0; i<15; i++)  game[i] = game[i+1];
  for (int i=16; i<31; i++) game[i] = game[i+1];

  // how many digits are there in score 
  int scoreArea;
  if (score >= 1000)     scoreArea = 13;
  else if (score >= 100) scoreArea = 14;
  else                   scoreArea = 15;
  
  // update display
  int temp1 = game[1];
  int temp2 = game[17];
  if (dinoPosition == 1) game[1] = dinoLeg;
  if (dinoPosition == 0) game[17] = dinoLeg;
  lcd.setCursor(0,0);
  for (int i=0; i<scoreArea; i++) {
    lcd.write(byte(game[i]));
  }
  lcd.setCursor(0,1);
  for (int i=16; i<32; i++) {
    lcd.write(byte(game[i]));
  }
  game[1] = temp1;
  game[17] = temp2;
  lcd.setCursor(scoreArea, 0);
  int scoreDisplay = score / 10;
  lcd.print(scoreDisplay);

  // update score
  score++;
  if (score % 100 == 0 && score != 0) playLevelUpSound();
  if (score >= 9999) score = 0; // i don't think anyone can get there

  // Switch Leg
  dinoLeg = 1 - dinoLeg;
}


void setup() {
  lcd.init();
  lcd.backlight();
  pinMode(buttonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  lcd.createChar(0, dino_1);
  lcd.createChar(1, dino_2);
  lcd.createChar(2, cacti_1);
  lcd.createChar(3, cacti_2);
  lcd.createChar(4, cacti_3);
  lcd.createChar(5, bird);
  // welcome Screen
  lcd.setCursor(0, 0);
  lcd.print(" Press To Start ");
  lcd.setCursor(1, 1);
  lcd.write(byte(0));
}


void loop() {
  // ready
  while(!pressed());
  
  // game start
  score = 0;
  gameOver = false;
  dinoPosition = 0;
  lcd.setCursor(0, 0);
  lcd.print("                "); // clear up title after start
  for (int i=0; i<32; i++) game[i] = 32; // reset game map
  
  // game loop
  while(!gameOver) {
    unsigned long currentTime = millis();
    
    if (pressed() && dinoPosition == 0) {
      // jump
      dinoPosition = 1;
      lcd.setCursor(1, 0);
      lcd.write(byte(dinoLeg));
      lcd.setCursor(1, 1);
      lcd.write(byte(32));
    }
    
    if (currentTime - lastUpdateTime >= updateInterval) {
      lastUpdateTime = currentTime;
      
      if (dinoPosition == 1) {
        // count jumps in the air
        static int jump = 0;
        if (++jump >= 5) {
          // land
          jump = 0;
          dinoPosition = 0;
          lcd.setCursor(1, 0);
          lcd.write(byte(32));
          lcd.setCursor(1, 1);
          lcd.write(byte(dinoLeg));
        }
      }
      
      scroll();
    }
  }
  
  // You died
  while(gameOver && !pressed());
  lcd.setCursor(0, 0);
  lcd.print(" Press To Start ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(1, 1);
  lcd.write(byte(0));
}
