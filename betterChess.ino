#include <LiquidCrystal.h>

char *gamemodes[] = {"Normal", "Chaos", "Chaos+"};
char *timeOpts[] = {"00:05","00:10","00:15","00:20","00:25","00:30"};

int rs = 0;
int e = 1;

struct button {
  int pin;
  int state;
  int lastState;
};

struct button button1 = {6,0,0};
struct button button2 = {7,0,0};


unsigned long prevMilis = 0;
unsigned long currentMilis = 0;
const int timerInterval = 1000; //1s 

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
    if (int checkYes = buttonStateChange(btnYes, 100) == 1) {
      promptActive = 0;
      confirm = 1;
    }
    if (int checkNo = buttonStateChange(btnNo, 100) == 1) {
      promptActive = 0;
      confirm = 0;
    }

  }
  lcd.clear();

  return confirm;
}

void displayModePrompt(char msg[], char *modes[], int i) {
    lcd.clear();
    lcd.print(msg);
    lcd.setCursor(0,2);
    lcd.print("<");
    lcd.setCursor(1,2);
    lcd.print(modes[i]);
    lcd.setCursor(strlen(modes[i]) + 1,2);
    lcd.print(">");
    lcd.setCursor(13,2);
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
      Serial.println(totalModes);
      Serial.println(displayedMode);
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

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16,2);
  pinMode(button1.pin, INPUT_PULLUP);
  pinMode(button2.pin, INPUT_PULLUP);

}

void loop() {

  idleMenu();
  char prompt[] = "Select Mode:";
  int selectedMode = promptModeSelect(prompt, gamemodes, sizeof(gamemodes) / 4, &button2, &button1);
  Serial.print("Selected: ");
  Serial.print(gamemodes[selectedMode]);
  Serial.print("\n");
  char timePrompt[] = "Set Time:";
  int selectedTime = promptModeSelect(timePrompt, timeOpts, sizeof(timeOpts) / 4, &button2, &button1);
  Serial.print("Selected: ");
  Serial.print(timeOpts[selectedTime]);
  Serial.print("\n");
  
  

}
