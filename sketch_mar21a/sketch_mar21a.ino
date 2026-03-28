#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUTTON_PIN_UP 9
#define BUTTON_PIN_DOWN 8
#define BUTTON_PIN_SELECT 10

#define POTENTIOMETER_PIN A0
#define LIGHT_SENSOR_PIN A1
#define KAYKAY_WIDTH 20
#define KAYKAY_HEIGHT 2
#define TOP_DIAMETER 2 
#define BRICK_WIDTH 10
#define BRICK_HEIGHT 3
#define NUM_BRICKS 15 
#define LIFE_LED_1 13  
#define LIFE_LED_2 12
#define LIFE_LED_3 11 
#define DROP_OBJECT_CHANCE 10 
int playerLives = 2; 
int score = 0;

struct Brick {
    int x;
    int y;
    bool active;
    int hitCount;
    int level;
};

Brick bricks[NUM_BRICKS];
struct Object {
    int x;
    int y;
    bool active;
};

Object dropObject;


int kaykayX = SCREEN_WIDTH / 2 - KAYKAY_WIDTH / 2;
int kaykayY = SCREEN_HEIGHT - KAYKAY_HEIGHT - 2;

float kaykaySpeed = 4;
int topX = SCREEN_WIDTH / 2;
int topY = SCREEN_HEIGHT / 2;
float topSpeedX = 2;
float topSpeedY = 2;
int currentLevel=0;
int NUM_LEVELS=5;
const int LIFE_LED_PIN = 13;


int menuSelection = 0; 
bool gameStarted = false;

void setup() {
    pinMode(LIGHT_SENSOR_PIN, INPUT); 
    pinMode(BUTTON_PIN_UP, INPUT_PULLUP);
    pinMode(BUTTON_PIN_DOWN, INPUT_PULLUP);
    pinMode(BUTTON_PIN_SELECT, INPUT_PULLUP);
    pinMode(LIFE_LED_1, OUTPUT);
    pinMode(LIFE_LED_2, OUTPUT);
    pinMode(LIFE_LED_3, OUTPUT);
    pinMode(2,OUTPUT);
    pinMode(3,OUTPUT);
    pinMode(4,OUTPUT);
    pinMode(5,OUTPUT);
    pinMode(6,OUTPUT);
    pinMode(7,OUTPUT);
    pinMode(1,OUTPUT);
    digitalWrite(LIFE_LED_1, HIGH);
    digitalWrite(LIFE_LED_2, HIGH);
    digitalWrite(LIFE_LED_3, HIGH);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 başlatılamadı. OLED ekran bağlantısını kontrol edin!"));
        for (;;);
    }
    
    display.clearDisplay();
    display.fillScreen(SSD1306_BLACK);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Menu:");
    display.println();
    updateMenu();
    display.display();

    currentLevel = 1;
    initBricks(currentLevel);
    dropObject.active = false;
}

void loop() {
  int lightLevel = analogRead(LIGHT_SENSOR_PIN); 

  if (lightLevel > 15) {
    display.invertDisplay(true); 
  } else {
    display.invertDisplay(false); 
  }

  display.display();
    if (!gameStarted) {
        if (digitalRead(BUTTON_PIN_SELECT) == LOW) {
            if (menuSelection == 0) {
                startGame();
            } else {
                exitGame();
            }
        }

        if (digitalRead(BUTTON_PIN_UP) == LOW) {
            menuSelection = 0;
            updateMenu();
           
        }

        if (digitalRead(BUTTON_PIN_DOWN) == LOW) {
            menuSelection = 1;
            updateMenu();
        
        }
    } else {
        int potValue = analogRead(POTENTIOMETER_PIN);
        moveSkateboard(potValue);

        moveBall();
        checkBrickCollision();
        updateDisplay();
        if (dropObject.active) {
            moveDropObject();
            checkDropObjectCollision();
        }
       
    }
}

void decreaseLife() {
    if (topY >= SCREEN_HEIGHT) {
        playerLives--;

        if (playerLives == 2) {
            digitalWrite(LIFE_LED_3, LOW);
        } else if (playerLives == 1) {
            digitalWrite(LIFE_LED_2, LOW);
        } else if (playerLives == 0) {
            digitalWrite(LIFE_LED_1, LOW);
        }
    }
}

