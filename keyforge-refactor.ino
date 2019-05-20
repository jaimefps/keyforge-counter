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

  const int baseKeyCost = 6;
  const int baseHandSize = 6;
  const int keyRange[2] = {0, 3};
  const int chainRange[2] = {0, 24};
  const int aemberRange[2] = {0, 99};
  const int forgeModRange[2] = {-99, 99};
  const int handRange[2] = {0, 36};

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

  int calcChange(int currentValue, int delta, int minimum, int maximum) {
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
  int aember = rules.aemberRange[0];
  int chains = rules.chainRange[0];
  int keys = rules.keyRange[0];

  PlayerState(String initName) {
    name = initName;
  }

  void changeAember(int delta) {
    aember = commons.calcChange(aember, delta, rules.aemberRange[0], rules.aemberRange[1]);
  }

  void changeKeys(int delta) {
    keys = commons.calcChange(keys, delta, rules.keyRange[0], rules.keyRange[1]);
  }

  void changeChains(int delta) {
    chains = commons.calcChange(chains, delta, rules.chainRange[0], rules.chainRange[1]);
  }
};


class GameState {
  public:

  PlayerState player1 = PlayerState("P1");
  PlayerState player2 = PlayerState("P2");

  String phase = "players";
  int currentPlayer = 1;
  int currentStat = 0;
  int forgeModifier = 0;
  int penalty = 0;

  void changeForgeMod(int delta) {
    forgeModifier = commons.calcChange(forgeModifier, delta, rules.forgeModRange[0], rules.forgeModRange[1]);
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

  void renderPlayersView(GameState game) {
    renderLabels();
    renderPlayerData(game.player1, p1offset);
    renderPlayerData(game.player2, p2offset);
    renderStatusCursor();
  }

  void renderChainsPromptView(GameState game) {}
  void renderDrawCardsView(GameState game) {}
  void renderForgePromptView(GameState game) {}

  void render(GameState game) {
    lcd.home();
    if (game.phase == "players") renderPlayersView(game);
    if (game.phase == "chainsPrompt") renderChainsPromptView(game);
    if (game.phase == "drawPrompt") renderDrawCardsView(game);
    if (game.phase == "forgePrompt") renderForgePromptView(game);
  }
};

GameVisuals visuals;

class MachineInterface {
  private:

  int lastButton;
  unsigned long lastRender = 0;

  public:

  void handleInteractions(GameState game, GameVisuals visuals) {
    const int currentButtons = lcd.readButtons();

    if (currentButtons != lastButton) {
      if (currentButtons == BUTTON_SELECT) {
        lcd.clear();
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
      if (currentButtons == BUTTON_RIGHT + BUTTON_LEFT) {
        game.currentPlayer = game.currentPlayer == 1 ? 2 : 1;
      }
    }

    lastButton = currentButtons;
    unsigned long currentTime = millis();

    if (currentTime - lastRender > 300) {
      lastRender = currentTime;
      visuals.render(game);
    }

  }
};

MachineInterface machine;

void loop() {
  machine.handleInteractions(game, visuals);
}
