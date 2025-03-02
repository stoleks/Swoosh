//This file contains the C++ entry main function and demonstrates
//using the Activity Controller with SFML's main loop
#include <Swoosh/ActivityController.h>
#include <Swoosh/Renderers/SimpleRenderer.h>
#include <Segues/ZoomOut.h>
#include <SFML/Window.hpp>
#include "Scenes/IntroScene.h"
#include "CustomRenderer.h"

int main()
{
  sf::RenderWindow window(sf::VideoMode({800, 600}), "Swoosh Demo");
  window.setFramerateLimit(60); // call this once, after creating the window
  window.setVerticalSyncEnabled(true);
  window.setMouseCursorVisible(false);

  // 11/23/2022 (NEW BEHAVIOR!)
  // Swoosh now enables custom render pipelines and
  // can switch between them in real-time
  sw::RenderEntries renderOptions;
  renderOptions
    .enroll<CustomRenderer>("custom", window.getView())
    .enroll<SimpleRenderer>("simple", window.getView());

    // Create an AC with the current window as our target to draw to
  sw::ActivityController app(window, renderOptions);

  // 10/9/2020 
  // For mobile devices or low-end GPU's, you can request optimized 
  // effects any time by setting the performance quality to
  // one of the following: { realtime, reduced, mobile }
  app.optimizeForPerformance(sw::quality::realtime); 
  // app.optimizeForPerformance(quality::mobile); // <-- uncomment me!

  // 06/12/2024
  // The AC needs a renderer in order to draw anything.
  // This was added to allow programmers to check for platform compatibilities
  // before commiting to constructing renderers which will require resources.
  // After the options are build, the programmer can iterate through the list
  // and check their SystemCompatibilityScores to determine which to use.
  app.buildRenderEntries();

  std::string errors;
  if(renderOptions.built() && renderOptions.countValid() > 0) {
    for(auto& iter : renderOptions.list()) {
      auto score = iter.getInstance().checkSystemCompatibility();
      if(score == sw::SystemCompatibilityScore::sufficient) {
        app.activateRenderEntry(iter.getIndex());
        continue;
      }
      if (score == sw::SystemCompatibilityScore::build_error) {
        errors += iter.getError() + "\n";
      }
      // For example's sake, we will free insufficient renderers
      iter.free();
    }
  } else {
    // Running this application without a renderer is pointless.
    std::cout << "Application cannot render as configured.\n";
    std::cout << "Errors: " << errors << std::endl;
    return -1; // Quit
  }


  // (DEFAULT BEHAVIOR!)
  // Add the Main Menu Scene as the first and only scene in our stack
  // This is our starting point for the user
  // app.push<MainMenuScene>(); // <-- uncomment to see a simple push

  // 10/9/2020 (NEW BEHAVIOR!)
  // Swoosh now supports generating blank activities from window contents!
  // The segue will copy the window at startup and use it as part of 
  // the screen transition as demonstrated here
  app.push<sw::segue<ZoomOut>::to<IntroScene>>();
  // app.push<MainMenuScene>(); // uncomment this and comment the line above for old behavior

  sf::Texture* cursorTexture = loadTexture(CURSOR_PATH);
  sf::Sprite cursor(*cursorTexture);

  // run the program as long as the window is open
  float elapsed = 0.0f;
  sf::Clock clock;
  bool pause = false;

  srand((unsigned int)time(0));

  while (window.isOpen())
  {
    clock.restart();

    // check all the window's events that were triggered since the last iteration of the loop
    while (const auto event = window.pollEvent ())
    {
      // "close requested" event: we close the window
      if (event->is <sf::Event::Closed> ()) {
        window.close();
      } else if (event->is <sf::Event::FocusLost> ()) {
        pause = true;
      }
      else if (event->is <sf::Event::FocusGained> ()) {
        pause = false;
      }
      else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed> ()) {
        // Toggle to different renderers using F-keys
        if (keyPressed->code == sf::Keyboard::Key::F1 && app.activateRenderEntry(0)) {
          const std::string& name = app.getActiveRenderEntry().getName();
          window.setTitle("Swoosh Demo (renderer=" + name + ")");
        }
        else if (keyPressed->code == sf::Keyboard::Key::F2 && app.activateRenderEntry(1)) {
          const std::string& name = app.getActiveRenderEntry().getName();
          window.setTitle("Swoosh Demo (renderer=" + name + ")");
        }
      }
    }

    // do not update segues when the window is frozen
    if (!pause) {
      app.update(elapsed);
    }

    // We clear after updating so that other items can copy the screen's contents
    window.clear();

    // Track the mouse and create a light source for this pass on the mouse!
    sf::Vector2f mousepos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    cursor.setPosition(mousepos);

    // draw() will directly draw onto the window's render buffer
    app.draw();

    // Draw the mouse cursor over everything else
    window.draw(cursor);

    // Display to our screen
    window.display();

    elapsed = static_cast<float>(clock.getElapsedTime().asSeconds());
  }

  delete cursorTexture;

  return 0;
}
