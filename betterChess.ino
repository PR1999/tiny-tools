#include <LiquidCrystal.h>
#define PLAYER1 "Player 1"
#define PLAYER2 "Player 2"
#define NO_CHAOS 0.0
#define CHAOS_CHANCE 0.2
#define CHAOS_PLUS_CHANCE 0.4
#define GAMEMODE_1 "Normal"
#define GAMEMODE_2 "Chaos"
#define GAMEMODE_3 "Chaos+"
#define PAUSE_CONTINUE "Continue"
#define PAUSE_LOSE "Surrender"
#define PAUSE_WIN "Checkmate"
#define KING "KING"
#define QUEEN "QUEEN"
#define KNIGHT "KNIGHT"
#define BISHOP "BISHOP"
#define ROOK "ROOK"
#define PAWN "PAWN"
#define BLACK "BLACK"
#define WHITE "WHITE"
#define MAX_LENGTH_CHAOS_STR 50

char *gamemodes[] = {GAMEMODE_1, GAMEMODE_2, GAMEMODE_3};
const float chaosChance[] = {NO_CHAOS, CHAOS_CHANCE, CHAOS_PLUS_CHANCE}; 
char *timeOpts[] = {"00:05","00:10","00:15","00:20","00:25","00:30"};
char *pauseModes[] = {PAUSE_CONTINUE, PAUSE_LOSE, PAUSE_WIN};
char *pieces[] = {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN};
char *colors[] = {BLACK, WHITE};
char *sides[] = {"LEFT", "RIGHT"};
const int timerInterval = 1000; //1s

struct chaos_args {
  int id;
  int argN;
  int optCount;
  char **chaosOpts[];
};

struct chaos {
  int id;
  int argCount;
  struct chaos_args *arglist1;
  struct chaos_args *arglist2;
  char formatStr[];
};


struct chaos_args chaos_1_1 = {1,1,2,{&pieces[4], &pieces[5]}};
struct chaos chaos_1 = {1,1, &chaos_1_1, NULL, "All Pieces Move Like a %s"};

struct chaos_args chaos_2_1 = {2,1,4,{&pieces[2], &pieces[3],&pieces[4], &pieces[5]}};
struct chaos_args chaos_2_2 = {2,2,4,{&pieces[2], &pieces[3],&pieces[4], &pieces[5]}};
struct chaos chaos_2 = {2,2,&chaos_2_1,&chaos_2_2, "Swap a %s with a %s"};

struct chaos chaos_3 = {3,0,NULL, NULL, "Trade a pawn with the other Player"};

struct chaos_args chaos_4_1 = {4,1,2,sides};
struct chaos chaos_4 = {4, 1, &chaos_4_1, NULL, "You can only move on the %s side"};

struct chaos chaos_5 = {5,0,NULL, NULL, "You can only move backwards"};

//struct chaos chaos_6 = {6,2, "a Piece on %s has Teleported to %s"};
//struct chaos chaos_7 = {7,0, "You get an extra move! Lucky!"};
//struct chaos chaos_8 = {8,0, "Pawns can only move backwards"};
//struct chaos chaos_9 = {9,1, "You can only move from %s Squares"};
//struct chaos chaos_10 = {10,1, "%s is out of order. Do not use it."};
//struct chaos chaos_11 = {11,2, "Your %s and %s are in love. Place them in the same square"};

struct chaos *chaosOptions[] = {&chaos_1,&chaos_2, &chaos_3, &chaos_4, &chaos_5};

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

char** splitStr(char msg[], int lineLen) {
  //split message into parts (!! allocs memory but doesn't free it!!)
  int msgLen = strlen(msg);
  int lines = (msgLen / lineLen) + 1;
  int i, pos;
  char **splitLines = NULL;
  splitLines = (char**)malloc(lines  * sizeof(char*));
  for (i = 0; i < lines; i++) {
    splitLines[i] = (char*)malloc(((lineLen+1) * sizeof(char)));
    char line[(lineLen + 1)];
    pos = i*lineLen;
    strncpy(line, msg + pos, lineLen);
    line[(lineLen)] = '\0';
    strncpy(splitLines[i], line, (lineLen + 1));
  }
  return splitLines;
  //todo use buttons to cycle through lines, and to ack when on last line. and free splitlines. 
}

