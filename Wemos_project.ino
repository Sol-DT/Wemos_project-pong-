
#include <Streaming.h>
// Expasion board library 
#include <TM1638plus.h> 
// OLED Libraries 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Definitions
// Controls
#define potPin A0
// OLED setup
#define OLED_RESET -1
#define OLED_SCREEN_I2C_ADDRESS 0x3C
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);
// Expansion board setup
#define STROBE_TM D5 
#define CLOCK_TM D6 
#define DIO_TM D7 
bool high_freq = false; 
TM1638plus tm(STROBE_TM, CLOCK_TM , DIO_TM, high_freq);
// Buzzer
#define BUZZER_PIN D8

// Variable definition
// Ball
int x, y, radius = 3;
int xSpeed, ySpeed;

// Players
float userX = 0, userY = 30;
float comX = 126, comY = 30;
float userSpeed = 1.23, comSpeed = 1.23;

int rallyCount = 0, maxRally = 10; // Add difficulty to the game.

// GUI
int border = 9;
int maxHeight = 63, maxWidth = 127; // screen size.
int padHeight = 15, padWidth = 2;// Size of the paddles.
int gameHeight = maxHeight - border, gameWidth = maxWidth - padWidth;
int userScore = 0, comScore = 0, maxScore = 7; // Keep track of player scores.
int wincount = 0;
String displayScore = "";
byte buttons; // variable for using the expansion board buttons.

bool isPaused = false; // Variable to track whether the game is paused.
bool Quit = false; // Variable to track whether the player has pressed the quit button.


void setup() {
  // Setup code 
  pinMode(potPin, INPUT); // Setting the potentiometer as an input.
  pinMode(BUZZER_PIN, OUTPUT); // Setting the buzzer as an output.
  Serial.begin(115200); // Beginning serial communication.
  Serial.println("Hello World"); // For debugging.
  delay(1000);  

  // OLED setup code.
  display.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_I2C_ADDRESS);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1); 
  display.setTextColor(WHITE);
  Serial.println("Configuring OLED"); // For debugging.
  delay(1000);  

  tm.displayBegin(); // Turning on the expansion board

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  // Draws a simple logo animation.
  for (int i = 0; i < 3; ++i) {
    display.clearDisplay();
    display.setCursor(20 * i, 20);
    display.print("PONG");
    display.display();
    delay(500);
  }
  delay(1000); // Add a pause after the animation.
  // Clear the display for the main game screen.
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1); 
  display.setTextColor(WHITE);
  display.display();

  // Initialize ball position and speed.
  x = random(50, 70);
  y = random(border + radius + 5, 63 - 5);
  xSpeed = 1;
  ySpeed = 1;
  Serial.println("Setup complete"); // For debugging 
  delay(1000);  
}

