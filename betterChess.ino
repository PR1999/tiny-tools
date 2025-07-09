#include <LiquidCrystal.h>
#define PLAYER1 "Player 1"
#define PLAYER2 "Player 2"

char *gamemodes[] = {"Normal", "Chaos", "Chaos+"};
char *timeOpts[] = {"00:05","00:10","00:15","00:20","00:25","00:30"};
const int timerInterval = 1000; //1s 

struct button {
  int pin;
  int state;
  int lastState;
};

struct button button1 = {6,0,0};
struct button button2 = {7,0,0};

struct player {
  char displayName[10];
  long timer;
  struct button *playerBtn;
};

struct game {
  int gamemode;
  int timeOpts;
  struct player p1;
  struct player p2;
  int activeplayer;
};

int rs = 0; //lcd register select pin
int e = 1; //lcd enable pin

LiquidCrystal lcd(0,1,2,3,4,5);

int buttonStateChange(struct button *btn, int debounce) {
  btn->state = digitalRead(btn->pin);
  int stateChange = 0;
  if (btn->state != btn->lastState) {
    if (btn->state == LOW) {
      stateChange = 1; //button is pressed
    } else {
      stateChange = 2; //button is released
    }
    delay(debounce);
  }
  btn->lastState = btn->state;
  return stateChange;
}

int promptConfirmButtons(char msg[], struct button *btnYes, struct button *btnNo) {
  int promptActive = 1;
  int confirm;
  lcd.clear();
  lcd.print(msg); //TODO check msg length
  delay(500); //give some time to read message and release button
  while (promptActive == 1) {
    if (buttonStateChange(btnYes, 100) == 1) {
      promptActive = 0;
      confirm = 1;
    }
    if (buttonStateChange(btnNo, 100) == 1) {
      promptActive = 0;
      confirm = 0;
    }
  }
  lcd.clear();
  return confirm;
}

void displayModePrompt(char msg[], char *modes[], int i) {
    //this could be better with sprintf or something 
    lcd.clear();
    lcd.print(msg);
    lcd.setCursor(0,1);
    lcd.print("<");
    lcd.setCursor(1,1);
    lcd.print(modes[i]);
    lcd.setCursor(strlen(modes[i]) + 1,1);
    lcd.print(">");
    lcd.setCursor(13,1);
    lcd.print("OK?");
}

int promptModeSelect(char msg[], char *modes[], int totalModes, struct button *btnMode, struct button *btnOk) {
  int promptActive = 1;
  int displayedMode = 0;
  displayModePrompt(msg, modes, displayedMode);
  while (promptActive==1) {
    if (buttonStateChange(btnMode, 50) == 1) {
      displayedMode = displayedMode + 1;
      if (displayedMode >= totalModes) {
        displayedMode = 0;
      }
      displayModePrompt(msg, modes, displayedMode);
    }
    if (buttonStateChange(btnOk, 50) == 1) {
      promptActive = 0;
    }
  }
  lcd.clear();
  return displayedMode;
}

void idleMenu() {
  int isIdle = 1;
  lcd.print("Press any button");
  while (isIdle == 1) {
    if (buttonStateChange(&button1, 50) == 1) {
      isIdle = 0;
    }
     if (buttonStateChange(&button2, 50) == 1) {
      isIdle = 0;
    }
  }
  lcd.clear();
}

long timerVal(int selectedTime) {
  //this feels stupid and doesnt need to be a function. 
  //I guess i had a plan and settled for this because I dont want to deal with it now. priorities. 
  long timeMiliseconds;
  switch (selectedTime) {
    case 0 : {
      timeMiliseconds = (5*60*1000);
      break;
    }
    case 1 : {
      timeMiliseconds = (10*60*1000);
      break;
    }
    case 2 : {
      timeMiliseconds = (15*60*1000);
      break;
    }
    case 3 : {
      timeMiliseconds = (20*60*1000);
      break;
    }
    case 4 : {
      timeMiliseconds = (25*60*1000);
      break;
    }
    case 5 : {
      timeMiliseconds = (30*60*1000);
      break;
    }
    default : {
      timeMiliseconds = (15*60*1000);
      Serial.println("WARNING: No valid time - set to default of 15minutes");
      Serial.println(selectedTime);
      break;
    }
  }
  return timeMiliseconds;
}

