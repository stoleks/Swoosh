#pragma once

#include "MainMenuScene.h"
#include "../TextureLoader.h"
#include "../Button.h"
#include "../ResourcePaths.h"

#include <Segues/VerticalSlice.h>
#include <Swoosh/ActivityController.h>
#include <Swoosh/Utils.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

const char* LOADING = "NOW LOADING";

// TODO: what to do visually with this scene?
class IntroScene : public sw::Activity{
private:
  sf::Font font;
  sf::Text text;

  sw::Timer timer;

  bool inFocus;
public:
  IntroScene(sw::ActivityController& controller) : Activity(&controller) {
    font.loadFromFile(GAME_FONT);
    text.setFont(font);
    text.setString(LOADING);
    text.setFillColor(sf::Color::White);
    sw::setOrigin(text, 0.5f, 0.5f);

    sf::Vector2u windowSize = getController().getVirtualWindowSize();
    setView(windowSize);
    text.setPosition(windowSize.x * 0.5f, windowSize.y * 0.5f);

    inFocus = false;
    timer.start();
  }

  void onStart() override {
    inFocus = true;
  }

  void onUpdate(double elapsed) override {
    timer.update(sf::seconds((float)elapsed));

    // faux paux "loading" time
    if (timer.getElapsed().asSeconds() > 3) {
      using fx = segue<VerticalSlice>;
      using tx = fx::to<MainMenuScene>;

      auto onReturn = [](sw::Context& context) {
        // We can check for previous contexts which were adopted
        auto& prev = context.previous();
        if (!prev.has_value()) return;

        sw::Context& prevContext = prev.value();
        std::cout << "prevContext typename: " << prevContext.type() << std::endl;
        if (!prevContext.has<std::string, bool, int>()) return;
        auto& [str, b, i] = prevContext.read<std::string, bool, int>();
        std::cout << str << ", " << b << ", " << i << std::endl;

      };
      getController()
        .push<tx>()
        .take(onReturn);

      // Reset so we can return and kick off the effect again
      timer.reset();
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
    renderer.submit(sw::Immediate(&text));
  }

  void onEnd() override {
  }

  ~IntroScene() { }
};