void loop() {
  // Main code 

  // Quit button
  if (tm.readButtons()) {
    buttons = tm.readButtons();
    if (buttons & 0b10000000) { // Check if the 8th button is pressed to quit the game.
      Quit = true; // Set quit to true.
      tm.reset();
      delay(500); // Add a delay to avoid rapid button presses.
    }
  }
  // Checks if the game should quit, then exits the loop if true.
  if (Quit) {
    display.clearDisplay();
    display.setCursor(30, 30);
    display.print("Exiting game.");
    display.display();
    Serial.print("Wemos going dark. ");
    delay(3000);
    display.ssd1306_command(SSD1306_DISPLAYOFF); // Turns off the OLED.
    return;
  } else {

    // Pause button
    if (tm.readButtons()) {
      buttons = tm.readButtons();
      if (buttons & 0b00000010) { // Check if the second button is pressed to pause the game.
        isPaused = !isPaused; // change pause state.
        delay(500); 
      }
    }

    // If the game is paused, skip the game logic and display a pause menu.
    if (!isPaused) {

      // Reset button
      if (tm.readButtons()) {
        buttons = tm.readButtons();
        if (buttons & 0b00000001) { // Check if the first button is pressed
          display.clearDisplay();
          display.setCursor(30, 30);
          display.print("Resetting game.");
          display.display();
          delay(3000);
          x = random(50, 70);
          y = random(border + radius + 5, maxHeight - 5);
          userScore = 0;
          comScore = 0;
          wincount = 0;
        }
      }

      // Move
      // Change direction if the ball reaches a boundary.
      if (x + radius > gameWidth || x - radius < padWidth) {
        xSpeed = -xSpeed;
      }
      if (y + radius >= maxHeight || y - radius <= border) {
        ySpeed = -ySpeed;
      }
      // Move the ball to its new position.
      x += xSpeed;
      y += ySpeed;

      // Move the user's paddle using the potentiometer value.
      int potValue = analogRead(potPin);
      Serial.print("Potentiometer Value: ");
      Serial.println(potValue);

      userY = map(potValue, 0, 1023, border, gameHeight);
      Serial.print("Mapped userY: "); // For debugging 
      Serial.println(userY); 

      // Move the computer's paddle.
      if (xSpeed) {
        if (ySpeed > 0) {
          comY += comSpeed; // If the ball is going down, the paddle moves down.
        } else {
          comY -= comSpeed; // and vice versa.
        }

        if (comY >= gameHeight){
          comY = gameHeight; // Stops the paddle from exiting the play area.
        }
        if (comY <= border){
          comY = border;
        }
      }

      // Check if computer has scored and adds to the computer's score.
      if ((x - radius) <= (userX + 1)) {
        if (!(y + radius >= userY && y - radius <= userY + padHeight)) {
          // Computer (com) scores a point.
          comScore++;
          delay(3000);
          // Reset the ball position and speed.
          xSpeed = 1;
          ySpeed = 1;
          rallyCount = 0;
          x = random(50, 70);
          y = random(border + radius + 5, 63 - 5);
        } else {
          // The player successfully hit the ball, increment rally count.
          rallyCount++;
        }

        // Check if rally count exceeds the maximum allowed.
        if (rallyCount > maxRally) {
          rallyCount = 0;
          // Increase the speed of the ball in both X and Y directions.
          xSpeed = xSpeed > 0 ? xSpeed + 1 : xSpeed - 1;
          ySpeed = ySpeed > 0 ? ySpeed + 1 : ySpeed - 1;
        }

        // Limit the maximum speed of the ball.
        if (xSpeed > 2 || ySpeed > 2) {
          xSpeed = 2;
          ySpeed = 2;
        } else if (xSpeed < -2 || ySpeed < -2) {
          xSpeed = -2;
          ySpeed = -2;
        }
      }

      // Check if user has scored.
      if ((comX - 1) <= (x + radius)) {
        if (!(y + radius >= comY && y - radius <= comY + padHeight)) {
          userScore++;
          delay(3000);
          xSpeed = 1;
          ySpeed = 1;
          rallyCount = 0;
          x = random(50, 70);
          y = random(border + radius + 5, maxHeight - 5);
        }
      }
      // Move end

      // Update score
      displayScore = "Home u: " + String(userScore) + " Away c: " + String(comScore);

      // Display
      display.clearDisplay();
      Serial.println("Drawing display"); // For debugging 
      // Draw ball.
      display.drawCircle(x, y, radius, SSD1306_WHITE);
      // Draw score.
      display.setCursor(0, 0);  display.print(displayScore);
      // Draw playing field.
      display.drawFastHLine(0, border, 127, SSD1306_WHITE);
      // Draw paddles.
      display.fillRect(userX, static_cast<int>(userY), padWidth, padHeight, SSD1306_WHITE);
      display.fillRect(comX, static_cast<int>(comY), padWidth, padHeight, SSD1306_WHITE);

      // Update the scoreboard on the TM1638 display.
      Serial.println("Updating TM1638 display");
      // Displays the user win count.
      tm.displayIntNum(wincount, false);
      display.display();

      // Game Over logic.
      if (userScore >= maxScore || comScore >= maxScore) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        const char* msg = (userScore >= maxScore) ? "YOU WON :)" : "YOU LOST :(";
        display.setCursor(15, 5);
        display.print(msg);
        // Activate the buzzer for 1 second.
        tone(BUZZER_PIN, 1000); 
        delay(1000); 
        noTone(BUZZER_PIN); // Turn off the buzzer and delay for 1 second.
        delay(1000);

        // Clear the display and restart the game.
        display.setCursor(30, 30);

        display.display();
        delay(5000);
        x = random(50, 70);
        y = random(border + radius + 5, maxHeight - 5);
        display.print("Game over.");
        if(userScore >= maxScore){
          wincount++;
        }
        userScore = 0;
        comScore = 0;
      }
    } else {
      // Display pause message.
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(15, 5);
      display.print("Game Paused");
      display.display();
    }
  }
}

