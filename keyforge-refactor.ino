#include <Adafruit_RGBLCDShield.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// TODO - promtp users for two letter names at start of game..?
// 1 - render players
// 2 - change views...?
// players
// chains prompt
// draw msg
// forge prompt
// players ...

void setup() {
  lcd.begin(16, 2);
  lcd.cursor();
  Serial.begin(9600);
}

class GameRules {
  public:

  const int minHand = 0;
  const int maxHand = 36;
  const int minKeys = 0;
  const int maxKeys = 3;
  const int minChains = 0;
  const int maxChains = 24;
  const int minAember = 0;
  const int maxAember = 99;
  const int minForgeMod = -99;
  const int maxForgeMod = 99;
  const int baseHandSize = 6;
  const int baseKeyCost = 6;

  int calcPenalty(int chains) {
    if (0 < chains && chains < 7) return -1;
    if (6 < chains && chains < 13) return -2;
    if (12 < chains && chains < 19) return -3;
    if (18 < chains) return -4;
    return 0;
  }
};

class CommonLogic {
  public:

  int calcChange(int currentValue, int delta, int maximum, int minimum) {
    int total = currentValue + delta;
    if (total < minimum) return minimum;
    if (total > maximum) return maximum;
    return total;
  }
};

const GameRules rules;
const CommonLogic commons;

class PlayerState {
  public: 

  String name;
  int aember = 0;
  int chains = 0;
  int keys = 0;

  PlayerState(String initName) {
    name = initName;
  }

  void changeAember(int delta) {
    aember = commons.calcChange(aember, delta, rules.maxAember, rules.minAember);
  }

  void changeKeys(int delta) {
    keys = commons.calcChange(keys, delta, rules.maxKeys, rules.minKeys);
  }

  void changeChains(int delta) {
    chains = commons.calcChange(chains, delta, rules.maxChains, rules.minChains);
  }
};

PlayerState player1("P1");
PlayerState player2("P2");

class GameState {
  public:

  String phase = "players";
  int currentPlayer = 1;
  int currentStat = 0;
  int forgeModifier = 0;
  int penalty = 0;

  void changeForgeMod(int delta) {
    forgeModifier = commons.calcChange(forgeModifier, delta, rules.maxForgeMod, rules.minForgeMod);
  }
};

GameState game;

class GameVisuals {
  private:

  int p1offset = 0;
  int p2offset = 11;

  void renderLabels() {
    lcd.setCursor(6, 0);
    lcd.print("ch:p");
    lcd.setCursor(6, 1);
    lcd.print("ae:k");
  }

  void renderStatusCursor() {
    int offset = game.currentPlayer == 1 ? p1offset : p2offset;
    if (game.currentStat == 0) lcd.setCursor(1 + offset, 0);
    if (game.currentStat == 1) lcd.setCursor(1 + offset, 1);
    if (game.currentStat == 2) lcd.setCursor(3 + offset, 1);
  }
  
  void renderPlayerData(PlayerState thePlayer, int offset) {
    // print chains:penalty
    lcd.setCursor(offset, 0);
    if (thePlayer.chains < 10) lcd.print(" ");
    lcd.print(thePlayer.chains);
    lcd.print(rules.calcPenalty(thePlayer.chains));
    // print aember:keys
    lcd.setCursor(offset, 1);
    if (thePlayer.aember < 10) lcd.print(" ");
    lcd.print(thePlayer.aember);
    lcd.print(":");
    lcd.print(thePlayer.keys);
  }

  public:

  void renderPlayersView(PlayerState player1, PlayerState player2) {
    lcd.home();
    renderLabels();
    renderPlayerData(player1, p1offset);
    renderPlayerData(player2, p2offset);
    renderStatusCursor();
  }

  void renderChainsPromptView() {}
  void renderDrawCardsView() {}
  void renderForgePromptView() {}
};

GameVisuals visuals;

int lastButton;
unsigned long lastRender = 0;

void loop() {
  int currentButtons = lcd.readButtons();
  if (currentButtons != lastButton) {
    if (currentButtons == BUTTON_SELECT) {
      // do something
    }
    if (currentButtons == BUTTON_LEFT) {
      // do something
    }
    if (currentButtons == BUTTON_RIGHT) {   
      // do something
    }
    if (currentButtons == BUTTON_DOWN) {
      // do something
    }
    if (currentButtons == BUTTON_UP) {
      // do something
    }
    // toggle between players:
    if (currentButtons == BUTTON_RIGHT + BUTTON_SELECT) {
      game.currentPlayer = game.currentPlayer == 1 ? 2 : 1;
    }
  }

  lastButton = currentButtons;
  unsigned long currentTime = millis();

  if (currentTime - lastRender > 300) {
    lastRender = currentTime;
    visuals.renderPlayersView(player1, player2);
  }
}
