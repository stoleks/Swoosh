#pragma once

#include "HiscoreScene.h"
#include "AboutScene.h"
#include "GamePlayScene.h"
#include "../CustomRenderer.h"
#include "../TextureLoader.h"
#include "../Button.h"
#include "../SaveFile.h"

// You can use any of the included segues in the actions below
// to see what they look like! Start at line 170 in this source file!
#include <Segues/BlackWashFade.h>
#include <Segues/CrossZoom.h>
#include <Segues/ZoomFadeIn.h>
#include <Segues/ZoomFadeInBounce.h>
#include <Segues/Checkerboard.h>
#include <Segues/WhiteWashFade.h>
#include <Segues/SlideIn.h>
#include <Segues/BlendFadeIn.h>
#include <Segues/PushIn.h>
#include <Segues/PageTurn.h>
#include <Segues/ZoomOut.h>
#include <Segues/ZoomIn.h>
#include <Segues/HorizontalSlice.h>
#include <Segues/VerticalSlice.h>
#include <Segues/HorizontalOpen.h>
#include <Segues/VerticalOpen.h>
#include <Segues/PixelateBlackWashFade.h>
#include <Segues/BlurFadeIn.h>
#include <Segues/SwipeIn.h>
#include <Segues/DiamondTileSwipe.h>
#include <Segues/DiamondTileCircle.h>
#include <Segues/CircleOpen.h>
#include <Segues/CircleClose.h>
#include <Segues/Morph.h>
#include <Segues/RadialCCW.h>
#include <Segues/Cube3D.h>
#include <Segues/RetroBlit.h>
#include <Segues/Dream.h> // 10/10/2020
// end segue effects

#include <Swoosh/ActivityController.h>
#include <Swoosh/Utils.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

const char* GAME_TITLE = "Swoosh Interactive Demo";
const char* PLAY_OPTION = "Play";
const char* SCORE_OPTION = "HiScore";
const char* ABOUT_OPTION = "About";
const char* QUIT_OPTION = "Quit";

class MainMenuScene : public sw::Activity {
private:
  sf::Texture* bgTexture, *bgNormal;
  sf::Texture* starTexture;
  sf::Texture* blueButton, *redButton, *greenButton;

  sf::Font menuFont;
  sf::Text menuText;

  sf::SoundBuffer buffer;
  sf::Sound selectFX;
  sf::Music themeMusic;

  std::vector<button> buttons;

  float screenMid;
  bool inFocus;
  bool fadeMusic;

  sw::Timer timer; // for onscreen effects. Or we could have stored the total elapsed from the update function
  SaveFile savefile;

public:
  MainMenuScene(sw::ActivityController& controller) : Activity(&controller), menuText (menuFont), selectFX (buffer) {
    setView(controller.getVirtualWindowSize());

    savefile.loadFromFile(SAVE_FILE_PATH);

    inFocus = true;
    fadeMusic = false;

    bgTexture = loadTexture(MENU_BG_PATH);
    bgNormal = loadTexture(MENU_BG_N_PATH);

    starTexture = loadTexture(STAR_PATH);

    blueButton = loadTexture(BLUE_BTN_PATH);
    redButton = loadTexture(RED_BTN_PATH);
    greenButton = loadTexture(GREEN_BTN_PATH);

    if (!menuFont.openFromFile(GAME_FONT)) {}
    menuText.setFillColor(sf::Color::White);

    screenMid = getController().getWindow().getSize().x / 2.0f;

    // Create the buttons
    button menuOption (*greenButton);
    menuOption.text = PLAY_OPTION;
    buttons.push_back(menuOption);

    menuOption.sprite.setTexture(*blueButton);
    menuOption.text = SCORE_OPTION;
    buttons.push_back(menuOption);

    menuOption.sprite.setTexture(*blueButton);
    menuOption.text = ABOUT_OPTION;
    buttons.push_back(menuOption);

    menuOption.sprite.setTexture(*redButton);
    menuOption.text = QUIT_OPTION;
    buttons.push_back(menuOption);

    // Load sounds
    if (!buffer.loadFromFile(SHIELD_UP_SFX_PATH)) {}
    if (!themeMusic.openFromFile(THEME_MUSIC_PATH)) {}

    timer.start();

    this->setBGColor(sf::Color(56, 7, 67));
  }

  void onStart() override {
    std::cout << "MainMenuScene OnStart called" << std::endl;
    themeMusic.play();
  }

