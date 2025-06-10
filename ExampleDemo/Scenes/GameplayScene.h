#pragma once

#include "../CustomRenderer.h"
#include "../TextureLoader.h"
#include "../Particle.h"
#include "../ResourcePaths.h"
#include "HiScoreScene.h"

#include <Segues/Checkerboard.h>
#include <Swoosh/ActivityController.h>
#include <Swoosh/Utils.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <assert.h>

class HiScoreScene;

class GameplayScene : public sw::Activity {
private:
  sf::Texture* btn;
  sf::Texture* bgTexture, * bgNormal, * bgEmissive;
  std::unique_ptr <sf::Sprite> bg;

  sf::Texture* playerTexture;
  sf::Texture* playerNormal;
  sf::Texture* playerEsm;
  particle player;

  sf::Texture* trailTexture;
  std::vector<particle> trails;

  sf::Texture* enemyTexture;
  sf::Texture* enemyNormal;
  sf::Texture* enemyEsm;
  std::vector<particle> enemies;

  sf::Texture * meteorBigN, *meteorMedN, *meteorSmallN, *meteorTinyN;
  sf::Texture* meteorBig, * meteorMed, * meteorSmall, * meteorTiny;
  std::vector<particle> meteors;

  sf::Texture *laserTexture;
  std::vector<particle> lasers;

  sf::Texture * shieldTexture;
  std::unique_ptr <sf::Sprite> shield;

  sf::Texture * extraLifeTexture;
  std::unique_ptr <sf::Sprite> star;

  sf::Texture * numeralTexture[11];
  sf::Texture * playerLifeTexture;
  std::unique_ptr <sf::Sprite> numeral;
  std::unique_ptr <sf::Sprite> playerLife;

  sf::Font   font;
  sf::Text   text;

  sf::SoundBuffer laserFX;
  sf::SoundBuffer shieldFX;
  sf::SoundBuffer gameOverFX;
  sf::SoundBuffer extraLifeFX;
  sf::SoundBuffer explodeFX;
  sf::Sound laserChannel;
  sf::Sound shieldChannel;
  sf::Sound explodeChannel;
  sf::Sound extraLifeChannel;
  sf::Sound gameOverChannel;
  sf::Music ingameMusic;

  int lives;
  long score;
  double alpha;
  bool hasShield;
  bool isExtraLifeSpawned;

  bool mousePressed;
  bool mouseRelease;
  bool inFocus;

