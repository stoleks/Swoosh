#pragma once
#include <SFML/Graphics.hpp>
#include <Swoosh/Utils.h>

// Custom class definitions
struct button;
const bool isMouseHovering(button& btn, sf::RenderWindow& window);

struct button {
  sf::Sprite sprite;
  std::string text;

  bool isClicked;
  bool isHovering;

  button(sf::Texture& texture) : sprite(texture) { isClicked = isHovering = false;  }
  void update(sf::RenderWindow& window) {
    if (isMouseHovering(*this, window)) {
      isHovering = true;
      isClicked = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    }
    else {
      isClicked = isHovering = false;
    }
  }

  void draw(IRenderer& renderer, sf::Text& sftext, float x, float y) {
    if (isHovering) {
      sprite.setColor(sf::Color(200, 200, 200));
    }
    else {
      sprite.setColor(sf::Color::White);
    }

    sprite.setPosition(sf::Vector2f(x, y));
    sprite.setOrigin({sprite.getGlobalBounds().size.x / 2.0f, sprite.getGlobalBounds().size.y / 2.0f});
    renderer.submit(Clone(sprite));

    sftext.setString(text);
    sftext.setOrigin({sftext.getGlobalBounds().size.x / 2.0f, sftext.getGlobalBounds().size.y / 2.0f});
    sftext.setPosition(sf::Vector2f(x, y - sftext.getGlobalBounds().size.y / 2.0f));
    renderer.submit(Clone(sftext));
  }
};

// Custom class definitions
const bool isMouseHovering(button& btn, sf::RenderWindow& window) {
  sf::Sprite& sprite = btn.sprite;
  sf::Vector2i mousei = sf::Mouse::getPosition(window);
  sf::Vector2f mouse = window.mapPixelToCoords(mousei);
  sf::FloatRect bounds = sprite.getGlobalBounds();

  return bounds.contains (mouse);
}