void timerStrFormat(char *tmr, long timerVal) {
  if (timerVal < 0) {
    timerVal = 0;
  }
  long seconds = timerVal / 1000;
  int minutes = seconds / 60;
  int remainingSeconds = seconds % 60;
  sprintf(tmr, "%2.2i:%2.2i",minutes,remainingSeconds);
}

void setupGame(struct game *game) {
  long timeMiliseconds = timerVal(game->timeOpts);
  game->p1.timer = timeMiliseconds;
  game->p2.timer = timeMiliseconds;
  Serial.printf("Setup: Timer set to %il ms\n", timeMiliseconds);
}

void playGame(struct game *game) {
  //if the game is not finished, start a new turn for the active player. 
  Serial.println("Starting Game!");
  int gameEnded = 0;
  int winner;
  while (gameEnded == 0) {
    if (game->activeplayer == 1) {
      gameEnded = turn(&game->p1);
      game->activeplayer = 2;
    } else if (game->activeplayer == 2) {
      gameEnded = turn(&game->p2);
      game->activeplayer = 1;
    }
  }
  winner = gameEnded == 1 ? game->activeplayer : (game->activeplayer % 2) + 1;
  struct player winningPlayer = winner == 1 ? game->p1 : game->p2;
  char promptWin[16];
  sprintf(promptWin, "%s Won!", winningPlayer.displayName);
  promptConfirmButtons(promptWin, &button1, &button2);
}

int turn(struct player *activePlayer) {
  lcd.clear();
  lcd.write(activePlayer->displayName);
  lcd.setCursor(0,1);
  int turnActive = 1;
  int resultCode = 0;
  unsigned long startTime, currentTime, timeDelta;
  int updateDelta;
  char timerStr[6];
  timerStrFormat(timerStr, activePlayer->timer);
  lcd.print(timerStr);
  startTime = millis();
  while (turnActive == 1) {
    currentTime = millis();
    timeDelta = currentTime - startTime;
    startTime = currentTime; 
    activePlayer->timer = activePlayer->timer - timeDelta;
    updateDelta += timeDelta;
    if (updateDelta > 1000) {
      updateDelta = 0;
      timerStrFormat(timerStr, activePlayer->timer);
      lcd.setCursor(0,1);
      lcd.print(timerStr);
    }
    if (activePlayer->timer <= 0) {
      turnActive = 0;
      resultCode = 1;
      break;
    }
    if (buttonStateChange(activePlayer->playerBtn, 50)==1) {
      turnActive = 0;
      resultCode = 0;
      Serial.printf("End Turn: Player: %s %i remaining\t%s\n", activePlayer->displayName, activePlayer->timer, timerStr);
    }
  }
  lcd.clear();
  return resultCode; //return 0 to continue game; 1 to indicate player lost during turn (time, surrender?), 2 to indicate player won?
}

void setup() {
  lcd.begin(16,2);
  pinMode(button1.pin, INPUT_PULLUP);
  pinMode(button2.pin, INPUT_PULLUP);
}

void loop() {
  idleMenu();
  char prompt[] = "Select Mode:";
  int selectedMode = promptModeSelect(prompt, gamemodes, sizeof(gamemodes) / sizeof(gamemodes[0]), &button2, &button1);
  Serial.printf("Selected: %s\n", gamemodes[selectedMode]);
  char timePrompt[] = "Set Time:";
  int selectedTime = promptModeSelect(timePrompt, timeOpts, sizeof(timeOpts) / sizeof(timeOpts[0]), &button2, &button1);
  Serial.printf("Selected: %s\n", timeOpts[selectedTime]);
  char p1Prompt[] = "Set 1st Player";
  int p1PromptAnswer = promptConfirmButtons(p1Prompt, &button1, &button2); //button 1 returns Yes=1, button 2 returns No=0
  struct player p1,p2;
  if (p1PromptAnswer == 1) {
    p1 = {PLAYER1, 0, &button1};
    p2 = {PLAYER2, 0, &button2};
  } else {
    p1 = {PLAYER1, 0, &button2};
    p2 = {PLAYER2, 0, &button1};
  }
  char readyPrompt[] = "Ready to Start";
  int readyPromptAnswer = promptConfirmButtons(readyPrompt, p1.playerBtn, p2.playerBtn);
  struct game game = {selectedMode, selectedTime, p1, p2, 1};
  setupGame(&game);
  playGame(&game); //future: maybe have this return the winner?
}
