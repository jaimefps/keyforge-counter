#include <Adafruit_RGBLCDShield.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// NOW : use enums to track static list options:

void setup() {
  lcd.begin(16, 2);
  lcd.cursor();
  Serial.begin(9600);
}

enum GamePhases {
  titlePage,
  player1Prompt,
  player2Prompt,
  mainPlayPhase,
  chainsPrompt,
  drawPrompt,
  forgePrompt
};

enum PlayerStats { 
  chains,
  keys,
  aember
};

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
  int chains = rules.chainRange[0];
  int aember = rules.aemberRange[0];
  int keys = rules.keyRange[0];
  int hand = 0;

  PlayerState(String initName) {
    name = initName;
  }

  void changeHand(int delta) {
    hand = commons.calcChange(hand, delta, rules.handRange[0], rules.handRange[1]);
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

  void changeStat(int stat, int delta) {
    if (stat == 0) changeChains(delta);
    if (stat == 1) changeAember(delta);
    if (stat == 2) changeKeys(delta);
  }
};


class GameState {
  public:

  PlayerState player1 = PlayerState("P1");
  PlayerState player2 = PlayerState("P2");

  int currentPhase = titlePage;
  int currentStat = 0;
  int currentPlayer = 1;
  int forgeModifier = 0;
  int penalty = 0;

  void changeForgeMod(int delta) {
    forgeModifier = commons.calcChange(forgeModifier, delta, rules.forgeModRange[0], rules.forgeModRange[1]);
  }

  void nextPhase() {
    switch(currentPhase) {
      case titlePage:
        currentPhase = player1Prompt;
        break;

      case player1Prompt:
        currentPhase = player2Prompt;
        break;

      case player2Prompt:
        currentPhase = mainPlayPhase;
        break;

      case mainPlayPhase:
        currentPhase = chainsPrompt;
        break;

      case chainsPrompt:
        currentPhase = drawPrompt;
        break;

      case drawPrompt:
        currentPhase = forgePrompt;
        break;

      case forgePrompt:
        currentPhase = mainPlayPhase;
        break;
    }
  }

  void changePlayerStat(int delta) {
    if (currentPlayer == 1) player1.changeStat(currentStat, delta);
    else player2.changeStat(currentStat, delta);
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
    lcd.print(":");
    lcd.print(rules.calcPenalty(thePlayer.chains));
    // print aember:keys
    lcd.setCursor(offset, 1);
    if (thePlayer.aember < 10) lcd.print(" ");
    lcd.print(thePlayer.aember);
    lcd.print(":");
    lcd.print(thePlayer.keys);
  }

  public:

  void renderTitle() {
    lcd.print("KEYFORGE !");
  }

  void renderP1Prompt(GameState game) {
    lcd.print("p1 name");
  }

  void renderP2Prompt(GameState game) {
    lcd.print("p2 name");
  }

  void renderPlayers(GameState game) {
    renderLabels();
    renderPlayerData(game.player1, p1offset);
    renderPlayerData(game.player2, p2offset);
    renderStatusCursor();
  }

  void renderChainsPrompt(GameState game) {
    lcd.print("chains prompt");
  }

  void renderDrawCards(GameState game) {
    lcd.print("draw cards prompt");
  }

  void renderForgePrompt(GameState game) {
    lcd.print("forge prompt");
  }

  bool isGameOver(GameState game) {
    if (game.player1.keys == 3) return true;
    if (game.player2.keys == 3) return true;
    return false;
  }

  void renderGameOver(GameState game) {
    String winner;
    if (game.player1.keys == 3) winner = game.player1.name;
    if (game.player2.keys == 3) winner = game.player2.name;
    lcd.print(winner);
    lcd.print(" wins!");
  }

  void render(GameState game) {
    lcd.home();
    if (isGameOver(game)) {
      renderGameOver(game);
    } else {
      if (game.currentPhase == titlePage) renderTitle();
      if (game.currentPhase == player1Prompt) renderP1Prompt(game);
      if (game.currentPhase == player2Prompt) renderP2Prompt(game);
      if (game.currentPhase == mainPlayPhase) renderPlayers(game);
      if (game.currentPhase == chainsPrompt) renderChainsPrompt(game);
      if (game.currentPhase == drawPrompt) renderDrawCards(game);
      if (game.currentPhase == forgePrompt) renderForgePrompt(game);
    }
  }
};

GameVisuals visuals;

int lastButton;
unsigned long lastRender = 0;

void loop() {
  const int currentButtons = lcd.readButtons();

  if (currentButtons != lastButton) {
    if (currentButtons == BUTTON_SELECT) {
      lcd.clear();
      game.nextPhase();
    }
    if (currentButtons == BUTTON_LEFT) {
      game.currentStat--;
      if (game.currentStat < 0) {
        game.currentStat = 2;
      }
    }
    if (currentButtons == BUTTON_RIGHT) {
      game.currentStat++;
      game.currentStat %= 3;
    }
    if (currentButtons == BUTTON_UP) {
      if (game.currentPhase == mainPlayPhase) game.changePlayerStat(1);
      // if (game.currentPhase == game.phases[4]) game.changeCardsInHand(1);
      // if (game.currentPhase == game.phases[6]) game.changeForgeMods(1);
    }
    if (currentButtons == BUTTON_DOWN) {
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