int promptConfirmButtons(char msg[], struct button *btnYes, struct button *btnNo) {
  int promptActive = 1;
  int confirm;
  unsigned long startTime, currentTime, timeDelta;
  int updateDelta;
  bool multiLine;
  int page, lineCount, currentLine;
  char **lines;
  int maxLen = 14;
  int updateInterval = 1000;
  lcd.clear();
  lcd.print(msg);  //TODO check msg length
  delay(500);      //give some time to read message and release button
  multiLine = strlen(msg) > maxLen;
  if (multiLine) {
    lines = splitStr(msg, maxLen);
    lineCount = (strlen(msg) / maxLen) + 1;
    Serial.printf("%i Lines\n", lineCount);
  }
  startTime = millis();
  while (promptActive == 1) {
    if (multiLine) {
      currentTime = millis();
      timeDelta = currentTime - startTime;
      startTime = currentTime;
      updateDelta += timeDelta;
      if ((updateDelta > updateInterval) && multiLine) {
        if (currentLine >= lineCount) {
          currentLine = 0;
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(lines[currentLine]);
        currentLine++;
        updateDelta = 0;
      }
    }
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
  if (multiLine) {
    for (int i = 0; i < lineCount; i++) {
      free(lines[i]);
    }
    free(lines);
  }
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

void getChaosMsg(char *msg) {
  int chaosId = random(1, sizeof(chaosOptions)/sizeof(chaosOptions[0]));
  Serial.printf("Chaos ID: %i\n", chaosId);
  struct chaos *current_chaos = chaosOptions[chaosId - 1];
  int index1,index2;
  switch (current_chaos->argCount) {
    case 2: {
      index1 = random(0, (current_chaos->arglist1->optCount - 1));
      index2 = random(0, (current_chaos->arglist2->optCount - 1));
      char *arg1 = *current_chaos->arglist1->chaosOpts[index1];
      char *arg2 = *current_chaos->arglist2->chaosOpts[index2];
      sprintf(msg,current_chaos->formatStr,arg1, arg2);
      Serial.printf("getChaosMsg: fstr: %s\targ1: %i %s\targ2: %i %s\tmsg: %s\n",current_chaos->formatStr, index1, arg1,index2,arg2,msg);
      break; 
    }
    case 1: {
      index1 = random(0, (current_chaos->arglist1->optCount - 1));
      char *arg1 = *current_chaos->arglist1->chaosOpts[index1];
      sprintf(msg,current_chaos->formatStr,arg1);
      break;
    }
    case 0: {
      sprintf(msg,current_chaos->formatStr);
      break;
    }
  }
}

void setupGame(struct game *game) {
  long timeMiliseconds = timerVal(game->timeOpts);
  game->p1.timer = timeMiliseconds;
  game->p2.timer = timeMiliseconds;
  Serial.printf("Setup: Timer set to %il ms\n", timeMiliseconds);
  //randomSeed();
}

void playGame(struct game *game) {
  //if the game is not finished, start a new turn for the active player. 
  Serial.println("Starting Game!");
  int gameEnded = 0;
  int winner;
  while (gameEnded == 0) {
    if (random(0,100) < (chaosChance[game->gamemode]*100)) {
      Serial.println("Chaos Turn!");
      lcd.clear();
      char chaosMsg[100];
      getChaosMsg(chaosMsg);

      int ackChaos = 0;
      while (ackChaos != 1) {
        ackChaos = promptConfirmButtons(chaosMsg, game->activeplayer == 1 ? game->p1.playerBtn : game->p2.playerBtn, game->activeplayer == 1 ? game->p2.playerBtn : game->p1.playerBtn);
      }
    } 
    if (game->activeplayer == 1) {
      gameEnded = turn(&game->p1, game->p2.playerBtn);
      game->activeplayer = 2;
    } else if (game->activeplayer == 2) {
      gameEnded = turn(&game->p2, game->p1.playerBtn);
      game->activeplayer = 1;
    }
  }
  winner = gameEnded == 1 ? game->activeplayer : (game->activeplayer % 2) + 1;
  struct player winningPlayer = winner == 1 ? game->p1 : game->p2;
  char promptWin[16];
  sprintf(promptWin, "%s Won!", winningPlayer.displayName);
  promptConfirmButtons(promptWin, &button1, &button2);
}

int turn(struct player *activePlayer, struct button *pauseBtn) {
  lcd.clear();
  lcd.write(activePlayer->displayName);
  lcd.setCursor(0,1);
  int turnActive = 1;
  int resultCode = 0;
  char pauseText[] = "Game Paused";
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
    if (buttonStateChange(pauseBtn, 50)==1) {
      Serial.printf("Game Paused: %s %s", activePlayer->displayName, timerStr);
      int pauseReturn = promptModeSelect(pauseText, pauseModes, sizeof(pauseModes)/sizeof(pauseModes[0]),&button2, &button1);
      if (pauseReturn == 0) {
        lcd.clear();
        lcd.home();
        lcd.write(activePlayer->displayName);
        lcd.setCursor(0,1);
        startTime = millis();
      } else if (pauseReturn == 1 ) {
        //pauseModes[1] is pause_lose, so return 1 for player lost and end turn
        resultCode = 1;
        turnActive = 0;
        break;
      } else if (pauseReturn == 2) {
        //pauseModes[2] is pause_win, so return 2 for player win and end turn
        resultCode = 2;
        turnActive = 0;
        break;
      }
    }
  }
  lcd.clear();
  return resultCode; //return 0 to continue game; 1 to indicate player lost during turn, 2 to indicate player won
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
  playGame(&game);
}