  SaveFile& savefile;
public:
  GameplayScene(sw::ActivityController& controller, SaveFile& savefile) 
  : Activity(&controller),
    text (font),
    laserChannel (laserFX),
    shieldChannel (shieldFX),
    explodeChannel (explodeFX),
    extraLifeChannel (extraLifeFX),
    gameOverChannel (gameOverFX),
    savefile (savefile)
  { 
    mousePressed = mouseRelease = inFocus = isExtraLifeSpawned = false;

    if (!ingameMusic.openFromFile(INGAME_MUSIC_PATH)) {}
    ingameMusic.setLooping(true);
    ingameMusic.setLoopPoints({sf::seconds(0), sf::seconds(49)});

    if (!laserFX.loadFromFile(LASER1_SFX_PATH)) {}
    if (!shieldFX.loadFromFile(SHIELD_DOWN_SFX_PATH)) {}
    if (!gameOverFX.loadFromFile(LOSE_SFX_PATH)) {}
    if (!extraLifeFX.loadFromFile(TWO_TONE_SFX_PATH)) {}
    if (!explodeFX.loadFromFile(EXPLODE_SFX_PATH)) {}

    laserChannel.setBuffer(laserFX);
    shieldChannel.setBuffer(shieldFX);
    gameOverChannel.setBuffer(gameOverFX);
    extraLifeChannel.setBuffer(extraLifeFX);
    explodeChannel.setBuffer(explodeFX);

    sf::Vector2u windowSize = getController().getVirtualWindowSize();
    setView(windowSize);

    bgTexture = loadTexture(GAME_BG_PATH);
    bgTexture->setRepeated(true);
    bgNormal = loadTexture(GAME_BG_N_PATH);
    bgNormal->setRepeated(true);
    bgEmissive = loadTexture(GAME_BG_E_PATH);
    bgEmissive->setRepeated(true);
    bg = std::make_unique <sf::Sprite> (*bgTexture);
    bg->setTextureRect(sf::IntRect ({0, 0}, {(int)windowSize.x, (int)windowSize.y}));

    meteorBig = loadTexture(METEOR_BIG_PATH);
    meteorMed = loadTexture(METEOR_MED_PATH);
    meteorSmall = loadTexture(METEOR_SMALL_PATH);
    meteorTiny = loadTexture(METEOR_TINY_PATH);

    meteorBigN = loadTexture(METEOR_BIG_N_PATH);
    meteorMedN = loadTexture(METEOR_MED_N_PATH);
    meteorSmallN = loadTexture(METEOR_SMALL_N_PATH);
    meteorTinyN = loadTexture(METEOR_TINY_N_PATH);

    laserTexture = loadTexture(LASER_BEAM_PATH);
    shieldTexture = loadTexture(SHIELD_LOW_PATH);
    enemyTexture = loadTexture(ENEMY_PATH);
    enemyNormal = loadTexture(ENEMY_N_PATH);
    enemyEsm = loadTexture(ENEMY_E_PATH);

    extraLifeTexture = loadTexture(EXTRA_LIFE_PATH);
    star = std::make_unique <sf::Sprite> (*extraLifeTexture);
    sw::setOrigin(*star, 0.5, 0.5);

    playerTexture = loadTexture(PLAYER_PATH);
    playerNormal = loadTexture(PLAYER_N_PATH);
    playerEsm = loadTexture(PLAYER_E_PATH);
    player.sprite = std::make_unique <sf::Sprite> (*playerTexture);
    sw::setOrigin(*player.sprite, 0.5, 0.5);

    trailTexture = loadTexture(PLAYER_TRAIL_PATH);

    shield = std::make_unique <sf::Sprite> (*shieldTexture);
    sw::setOrigin(*shield, 0.5, 0.5);

    for (int i = 0; i < 11; i++) {
      numeralTexture[i] = loadTexture(NUMERAL_PATH[i]);
    }
    numeral = std::make_unique <sf::Sprite> (*numeralTexture[10]); // X

    playerLifeTexture = loadTexture(PLAYER_LIFE_PATH);
    playerLife = std::make_unique <sf::Sprite> (*playerLifeTexture);

    resetPlayer();
    alpha = 255.0; // resetPlayer() sets player alpha to 0, prevent that on first boot

    if (!font.openFromFile(GAME_FONT)) {}
    text.setFont(font);

    text.setFillColor(sf::Color::White); 

    lives = 3;
    score = 0;
  }

  void spawnEnemy() {
    particle enemy;
    enemy.sprite = std::make_unique <sf::Sprite> (*enemyTexture);
    sw::setOrigin(*enemy.sprite, 0.5, 0.5);

    sf::Vector2u windowSize = getController().getVirtualWindowSize();

    int side = rand() % 2;
    if(side == 0)
      enemy.pos = sf::Vector2f((float)((rand() % 2) * windowSize.x), (float)(rand() % windowSize.y));
    else 
      enemy.pos = sf::Vector2f((float)(rand() % windowSize.x), (float)((rand() % 2) * windowSize.y));

    enemy.lifetime = 0; // this enemy stays alive until a lifetime is provided
    enemy.sprite->setPosition(enemy.pos);
    enemies.push_back(enemy);
  }

  void resetPlayer() {
    // start is in the center of the screen
    sf::Vector2u windowSize = getController().getVirtualWindowSize();

    player.pos = sf::Vector2f(windowSize.x / 2.0f, windowSize.y / 2.0f);
    player.speed = sf::Vector2f(0, 0);
    player.sprite->setPosition(player.pos);
    player.friction = sf::Vector2f(0.96f, 0.96f);
    sw::setOrigin(*player.sprite, 0.5, 0.5);
    alpha = 0;
    hasShield = true;
  }

  void onStart() override {
    std::cout << "DemoScene OnStart called" << std::endl;
    ingameMusic.play();
    inFocus = true;
  }

