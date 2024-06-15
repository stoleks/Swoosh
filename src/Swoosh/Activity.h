#pragma once
#include <Swoosh/Renderers/Renderer.h>
#include <SFML/Graphics.hpp>
#include <functional>

namespace sw {
  class ActivityController; /* forward decl */

    // Forward decl.
  class PopDataHolder;

  /**
  * @class Context
  * @brief When push() later produces data via pop(...), it lives in Context.
  */
  class Context {
    friend class PopDataHolder;

    void (*deleter)(void*) { nullptr };
    void* data{ nullptr };
    std::string typenameStr;
    Context* adopted{ nullptr };

    template<typename T>
    void init(const T& copyable) {
      static void (*DeletePolicyT)(void*) =
        +[](void* data) { ((T*)data)->~T(); free(data); };

      deleter = DeletePolicyT;

      data = malloc(sizeof(T));
      T* ptr = new (data) T;
      *ptr = copyable;
      typenameStr = typeid(T).name();
    }

    void adopt(Context&& other) {
      adopted = new Context(std::move(other));
    }

  public:
    Context() = default;

    Context(const char* str) {
      init(std::string(str));
    }

    template<typename T>
    Context(const T& copyable) {
      init(copyable);
    }

    Context(Context&& rhs) noexcept {
      *this = std::move(rhs);
    }

    ~Context() {
      // Case: never initialized, abort early
      if (typenameStr.empty()) return;
      
      // free allocated memory
      if(deleter) (*deleter)(data);

      // free adopted memory
      delete adopted;
      adopted = nullptr;
    }

    Context& operator=(Context&& rhs) noexcept {
      std::swap(typenameStr, rhs.typenameStr);
      std::swap(data, rhs.data);
      std::swap(deleter, rhs.deleter);
      std::swap(adopted, rhs.adopted);

      rhs.typenameStr.clear();
      rhs.data = nullptr;
      rhs.deleter = nullptr;
      rhs.adopted = nullptr;

      return *this;
    }

    const std::string& type() const {
      return typenameStr;
    }

    template<typename T>
    const bool is() const {
      return typenameStr == typeid(T).name();
    }

    template<typename T>
    T& as() const {
      return *((T*)data);
    }

    const bool empty() const {
      return data == nullptr;
    }

    std::optional<Context> previous(size_t skip = 0) {
      Context* prev = adopted;
      while (skip-- > 0 && prev) {
        prev = prev->adopted;
      }

      if (prev == nullptr) return std::nullopt;
      return std::make_optional<Context>(std::move(*prev));
    }
  };

  /**
  * @class PopDataHolder
  * @brief A construct to handle data from popped activities
  */
  class PopDataHolder {
    friend class ActivityController;
    friend class Activity;

    using CallbackFn = std::function<void(Context&)>;
    CallbackFn callback;

    static PopDataHolder& dummy() {
      static PopDataHolder _; return _;
    }

    Context context;
    bool adopted{};

    // Default constructor
    PopDataHolder() = default;

    // No copies
    PopDataHolder(const PopDataHolder&) = delete;

    // No moves
    PopDataHolder(PopDataHolder&&) = delete;

    // Carry over context data from another PopDataHolder
    // Our context will adopt the data (own)
    void carry(PopDataHolder& from) {
      context.adopt(std::move(from.context));
    }

    void exec() {
      if (!callback) return;
      callback(context);
    }

    PopDataHolder& reset() {
      context = Context();
      callback = nullptr;
      adopted = false;
      return *this;
    }

    template<typename... Args>
    PopDataHolder& resolve(Args&&... args) {
      context = Context(std::forward<Args>(args)...);
      return *this;
    }

  public:
    void take(const CallbackFn& fn) {
      callback = fn;
    }

    void adopt() {
      adopted = true;
    }
  };


  /**
  @class Activity
  @brief An activity is an isolated screen with content drawn onto it or a unique scene in a game

  Every scene in your application must inherit Activity to work with the Swoosh library.

  An activity has 8 unique lifecycle events that can be overriden:
    - onStart , called once when this activity begins for the first time
    - onExit  , called once before this activity is deleted
    - onEnter , called when this activity is entering the view during a segue
    - onResume, called when this activity has finished entering the view after a segue
    - onLeave , called when this activity is leaving the view during a segue
    - onEnd   , called when the activity has finished leaving a view after a segue
    - onUpdate, called every tick while still in view
    - onDraw  , called every tick while still in view (*)
    
    (*) some segues may optimize and skip draw calls (see: class WhiteWashFade)
  */
  class Activity {
    friend class ActivityController;

  private:
    bool started{}; //!< Flag denotes if an activity should call onStart() or onResume()
    PopDataHolder popDataHolder; //!< Callback handle when returning

  protected:
    ActivityController* controller{ nullptr }; //!< Pointer to the activity controller
    sf::View view; //!< Custom view for this activity
    sf::Color bgColor; //!< Color to paint the background

  public:
    Activity() = delete;

    /**
      @brief constructs the activity
    */

    Activity(ActivityController* controller) : controller(controller) { started = false; }
    virtual void onStart() = 0;
    virtual void onLeave() = 0;
    virtual void onExit() = 0;
    virtual void onEnter() = 0;
    virtual void onResume() = 0;
    virtual void onEnd() = 0;
    virtual void onUpdate(double elapsed) = 0;
    virtual void onDraw(IRenderer& renderer) = 0;
    virtual ~Activity() { }
    void setView(const sf::View& view) { this->view = view; }
    void setView(const sf::Vector2u& size) { view = sf::View(sf::FloatRect(0.0f, 0.0f, (float)size.x, (float)size.y)); }
    void setView(const sf::FloatRect& rect) { view = sf::View(rect); }
    void setBGColor(const sf::Color color) { bgColor = color;  }
    const sf::View getView() const { return view; }
    const sf::Color getBGColor() const { return bgColor; }
    ActivityController& getController() { return *controller; }
  };
}
