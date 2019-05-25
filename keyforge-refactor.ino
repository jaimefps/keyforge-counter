#include <Adafruit_RGBLCDShield.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

/**
 * TODO
 *
 * 1- titlePage animation
 * 1.2 - winner screen animation
 * 2- prompt for player names before game starts
 */

void setup() {
  lcd.begin(16, 2);
  lcd.cursor();
  Serial.begin(9600);
}

enum PlayerStats { 
  ch, 
  ae, 
  ky
};

enum GamePhases {
  titlePage,
  // player1Prompt,
  // player2Prompt,
  mainPlayPhase,
  chainsPrompt,
  drawPrompt,
  forgePrompt
};

int calcChange(int currentValue, int delta, int minimum, int maximum) {
  return min(max(currentValue + delta, minimum), maximum);
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

const GameRules rules;

class PlayerState {
  public:

  String name;
  int chains = rules.chainRange[0];
  int aember = rules.aemberRange[0];
  int keys = rules.keyRange[0];

  PlayerState(String initName) {
    name = initName;
  }

  void changeAember(int delta) {
    aember = calcChange(aember, delta, rules.aemberRange[0], rules.aemberRange[1]);
  }

  void changeKeys(int delta) {
    keys = calcChange(keys, delta, rules.keyRange[0], rules.keyRange[1]);
  }

  void changeChains(int delta) {
    chains = calcChange(chains, delta, rules.chainRange[0], rules.chainRange[1]);
  }

  void changeStat(int stat, int delta) {
    if (stat == ch) changeChains(delta);
    if (stat == ae) changeAember(delta);
    if (stat == ky) changeKeys(delta);
  }
};


class GameState {
  public:

  PlayerState player1 = PlayerState("P1");
  PlayerState player2 = PlayerState("P2");

  int currentStat = 0;
  int currentPhase = titlePage;

  int targetPlayerStats = 1;
  int currentPlayer = 1;

  int currentPlayerForgeMod = 0;
  int currentPlayerHand = 0;
  int currentPlayerPenalty = 0;

  int calcKeyCost() {
    return max(rules.baseKeyCost + currentPlayerForgeMod, 0);
  }

  void changeForgeMod(int delta) {
    currentPlayerForgeMod = calcChange(currentPlayerForgeMod, delta, rules.forgeModRange[0], rules.forgeModRange[1]);
  }

  void changeHandSize(int delta) {
    currentPlayerHand = calcChange(currentPlayerHand, delta, rules.handRange[0], rules.handRange[1]);
  }

  void changePlayerStat(int delta) {
    if (targetPlayerStats == 1) player1.changeStat(currentStat, delta);
    else player2.changeStat(currentStat, delta);
  }

  void setNextTurnState() {
    currentStat = 0;
    currentPlayerHand = 0;
    currentPlayerPenalty = 0;
    currentPlayer = currentPlayer == 1 ? 2 : 1;
    targetPlayerStats = currentPlayer;
  }

  void attemptForge() {
    int keyCost = calcKeyCost();
    if (currentPlayer == 1 && player1.aember >= keyCost) {
      player1.changeStat(2, 1);
      player1.changeStat(1, -keyCost);
    }
    if (currentPlayer == 2 && player2.aember >= keyCost){
      player2.changeStat(2, 1);
      player2.changeStat(1, -keyCost);
    }
  }

  void nextPhase() {
    bool p1SkipChains = currentPlayer == 1 && player1.chains == 0;
    bool p2SkipChains = currentPlayer == 2 && player2.chains == 0;

    switch(currentPhase) {
      case titlePage:
        currentPhase = mainPlayPhase;
        break;

      // case player1Prompt:
      //   currentPhase = player2Prompt;
      //   break;
      // case player2Prompt:
      //   currentPhase = mainPlayPhase;
      //   break;

      case mainPlayPhase:
        if (p1SkipChains || p2SkipChains) {
          setNextTurnState();
          currentPhase = forgePrompt;
        } else {
          currentPhase = chainsPrompt;
        }
        break;

      case chainsPrompt:
        currentPlayerPenalty = currentPlayer == 1 
          ? rules.calcPenalty(player1.chains) 
          : rules.calcPenalty(player2.chains);
        if (currentPlayerHand < rules.baseHandSize + currentPlayerPenalty) {
          if (currentPlayer == 1) player1.changeStat(0, -1);
          else player2.changeStat(0, -1);
        }
        currentPhase = drawPrompt;
        break;

      case drawPrompt:
        setNextTurnState();
        currentPhase = forgePrompt;
        break;

      case forgePrompt:
        attemptForge();
        currentPlayerForgeMod = 0;
        currentPhase = mainPlayPhase;
        break;
    }
  }
};

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

  void renderStatusCursor(GameState game) {
    int offset = game.targetPlayerStats == 1 ? p1offset : p2offset;
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
    if (thePlayer.chains == 0) lcd.print(" ");
    // print aember:keys
    lcd.setCursor(offset, 1);
    if (thePlayer.aember < 10) lcd.print(" ");
    lcd.print(thePlayer.aember);
    lcd.print(":");
    lcd.print(thePlayer.keys);
  }

  int getDrawAmount(GameState game) {
    int drawCapacity = 
      rules.baseHandSize 
      + game.currentPlayerPenalty 
      - game.currentPlayerHand;
    return max(drawCapacity, 0);
  }

  String getPlayerName(GameState game) {
    return game.currentPlayer == 1 
      ? game.player1.name 
      : game.player2.name;
  }

  void hideCursor() {
    lcd.setCursor(16, 0);
  }

  public:

  void renderTitle() {
    lcd.print("KEYFORGE !");
    hideCursor();
  }

  // void renderP1Prompt(GameState game) {
  //   lcd.print("p1 name");
  // }

  // void renderP2Prompt(GameState game) {
  //   lcd.print("p2 name");
  // }

  void renderPlayers(GameState game) {
    renderLabels();
    renderPlayerData(game.player1, p1offset);
    renderPlayerData(game.player2, p2offset);
    renderStatusCursor(game);
  }

  void renderChainsPrompt(GameState game) {
    lcd.print("# of cards " + getPlayerName(game) + ":");
    if (game.currentPlayerHand < 10) lcd.print(" ");
    lcd.print(game.currentPlayerHand);
    lcd.setCursor(15,0);
  }

  void renderDrawCards(GameState game) {
    lcd.home();
    lcd.print(getPlayerName(game) + " draws ");
    lcd.print(getDrawAmount(game));
    hideCursor();
  }

  void renderForgePrompt(GameState game) {
    lcd.home();
    const int mod = game.currentPlayerForgeMod;
    lcd.print(getPlayerName(game) + " forge mod:");
    if (mod < 0 && mod > -10) lcd.print(" ");
    if (mod == 0) lcd.print("  ");
    if (mod > 0 && mod < 10) lcd.print(" +");
    if (mod > 9) lcd.print("+");
    lcd.print(mod);
    lcd.setCursor(15, 0);
  }

  bool isGameOver(GameState game) {
    return game.player1.keys == 3 || game.player2.keys == 3 ? true : false;
  }

  // assumes ties are not possible.
  void renderWinner(GameState game) {
    lcd.clear();
    String winner;
    if (game.player1.keys == 3) winner = game.player1.name;
    if (game.player2.keys == 3) winner = game.player2.name;
    lcd.print(winner);
    lcd.print(" wins!");
    hideCursor();
  }

  void render(GameState game) {
    lcd.home();
    if (isGameOver(game)) {
      renderWinner(game);
    } else {
      if (game.currentPhase == titlePage) renderTitle();
      // if (game.currentPhase == player1Prompt) renderP1Prompt(game);
      // if (game.currentPhase == player2Prompt) renderP2Prompt(game);
      if (game.currentPhase == mainPlayPhase) renderPlayers(game);
      if (game.currentPhase == chainsPrompt) renderChainsPrompt(game);
      if (game.currentPhase == drawPrompt) renderDrawCards(game);
      if (game.currentPhase == forgePrompt) renderForgePrompt(game);
    }
  }
};

class MachineState {
  private: 

  GameState game;
  const GameVisuals visuals;

  int lastButton;
  unsigned long lastRender = 0;

  void handleUserActions() {
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
        if (game.currentPhase == chainsPrompt) game.changeHandSize(1);
        if (game.currentPhase == forgePrompt) game.changeForgeMod(1);
      }
      if (currentButtons == BUTTON_DOWN) {
        if (game.currentPhase == mainPlayPhase) game.changePlayerStat(-1);
        if (game.currentPhase == chainsPrompt) game.changeHandSize(-1);
        if (game.currentPhase == forgePrompt) game.changeForgeMod(-1);
      }
      // toggle between players without finish the current turn:
      if (currentButtons == BUTTON_RIGHT + BUTTON_LEFT) {
        game.targetPlayerStats = game.targetPlayerStats == 1 ? 2 : 1;
      }
    }
    lastButton = currentButtons;
  }

  void renderGame() {
    unsigned long currentTime = millis();
    if (currentTime - lastRender > 250) {
      lastRender = currentTime;
      visuals.render(game);
    }
  }

  public:

  void handleGame() {
    handleUserActions();
    renderGame();
  }
};

MachineState machine;

void loop() {
  machine.handleGame();
}
