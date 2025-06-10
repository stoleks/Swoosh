#pragma once

#include "../TextureLoader.h"
#include "../Button.h"
#include "../ResourcePaths.h"
#include "../SaveFile.h"

#include <Segues/PushIn.h>
#include <Segues/BlendFadeIn.h>
#include <Segues/Cube3D.h>
#include <Swoosh/ActivityController.h>
#include <Swoosh/Utils.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <memory>
#include <iostream>

const char* TEXT_BLOCK_INFO = 
"Swoosh is an Activity and Segue mini library\n" \
"designed to make complex screen transitions\n" \
"a thing of the past.\n" \
"This is a proof-of-concept demo showcasing\n" \
"its features and includes helpful utilities\n" \
"for your SFML apps or games.\n\n" \
"Fork at\ngithub.com/TheMaverickProgrammer/Swoosh";

const char* CONTROLS_INFO = 
">> Left click to shoot\n\n"\
">> Right click to boost and dodge\n\n" \
">> Collect stars for extra life\n\n";

class AboutScene : public sw::Activity {
private:
  sf::Texture * btn;
  sf::Texture * sfmlTexture;
  std::unique_ptr <sf::Sprite> sfml;
  std::unique_ptr <button> goBack;

  sf::Font manual;
  sf::Font font;
  sf::Text text;
  std::string info;

  sf::SoundBuffer buffer;
  sf::Sound selectFX;

  float screenDiv;
  float screenMid;
  float screenBottom;

  sw::Timer timer;

  bool inFocus;
  bool canClick;
public:
  AboutScene(sw::ActivityController& controller) : Activity(&controller), text(font), selectFX (buffer) {
    canClick = false;

    if (!font.openFromFile(GAME_FONT)) {}
    text.setFillColor(sf::Color::White);

    if (!manual.openFromFile(MANUAL_FONT)) {}

    btn = loadTexture(BLUE_BTN_PATH);
    goBack = std::make_unique <button> (*btn);
    goBack->text = "Continue";
    info = TEXT_BLOCK_INFO;

    sfmlTexture = loadTexture(SFML_PATH);
    sfml = std::make_unique <sf::Sprite> (*sfmlTexture);
    sfml->setScale({0.7f, 0.7f});
    sw::setOrigin(*sfml, 0.60f, 0.60f);

    sf::Vector2u windowSize = getController().getVirtualWindowSize();
    setView(windowSize);

    screenBottom = (float)windowSize.y;
    screenMid = windowSize.x / 2.0f;
    screenDiv = windowSize.y / 4.0f;

    // Load sounds
    if (!buffer.loadFromFile(SHIELD_UP_SFX_PATH)) {}

    inFocus = false;

    timer.start();
  }

  void onStart() override {
    inFocus = true;
  }

  void onUpdate(double elapsed) override {
    timer.update(sf::seconds((float)elapsed));

    double offset = 0;
    if (timer.getElapsed().asSeconds() > 3) {
      offset = ease::wideParabola(timer.getElapsed().asSeconds()-3.0, 5.0, 0.9);
    }

    sf::Vector2u windowSize = getController().getVirtualWindowSize();

    sfml->setPosition({100.0f + (float)(offset * (windowSize.x - 300)), 100.0f});
    sfml->setRotation(sf::degrees(offset * 360 * 2));

    goBack->update(getController().getWindow());

    if (goBack->isClicked && inFocus) {
      if (canClick) {
        canClick = false;
        selectFX.play();

        if (goBack->text == "FIN") {
          struct Reason {
            std::string message;
          };

          using tx = segue<Cube3D<arg::direction::right>, arg::sec<2>>;
          getController().pop<tx>(std::string("Goodbye from the AboutScene!"), false, 12);
        }
        else {
          goBack->text = "FIN";
          info = CONTROLS_INFO;
        }
      }

    }

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !canClick) {
      canClick = true;
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
    renderer.clear(sf::Color::Black);
    renderer.submit(sfml.get ());

    text.setFont(manual);
    text.setPosition(sf::Vector2f(screenMid, 200));
    text.setFillColor(sf::Color::White);
    text.setString(info);
    sw::setOrigin(text, 0.5f, 0);

    renderer.submit(sw::Immediate(&text));

    text.setFont(font);
    text.setFillColor(sf::Color::Black);
    sw::setOrigin(text, 0.5f, 0.5f);
    goBack->draw(renderer, text, screenMid, screenBottom - 40);
  }

  void onEnd() override {
  }

  ~AboutScene() { delete btn; }
};