void startGame() {
    gameStarted = true;
    playerLives = 2; 
    score = 0; 
    digitalWrite(LIFE_LED_1, HIGH);
    digitalWrite(LIFE_LED_2, HIGH);
    digitalWrite(LIFE_LED_3, HIGH);
    topX = SCREEN_WIDTH / 2;
    topY = SCREEN_HEIGHT / 2;
    currentLevel = 1;
    initBricks(currentLevel);
    dropObject.active = false;

    display.clearDisplay();
    display.display();
}


void exitGame() {
    gameStarted = false;
    display.clearDisplay();
    display.println("Oyun Sonlandirildi!");
    display.display();

    display.clearDisplay();
    updateMenu();
    display.display();
}


void moveSkateboard(int potValue) {
  
    int mappedValue = map(potValue, 0, 1023, 0, SCREEN_WIDTH - KAYKAY_WIDTH);
    kaykayX = mappedValue;
}

void moveBall() {
  
    topX += topSpeedX;
    topY += topSpeedY;

    if (topX <= 0 || topX >= SCREEN_WIDTH - TOP_DIAMETER) {
        topSpeedX = -topSpeedX; 
    }
       if (topY <= 0) {
        topSpeedY = -topSpeedY; 
    }


    if (topX + TOP_DIAMETER >= kaykayX && topX <= kaykayX + KAYKAY_WIDTH && topY + TOP_DIAMETER >= kaykayY && topY <= kaykayY + KAYKAY_HEIGHT) {

        topSpeedY = -topSpeedY;
    } else if (topY >= SCREEN_HEIGHT) {
        gameStarted = false;
        if(playerLives>0){
          decreaseLife(); 
          gameStarted = true;
          topX =kaykayX+KAYKAY_WIDTH/2;
          topY = kaykayY-KAYKAY_HEIGHT-TOP_DIAMETER;
          
          moveBall();
        }
        else{
          decreaseLife();
        display.clearDisplay();
        display.setCursor(20, 20);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.println("Oyun Bitti! ");
        drawScore();
        display.display();
        delay(2000); 
        updateMenu(); 
        }
    }
}
void moveDropObject() {
    
    dropObject.y += 1;
 
    if (dropObject.y >= SCREEN_HEIGHT) {
        dropObject.active = false; 
    }
    drawObject(dropObject.x, dropObject.y, BRICK_WIDTH, BRICK_HEIGHT);
    display.display(); 
}


void checkDropObjectCollision() {
    
    if (dropObject.x + BRICK_WIDTH >= kaykayX && dropObject.x <= kaykayX + KAYKAY_WIDTH && dropObject.y + BRICK_HEIGHT >= kaykayY && dropObject.y <= kaykayY + KAYKAY_HEIGHT) {
       
        playerLives++; 

        updateDisplay(); 
        dropObject.active = false; 
        if (playerLives == 3) {
            digitalWrite(LIFE_LED_3, HIGH);
        } else if (playerLives == 2) {
            digitalWrite(LIFE_LED_2, HIGH);
        } else if (playerLives == 1) {
            digitalWrite(LIFE_LED_1, HIGH);
        }
    }
}



void drawBricks() {
    for (int i = 0; i < NUM_BRICKS; i++) {
        if (bricks[i].active) {
            display.drawRect(bricks[i].x, bricks[i].y, BRICK_WIDTH, BRICK_HEIGHT, SSD1306_WHITE);
        }
    }
}
void generateDropObject(Brick brick) {

    dropObject.x = brick.x;
    dropObject.y = brick.y;
    dropObject.active = true;
}

void updateMenu() {
    display.clearDisplay();
    display.fillScreen(SSD1306_BLACK);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Menu:");
    display.println();
    if (menuSelection == 0) {
        display.println("> Baslat");
        display.println("  Cikis");
    } else {
        display.println("  Baslat");
        display.println("> Cikis");
    }
    display.display();
}


void updateDisplay() {
    display.clearDisplay();
    drawBricks(); 
    drawSkateboard(); 
    drawBall(); 
    drawScore(); 
    display.display();
}