  void onUpdate(double elapsed) override {
    auto& C = getController();
    sf::RenderWindow& window = C.getWindow();
    auto windowSize = C.getVirtualWindowSize();

    // End the game if the player is out of lives OR escape key is pressed
    if (lives < 0 || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
      // Some segues can be customized like Checkerboard effect
      using custom = CheckerboardCustom<40, 40>;
      using tx = segue<custom, arg::milli<900>>;

      // NOTE: This will never be called from HiScoreScene because
      // that screen _rewinds_ back to the title scene.
      // This code is left here to demonstrate that behavior.
      // In action, you will never see this function called.
      auto onReturn = [this](const sw::Context& context) {
        std::cout << "GamePlayScene popped with data: " << context.type() << std::endl;
      };

      C.push <tx::to <HiScoreScene>> (savefile).take(onReturn);
    }

    for (auto& m : meteors) {
      m.pos += sf::Vector2f(m.speed.x * (float)elapsed, m.speed.y * (float)elapsed);
      m.sprite->setPosition(m.pos);
      m.sprite->setRotation(sf::degrees (m.pos.x));

      const sf::Vector2u screenSize = C.getVirtualWindowSize();
      if (m.pos.x > screenSize.x + 100) {
        m.pos.x = -50.0f;
      } else if (m.pos.x < -100) {
        m.pos.x = (float)screenSize.x + 50.f;
      }

      if (m.pos.y > (float)screenSize.y + 100) {
        m.pos.y = -50.0f;
      }
      else if (m.pos.y < -100) {
        m.pos.y = (float)screenSize.y + 50.0f;
      }
    }

    int i = 0;
    for (auto& t : trails) {
      if (t.sprite == nullptr) {
        i++;
        continue;
      }
      t.sprite->setPosition(t.pos);
      t.sprite->setScale({(float)(t.life / t.lifetime), (float)(t.life / t.lifetime)});
      t.sprite->setColor(sf::Color(
        t.sprite->getColor().r, 
        t.sprite->getColor().g, 
        t.sprite->getColor().b, 
        (std::uint8_t)(10.0f * (t.life / t.lifetime))
      ));
      t.life -= elapsed;

      if (t.life <= 0) {
        i++;
        trails.erase(trails.begin() + i);
        continue;
      }

      i++;
    }

    i = 0;
    bool killShield = false;
    for (auto& e : enemies) {
      if (e.sprite == nullptr) {
        i++;
        continue;
      }
      if (e.lifetime == 0) {
        for (auto& l : lasers) {
          if (l.sprite == nullptr) break;
          if (e.lifetime != 0) break; // Reward player once
          if (sw::doesCollide(*l.sprite, *e.sprite)) {
            l.life = 0;
            e.lifetime = 1.0; // trigger scale out
            score += 1000;
            explodeChannel.play();
          }
        }
      }

      if (e.life <= 0) {
        enemies.erase(enemies.begin() + i);
        i++;
        continue;
      }

      if (lives >= 0 && e.lifetime == 0) {
        if (alpha >= 255.0 && sw::doesCollide(*e.sprite, *player.sprite)) {
          if (hasShield && !killShield) {
            shieldChannel.play();
            killShield = true; // give us time to protect from other enemies
            alpha = 100; // invincibility 
          }
          else {
            resetPlayer();
            gameOverChannel.play();
            lives--;

            for (auto& e2 : enemies) {
              e2.lifetime = 1.0; // trigger scale out on ALL enemies
            }
          }

          e.lifetime = 1.0; // trigger scale out on this enemy
        }

        double angle = sw::angleTo(player.pos, e.pos);

        if (e.sprite != nullptr) {
          e.sprite->setRotation(sf::degrees (90.0f + (float)angle));
          e.sprite->setPosition(e.pos);
        }

        sf::Vector2f dir = sw::directionTo<float>(player.pos, e.pos);
        sf::Vector2f delta;
        delta.x = dir.x * 2.0f;
        delta.y = dir.y * 2.0f;
        e.speed += delta;
      }

      if (e.lifetime > 0) {
        e.life -= 2*elapsed;
        auto scale = (float)(e.life / e.lifetime);
        if (e.sprite != nullptr) {
          e.sprite->setScale({scale, scale});
        }
        e.speed.x = e.speed.y = 0;
      } 

      e.pos += sf::Vector2f(e.speed.x * (float)elapsed, e.speed.y * (float)elapsed);
      if (e.sprite != nullptr) {
        e.sprite->setPosition(e.pos);
      }

      i++;
    }

    i = 0;
    for (auto& l : lasers) {
      l.pos.x += l.speed.x * (float)elapsed;
      l.pos.y += l.speed.y * (float)elapsed;
      if (l.sprite != nullptr) {
        l.sprite->setPosition(l.pos);
      }
      l.life -= elapsed;

      if (l.life <= 0) {
        lasers.erase(lasers.begin() + i);
        i++;
        continue;
      }

      i++;
    }

    if (rand() % 50 == 0 && enemies.size() < 20 && inFocus) {
      spawnEnemy();

      if (rand() % 30 == 0 && !isExtraLifeSpawned) {
        isExtraLifeSpawned = true;

        star->setPosition({(float)(rand() % windowSize.x), (float)(rand() % windowSize.y)});

        // do not spawn on top of player
        while (sw::doesCollide(*star, *player.sprite)) {
          star->setPosition({(float)(rand() % windowSize.x), (float)(rand() % windowSize.y)});
        }
      }
    }

    if (lives < 0) return; // do not update player logic 

    if (isExtraLifeSpawned) {
      if (sw::doesCollide(*star, *player.sprite)) {
        isExtraLifeSpawned = false;
        extraLifeChannel.play();
        lives = std::min(lives+1, 9);
        score += 250;
      }
    }

    sf::Vector2f mousepos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    double angle = sw::angleTo(mousepos, player.pos);

    player.sprite->setRotation(sf::degrees (90.0f + (float)angle));

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
      sf::Vector2f dir = sw::directionTo<float>(mousepos, player.pos);
      sf::Vector2f delta = player.speed;
      delta.x += dir.x * 30.0f * (float)elapsed;
      delta.y += dir.y * 30.0f * (float)elapsed;

      player.speed = delta;

      particle trail = player;
      trail.sprite = std::make_unique <sf::Sprite> (*trailTexture);
      sw::setOrigin(*trail.sprite, 0.5, 0.5);
      trail.life = trail.lifetime = 1.0; // secs
      trails.insert(trails.begin(), trail);
    }
    else {
      // apply the brakes
      player.speed.x *= player.friction.x;
      player.speed.y *= player.friction.y;
    }

