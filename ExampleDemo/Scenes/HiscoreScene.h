#pragma once
#include "../TextureLoader.h"
#include "../Button.h"
#include "../ResourcePaths.h"
#include "../SaveFile.h"

#include <Swoosh/ActivityController.h>
#include <Swoosh/Utils.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Segues/SlideIn.h>
#include <Segues/CircleClose.h>
#include <Segues/RetroBlit.h>
#include <iostream>

class MainMenuScene;

class HiScoreScene : public sw::Activity {
private:
  sf::Texture * meteorBig, * meteorMed, * meteorSmall, * meteorTiny, * btn;

  std::unique_ptr<button> goBack;

  sf::Font   font;
  sf::Text   text;

  sf::SoundBuffer buffer;
  sf::Sound selectFX;

  float screenDiv;
  float screenMid;
  float screenBottom;

  sw::Timer waitTime;
  double scrollOffset;

  bool inFocus;
public:
  SaveFile& saveFile;

  HiScoreScene(sw::ActivityController& controller, SaveFile& save) : Activity(&controller), text (font), selectFX (buffer), saveFile(save) {
    // Proof that this is the same save file in memory as it is passed around the scenes
    std::cout << "savefile address is " << &save << std::endl;

    // keep our window size dimensions consistent based on the initial window size when the AC was created
    auto windowSize = getController().getVirtualWindowSize();
    setView(windowSize);

    if (!font.openFromFile(GAME_FONT)) {}
    text.setFillColor(sf::Color::White);

    btn = loadTexture(BLUE_BTN_PATH);
    goBack = std::make_unique <button> ((*btn));
    goBack->text = "Return";

    meteorBig = loadTexture(METEOR_BIG_PATH);
    meteorMed = loadTexture(METEOR_MED_PATH);
    meteorSmall = loadTexture(METEOR_SMALL_PATH);
    meteorTiny = loadTexture(METEOR_TINY_PATH);

    screenBottom = (float)windowSize.y;
    screenMid = windowSize.x / 2.0f;
    screenDiv = windowSize.x / 4.0f;

    if (saveFile.empty()) {
      saveFile.writeToFile(SAVE_FILE_PATH);
      saveFile.loadFromFile(SAVE_FILE_PATH);
    }

    waitTime.start();
    scrollOffset = 0;

    // Load sounds
    if (buffer.loadFromFile(SHIELD_UP_SFX_PATH)) {}

    inFocus = false;

    this->setBGColor(sf::Color::Black);
  }

  void onStart() override {
    inFocus = true;
  }

  void onUpdate(double elapsed) override {
    waitTime.update(sf::seconds((float)elapsed));

    goBack->update(getController().getWindow());

    if (goBack->isClicked && inFocus) {
      selectFX.play();

      // Rewind lets us pop back to a particular scene in our stack history
      using fx = segue<CircleClose, arg::sec<1>>;
      using tx = fx::to<MainMenuScene>;
      const bool found = getController().rewind<tx>(saveFile);

      // should never happen
      // but your games may need to check so here it is an example
      assert(found && "MainMenuScene not found in our running game");
    }

    // After 3 seconds, scroll up
    if (waitTime.getElapsed().asSeconds() > 3) {

      // If the scroll offset is greater than the height of all drawn scores
      if (scrollOffset > 200 + (saveFile.names.size() * 100)) {
        // We hit them all, reset
        scrollOffset = 0;
        waitTime.reset();
      }
      else {
        scrollOffset += 100.0 * elapsed;
      }
    }

  }

  void onLeave() override {
    inFocus = false;
  }

  void onExit() override {
  }

  void onEnter() override {
  }

  void onResume() override {
  }

  void onDraw(sw::IRenderer& renderer) override {
    text.setFillColor(sf::Color::Yellow);
    text.setPosition(sf::Vector2f(screenMid, 100));
    text.setString("Hi Scores");
    sw::setOrigin(text, 0.5, 0.5);
    renderer.submit(sw::Immediate(&text));

    text.setFillColor(sf::Color::White);

    for (uint32_t i = 0; i < saveFile.names.size(); i++) {
      std::string name = saveFile.names[i];
      int score = saveFile.scores[i];

      text.setString(name);
      text.setPosition(sf::Vector2f((float)(screenDiv), (float)(200 + (i*100) - scrollOffset)));
      sw::setOrigin(text, 0.5, 0.5);
      renderer.submit(sw::Immediate(&text));

      text.setString(std::to_string(score));
      text.setPosition(sf::Vector2f((float)(screenDiv * 3), (float)(200 + (i*100) - scrollOffset)));
      sw::setOrigin(text, 0.5, 0.5);
      renderer.submit(sw::Immediate(&text));
    }

    text.setFillColor(sf::Color::Black);
    goBack->draw(renderer, text, screenMid, screenBottom - 40);
  }

  void onEnd() override {
  }

  ~HiScoreScene() { delete btn; }
};