void initBricks(int level) {
    int brickSpacing = 3;
    int startX = 10;
    int startY = 10;
    
    if (level == 1) {
      startX = 10;
        startY = 10;
        brickSpacing = 3;
        for (int i = 0; i < NUM_BRICKS-6; i++) {
            bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * i;
            bricks[i].y = startY;
            bricks[i].active = true;
            bricks[i].level = level; 
        }
    } else if (level == 2) {
        
        topSpeedX=topSpeedX+topSpeedX*20/100;
        topSpeedY=topSpeedY+topSpeedY*20/100;
        for (int i = 0; i < NUM_BRICKS-3; i++) {
            bricks[i].x = startX ;
            bricks[i].y = startY + (BRICK_WIDTH + brickSpacing) * i;
            if(i>2 && i<6){
              
              bricks[i].x = startX +20;
              bricks[i].y = startY + (BRICK_WIDTH + brickSpacing) * (i-2);
              
            }
            if(i>=6 && i<9){
                
              bricks[i].x = SCREEN_WIDTH-startX-BRICK_WIDTH;
              bricks[i].y = startY + (BRICK_WIDTH + brickSpacing) * (i-6);
              }
            if(i>=9){
              bricks[i].x = SCREEN_WIDTH-startX-BRICK_WIDTH-20;
              bricks[i].y = startY + (BRICK_WIDTH + brickSpacing) * (i-8);
            }
            bricks[i].active = true;
            bricks[i].level = level; 
        }
    }
        else if (level == 3) {
        topSpeedY=topSpeedY+topSpeedY*20/100;
        topSpeedX=topSpeedX+topSpeedX*20/100;
        startX = 10;
        startY = 10;
        brickSpacing = 1;
        for (int i = 0; i < NUM_BRICKS; i++) {
            bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * i;
            bricks[i].y = startY;
             
            if(i>=3 && i<6){
                bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * i+40;
                bricks[i].y = startY;
            }
            if(i>=6 && i<9){
              bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * (i-6);
            bricks[i].y = startY+30;
            }
            if(i>= 9 && i<12){
              bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) *(i-6)+40;
                bricks[i].y = startY+30;
                
            }
            if(i>=12){
              bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * (i-11)+25;
              bricks[i].y = startY+15;
            }
            bricks[i].active = true;
            bricks[i].level = level;
        }
    }
     else if (level == 4) {
        
      topSpeedX=topSpeedX+topSpeedX*20/100;
      topSpeedY=topSpeedY+topSpeedY*20/100;
        startX = 10;
        startY = 10;
        for (int i = 0; i < NUM_BRICKS; i++) {
            bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * i;
            bricks[i].y = startY;
             
            if(i>=3 && i<6){
                bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * i+40;
                bricks[i].y = startY;
            }
            if(i>=6 && i<9){
            bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * (i-6);
            bricks[i].y = startY+15;
            }
            if(i>= 9 && i<12){
              bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) *(i-6)+40;
                bricks[i].y = startY+15;
                
            }
            if(i>=12){
              bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * (i-11)+25;
              bricks[i].y = startY+30;
            }
            bricks[i].active = true;
            bricks[i].level = level;
        }
    }
     else if (level == 5) {

        topSpeedX=topSpeedX+topSpeedX*20/100;
        topSpeedY=topSpeedY+topSpeedY*20/100;
        brickSpacing =7;
        startX = 25;
        startY = 10;
        for (int i = 0; i < NUM_BRICKS; i++) {
            bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * i;
            bricks[i].y = startY;
            if(i>=5 && i<9){
                bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * (i-5)+(BRICK_WIDTH+brickSpacing)/2;
                bricks[i].y = startY+10;
            }
            if(i>=9 && i<12){
            bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * (i-9)+BRICK_WIDTH+brickSpacing;
                bricks[i].y = startY+20;
            }
            if(i>= 12 && i<14){
              bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * (i-12)+(BRICK_WIDTH+brickSpacing)*3/2;
                bricks[i].y = startY+30;
                
            }
            if(i>=14){
              bricks[i].x = startX + (BRICK_WIDTH + brickSpacing) * (i-14)+(BRICK_WIDTH+brickSpacing)*2;
                bricks[i].y = startY+40;
            }
            bricks[i].active = true;
            bricks[i].level = level; 
        }
    }
}