    player.speed.x = std::min(20.f, player.speed.x);
    player.speed.x = std::max(-20.f, player.speed.x);
    player.speed.y = std::min(20.f, player.speed.y);
    player.speed.y = std::max(-20.f, player.speed.y);

    player.pos += player.speed;

    player.pos.x = std::min(player.pos.x, (float)windowSize.x);
    player.pos.x = std::max(0.0f, player.pos.x);
    player.pos.y = std::min(player.pos.y, (float)windowSize.y);
    player.pos.y = std::max(0.0f, player.pos.y);

    player.sprite->setPosition(player.pos);

    alpha += 50 * elapsed;

    alpha = std::min(alpha, 255.0);
    player.sprite->setColor(sf::Color(255, 255, 255, (std::uint8_t)alpha));

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && inFocus) {
      if (!mousePressed) {
        particle laser;
        laser.sprite = std::make_unique <sf::Sprite> (*laserTexture);
        sw::setOrigin (*laser.sprite, 0.5, 0.5);
        laser.pos = player.pos;
        laser.sprite->setRotation(sf::degrees (90.0f + (float)angle));
        laser.sprite->setPosition(laser.pos);

        sf::Vector2f dir = sw::directionTo<float>(mousepos, laser.pos);
        sf::Vector2f delta;
        delta.x = dir.x * 500.0f;
        delta.y = dir.y * 500.0f;
        laser.speed = delta;

        lasers.push_back(laser);

        laserChannel.play();

        mousePressed = true;
      }
    }
    else {
      mousePressed = false;
    }

    if(hasShield && killShield)
      hasShield = false;

    // Left here as an example on how safe it is to clear the stack anywhere:
    // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num0)) {
    //   C.clearStackSafely();
    // }
  }

  void onLeave() override {
    std::cout << "DemoScene OnLeave called" << std::endl;
    ingameMusic.stop();
    inFocus = false;
  }

  void onExit() override {
    std::cout << "DemoScene OnExit called" << std::endl;

    savefile.names.insert(savefile.names.begin(), "YOU");
    savefile.scores.insert(savefile.scores.begin(), score);
    savefile.writeToFile(SAVE_FILE_PATH);
  }

  void onEnter() override {
    auto& C = getController();

    std::cout << "DemoScene OnEnter called" << std::endl;

    for (int i = 50; i > 0; i--) {
      int randNegativeX = rand() % 2 == 0 ? -1 : 1;
      int randNegativeY = rand() % 2 == 0 ? -1 : 1;

      int randSpeedX = rand() % 40;
      randSpeedX *= randNegativeX;

      int randSpeedY = rand() % 40;
      randSpeedY *= randNegativeY;

      particle p;

      int randTexture = rand() % 4;

      switch (randTexture) {
      case 0:
        p.sprite = std::make_unique <sf::Sprite> (*meteorBig);
        break;
      case 1:
        p.sprite = std::make_unique <sf::Sprite> (*meteorMed);
        break;
      case 2:
        p.sprite = std::make_unique <sf::Sprite> (*meteorSmall);
        break;
      default:
        p.sprite = std::make_unique <sf::Sprite> (*meteorTiny);
      }

      auto windowSize = C.getVirtualWindowSize();
      p.pos = sf::Vector2f((float)(rand() % windowSize.x), (float)(rand() % windowSize.y));
      p.sprite->setPosition(p.pos);
      p.sprite->setRotation(sf::degrees (p.pos.x));

      p.speed = sf::Vector2f((float)randSpeedX, (float)randSpeedY);
      meteors.push_back(p);
    }
  }

  void onResume() override {
    std::cout << "DemoScene OnResume called" << std::endl;

  }

  void onDraw(sw::IRenderer& renderer) override {
    auto& C = getController();
    const bool isCustomRenderer = C.getActiveRenderEntry().getName() == "custom";
    sf::RenderWindow& window = C.getWindow();
    auto windowSize = C.getVirtualWindowSize();

    // Track the mouse and create a light source for this pass on the mouse!
    sf::Vector2f mousepos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    // We can filter what we submit to the renderer by checking the current renderer's name or ID
    if (isCustomRenderer) {
      // Draw a light tracking the cursor
      renderer.submit (Light(256.0f, WithZ(mousepos, 10.f), sf::Color(100U, 100U, 150U), 1.0));

      // Draw a light in the scene so we can see everything
      sf::Vector2f center = sf::Vector2f (windowSize) / 2.0f;
      renderer.submit (Light(1000.0f, WithZ(center, 100.0f), sf::Color(255U, 205U, 255U)));
    }

    renderer.submit(Draw3D(bg.get (), bgNormal).WithZ(-100));

    for (auto& t : trails) {
      renderer.submit(Draw3D(t.sprite.get (), nullptr, trailTexture));
    }

    for (auto& m : meteors) {
      if (m.sprite == nullptr) continue;
      sf::Texture* normal = meteorTinyN;
      const sf::Texture* spriteTexture = &m.sprite->getTexture();
      if (spriteTexture == meteorBig) {
        normal = meteorBigN;
      } else if (spriteTexture == meteorMed) {
        normal = meteorMedN;
      }
      else if (spriteTexture == meteorSmall) {
        normal = meteorSmallN;
      }
      // else - handled by default value of `normal`
      renderer.submit(Draw3D(m.sprite.get (), normal).WithZ(-50));
    }

    for (auto& e : enemies) {
      if (e.sprite == nullptr) continue;
      renderer.submit(Draw3D(e.sprite.get (), enemyNormal, enemyEsm));

      if (e.lifetime > 0) {
        const auto alpha = std::max(0.f, (float)(e.life / e.lifetime));
        const auto beta = 1.0f - alpha;
        const auto ch = std::uint8_t (alpha*255);
        renderer.submit(Light(
          100.0f + (200.0f*beta),
          WithZ(e.sprite->getPosition(), 100.0f), 
          sf::Color(255, ch, 0, ch), 10.0f, 0.5f)
        );
      }
    }

    for (auto& l : lasers) {
      if (l.sprite == nullptr) continue;
      renderer.submit(Draw3D(l.sprite.get (), nullptr, laserTexture).WithZ(-1.0f));

      if (isCustomRenderer) {
        renderer.submit(Light(
          100.0, 
          WithZ(l.sprite->getPosition(), 9.0f), 
          sf::Color(0, 215, 0, 255), 20.0f)
        );
      }
    }
    
    text.setString(std::string("score: ") + std::to_string(score));
    sw::setOrigin(text, 1, 0);
    text.setPosition(sf::Vector2f((float)windowSize.x - 50.0f, 0.0f));

    if (alpha < 255) {
      text.setFillColor(sf::Color::Red);
    }
    else {
      text.setFillColor(sf::Color::White);
    }

    renderer.submit(&text);

    if (isExtraLifeSpawned) renderer.submit(star.get ());


    if (lives >= 0) {
      renderer.submit(Draw3D(player.sprite.get (), playerNormal, playerEsm, 0.5f));

      if (hasShield) {
        shield->setPosition(player.pos);
        shield->setRotation(player.sprite->getRotation());
        renderer.submit(shield.get ());
      }

      numeral->setTexture (*numeralTexture[10]); // X
      numeral->setPosition({player.pos.x, player.pos.y - 100});

      // NOTE: `numeral` sprite is re-used so we must clone before submitting!
      renderer.submit(sw::Clone(*numeral));

      numeral->setTexture (*numeralTexture[lives]);
      numeral->setPosition({player.pos.x + 20, player.pos.y - 100});

      // NOTE: The original `numeral` will not affect the clone and vice-versa
      renderer.submit(numeral.get ());

      playerLife->setPosition({player.pos.x - 40, player.pos.y - 100});
      renderer.submit(playerLife.get ());
    }
  }

  void onEnd() override {
    std::cout << "DemoScene OnEnd called" << std::endl;
  }

  ~GameplayScene() { delete bgTexture;; }
};