  void onUpdate(double elapsed) override {
    timer.update(sf::seconds((float)elapsed));

    if (!inFocus && fadeMusic) {
      themeMusic.setVolume(themeMusic.getVolume() * 0.98f); // fades out the music
    }

    for (auto& b : buttons) {
      b.update(getController().getWindow());

      if (b.isClicked && inFocus) {
        selectFX.play();

        if (b.text == PLAY_OPTION) {
          using fx = sw::segue<HorizontalOpen>;
          using tx = fx::to<GameplayScene>;
          getController().push<tx>(savefile);
          fadeMusic = true;
        }
        else if (b.text == SCORE_OPTION) {
          using fx = segue<RadialCCW, arg::sec<2>>;
          using tx = fx::to<HiScoreScene>;

          auto onReturn =
            [this](sw::Context& context) {
            // Notice that this callback happens ONLY when we return
            // _directly_ from the HiScoreScene from this option and not from
            // the PLAY_OPTION flow.
            if (!context.has<SaveFile>()) return;
            SaveFile& s = context.read<SaveFile>();

            std::cout << "Recent hiscore was: " << s.scores.back() << std::endl;
            };

          getController()
            .push<tx>(savefile) // pass savefile into next scene's ctor
            .take(onReturn);    // when we return, obtain data passed up
        }
        else if (b.text == ABOUT_OPTION) {
          using fx = segue<PageTurn, arg::sec<2>>;
          using tx = fx::to<AboutScene>;
          
          // adopt() stores the context data to forward when this scene also
          // pops off the stack. The alternative would be to take(Context&),
          // then check, somehow store the data manually, and finally pass
          // this data back wherever pop() is called. 
          // adopt() conveniently does this for you.
          getController().push<tx>().adopt();
        }
        else if (b.text == QUIT_OPTION) {
          using fx = segue<ZoomFadeIn>;
          getController().pop<fx>();
        }
      }
    }
  }

  void onLeave() override {
    std::cout << "MainMenuScene OnLeave called" << std::endl;
    inFocus = false;
  }

  void onExit() override {
    std::cout << "MainMenuScene OnExit called" << std::endl;

    if (fadeMusic) {
      themeMusic.stop();
    }
  }

  void onEnter() override {
    std::cout << "MainMenuScene OnEnter called" << std::endl;
  }

  void onResume() override {
    inFocus = true;

    // If fadeMusic == true, then we were coming from demo, the music changes
    if (fadeMusic) {
      themeMusic.play();
      themeMusic.setVolume(100);
    }

    fadeMusic = false;

    std::cout << "MainMenuScene OnResume called" << std::endl;
  }

  void onDraw(sw::IRenderer& renderer) override {
    const bool isCustomRenderer = getController().getActiveRenderEntry().getName() == "custom";

    auto bg = sf::Sprite (*bgTexture);
    renderer.submit(Draw3D(&bg, bgNormal));

    int i = 0;
    menuText.setFillColor(sf::Color::Black);

    for (auto& b : buttons) {
      b.draw(renderer, menuText, screenMid, (float)(200 + (i++*100)));
    }

    // First set the text as the it would render as a full string
    menuText.setString(GAME_TITLE);
    sw::setOrigin(menuText, 0.5, 0.5);

    // -30 is a made up number offset to help it look right
    menuText.setPosition(sf::Vector2f(screenMid-30.f, 100));

    // Get the global bounds information from that to pick out
    double startX = menuText.getGlobalBounds().position.x;
    double offset = 0;

    // For each letter in the string, make it jump while preserving placement
    size_t len = strlen(GAME_TITLE);
    double frequency = sw::ease::pi * 2.0 / len;
    double dt = timer.getElapsed().asSeconds();
    for (size_t i = 0; i < len; i++) {
      menuText.setFillColor(sf::Color::White);
      menuText.setString(GAME_TITLE[i]);
      sw::setOrigin(menuText, 0.5, 0.5); // origin is in the center of the letter

      // This creates our wave over all letters
      double ratio = (ease::pi) / len;
      double wave = (std::sin(timer.getElapsed().asSeconds()*2.0+((i+1)*ratio)));

      // Only add the peaks
      double startY = 100 + ((wave > 0)? -wave * 50.0 : 0);

      // Each letter is separate so add back the spacing
      const float letterSpacing = static_cast<float>(menuText.getCharacterSize()) / 3.0f;

      // We want at max 24.f units of space inbetween letters
      // Everything else can be smaller
      offset += std::fminf(menuText.getGlobalBounds().size.x + letterSpacing, 24.f);

      // Include spaces
      if (menuText.getString() == ' ') { offset += menuText.getCharacterSize(); }

      menuText.setPosition(sf::Vector2f((float)(startX + offset), (float)startY));
      renderer.submit(sw::Clone(menuText));

      if (isCustomRenderer) {
        auto r = std::uint8_t(((sin(frequency * i + 2 + dt) + 1.0) / 2.0) * 255U);
        auto g = std::uint8_t(((sin(frequency * i + 0 + dt) + 1.0) / 2.0) * 255U);
        auto b = std::uint8_t(((sin(frequency * i + 4 + dt) + 1.0) / 2.0) * 255U);
        renderer.submit(Light(160.0, WithZ(menuText.getPosition(), 50.0f), sf::Color(r, g, b, 255), 0.1f));
      }
    }
  }

  void onEnd() override {
    std::cout << "MainMenuScene OnEnd called" << std::endl;
  }

  ~MainMenuScene() {
    delete bgTexture;
    delete starTexture;
    delete blueButton;
    delete greenButton;
    delete redButton;

    savefile.writeToFile(SAVE_FILE_PATH);
  }
};