void checkBrickCollision() {
    int totalActiveBricks = 0; 

    for (int i = 0; i < NUM_BRICKS; i++) {
        if (bricks[i].active) {
            totalActiveBricks++;
            if (topX + TOP_DIAMETER >= bricks[i].x && topX <= bricks[i].x + BRICK_WIDTH && topY + TOP_DIAMETER >= bricks[i].y && topY <= bricks[i].y + BRICK_HEIGHT) {
                bricks[i].hitCount++;
                display.display();
                if (playerLives < 2) {
                    if (!dropObject.active && random(100) < DROP_OBJECT_CHANCE) {
            generateDropObject(bricks[i]); 
            
        }
        
                }
                
                if (bricks[i].hitCount >=1) {
                    bricks[i].active = false;
                    score++; 
                    if(score%10==0)
                      zero();
                    else if(score%10 ==1)
                      one();
                    else if(score%10 ==2)
                      two();
                    else if(score%10 ==3)
                      three();
                    else if(score%10 ==4)
                      four();
                    else if(score%10 ==5)
                      five();
                    else if(score%10 ==6)
                      six();
                    else if(score%10 ==7)
                      seven();
                    else if(score%10 ==8)
                      eight();
                    else if(score%10 ==9)
                      nine();
                }
            if(topX>= bricks[i].x + BRICK_WIDTH && bricks[i].x <topY){
              topSpeedX = -topSpeedX;
              topSpeedY = -topSpeedY; 
            }
            
             if(topX <=bricks[i].x ){
              topSpeedX = -topSpeedX;
              topSpeedY = -topSpeedY;
            }
            
            topSpeedY = -topSpeedY;
                    
            }
        }
    }


    if (topY >= SCREEN_HEIGHT) {
        playerLives--; 
        if (playerLives == 2) {
            digitalWrite(LIFE_LED_3, LOW);
        } else if (playerLives == 1) {
            digitalWrite(LIFE_LED_2, LOW);
        } else if (playerLives == 0) {
            digitalWrite(LIFE_LED_1, LOW);
        }
    }

    if (totalActiveBricks == 0 && currentLevel < NUM_LEVELS) {
        currentLevel++;
        initBricks(currentLevel); 
        display.clearDisplay();
        display.setCursor(20, 20);
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.println("LEVEL " + String(currentLevel));
        display.display();
        delay(5000); 
        updateDisplay(); 
    }else if (totalActiveBricks == 0 && currentLevel == NUM_LEVELS) {
      drawScore();
    endGame(); 
}
}

void endGame() {
    display.clearDisplay();
    display.setCursor(20, 20);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println("OYUN BITTI!");
    display.display();
    delay(2000); 
    updateMenu(); 
}

void drawSkateboard() {
    display.fillRect(kaykayX, kaykayY, KAYKAY_WIDTH, KAYKAY_HEIGHT, SSD1306_WHITE);
}

void drawBall() {
    display.fillCircle(topX + TOP_DIAMETER / 2, topY + TOP_DIAMETER / 2, TOP_DIAMETER / 2, SSD1306_WHITE);
}
void drawScore() {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Score: "); 
    display.println(score); 
    display.display();
}

void drawObject(int x, int y, int width, int height) {
    display.drawCircle(x, y, 3, SSD1306_WHITE);
    display.display();
}

void zero() {
  digitalWrite(1,HIGH);
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(6,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(7,LOW);
 
}
void one(){
  digitalWrite(1,LOW);
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(4,LOW);
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
  digitalWrite(7,LOW);
  
}

void two() {
  digitalWrite(1,HIGH);
  digitalWrite(2,HIGH);
  digitalWrite(3,LOW);
  digitalWrite(4,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(6,LOW);
  digitalWrite(7,HIGH);

}

 void three() {
  digitalWrite(1,HIGH);
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
  digitalWrite(7,HIGH);
  
 }

void four() {
  digitalWrite(1,LOW);
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(4,LOW);
  digitalWrite(5,LOW);
  digitalWrite(6,HIGH);
  digitalWrite(7,HIGH);
  
}

void five() {
  digitalWrite(1,HIGH);
  digitalWrite(2,LOW);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(5,LOW);
  digitalWrite(6,HIGH);
  digitalWrite(7,HIGH);
  
}

void six() {
  digitalWrite(1,HIGH);
  digitalWrite(2,LOW);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(6,HIGH);
  digitalWrite(7,HIGH);

}

void seven() {
  digitalWrite(1,HIGH);
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(4,LOW);
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
  digitalWrite(7,LOW);

  }

void eight() {
  digitalWrite(1,HIGH);
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(5,HIGH);
  digitalWrite(6,HIGH);
  digitalWrite(7,HIGH);
  
}

void nine() {
  digitalWrite(1,HIGH);
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(5,LOW);
  digitalWrite(6,HIGH);
  digitalWrite(7,HIGH);

}