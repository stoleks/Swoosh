#pragma once

#include <Swoosh/Renderers/Renderer.h>

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>

namespace sw {
  class ActivityController; /* forward decl */

    // Forward decl.
  class PopDataHolder;
  class Context;

  namespace Impl {
    // FOR INTERNAL USE ONLY.
    // 
    // This utility class is used to manage the data stored in the Context.
    // Data stored must be copyable otherwise the compiler will abort.
    // 
    // The bucket remembers its type and can dissolve single type values 
    // into their immediate return type via `T& read()`. 
    // 
    // Multi values are returned as `std::tuple<Ts...>& read()`.
    //
    // The data in Bucket cleans up after itself.
    class Bucket {
      friend class sw::Context;

      void (*deleter)(void*) { nullptr };
      void* data{ nullptr };
      std::string underliningTypename;

      //
      // Private memory management methods
      //

      template<typename UnderliningType>
      void copy(const std::optional<UnderliningType>& option) {
        if (!option.has_value()) return;
        copy(*option);
      }

      template<typename UnderliningType>
      void copy(const UnderliningType& copyable) {
        static void (*DeletePolicyPtr)(void*) =
          +[](void* data){ 
          ((UnderliningType*)data)->~UnderliningType(); 
          free(data);
        };

        // sanity check
        cleanup();

        deleter = DeletePolicyPtr;
        data = malloc(sizeof(UnderliningType));

        UnderliningType* ptr = new (data) UnderliningType;
        *ptr = copyable;
        underliningTypename = typeid(UnderliningType).name();
      }

      // Initialize data with value T or a tuple<T, Ts...>
      // Iff param is one optional<T> whose value is nullopt, then noop
      // Iff param is one optional<T> with a value, then the value is extracted
      template<typename T, typename...Ts>
      void init(T&& t, Ts&&...ts) {
        if constexpr (sizeof...(Ts) == 0) {
          static_assert(
            std::is_copy_constructible_v<T>,
            "Popping with userdata requires copying"
            );

          copy(t);
        }
        else {
          static_assert(
            std::is_copy_constructible_v<T>
            && (std::is_copy_constructible_v<Ts>, ...),
            "Popping with userdata requires copying"
            );
          copy(std::tuple{ t, ts... });
        }
      }


      void cleanup() {
        // free allocated memory
        if (deleter) (*deleter)(data);
        data = nullptr;
      }

      bool empty() const {
        return data == nullptr;
      }

      // Checks the underlining type
      template<typename T, typename... Ts>
      bool has() const {
        if constexpr (sizeof...(Ts) == 0) {
          return underliningTypename == typeid(T).name();
        }
        else {
          return underliningTypename == typeid(std::tuple<T, Ts...>).name();
        }
      }

      // Returns reference to the underlining object or the tuple
      template<typename T, typename... Ts>
      auto read() -> decltype(auto) {
        if constexpr (sizeof...(Ts) == 0) {
          return *((T*)data);
        }
        else {
          return *((std::tuple<T, Ts...>*)data);
        }
      }

      const std::string& type() const {
        return underliningTypename;
      }

      //
      // public
      //

    public:
      Bucket() = default;
      Bucket(Bucket&& rhs) noexcept {
        *this = std::move(rhs);
      }

      Bucket& operator=(Bucket&& rhs) noexcept {
        std::swap(underliningTypename, rhs.underliningTypename);
        std::swap(data, rhs.data);
        std::swap(deleter, rhs.deleter);
        rhs.underliningTypename.clear();
        rhs.data = nullptr;
        rhs.deleter = nullptr;
        return *this;
      }

      ~Bucket() {
        cleanup();
      }
    };
  } // namespace Impl

  /**
  * @class Context
  * @brief If a push() later produces data via pop(...), it lives in Context.
  */
  class Context {
    friend class PopDataHolder;
    Context* adopted{ nullptr };
    Impl::Bucket mem{};

    void adopt(Context&& other) {
      adopted = new Context(std::move(other));
    }

  public:
    Context() = default;

    // Raw str specialization
    Context(const char* str) {
      mem.init(std::string(str));
    }

    template <typename... Ts>
    Context(Ts&&...ts) {
      mem.init(std::forward<Ts>(ts)...);
    }

    Context(Context&& rhs) noexcept {
      *this = std::move(rhs);
    }

    ~Context() {
      // free adopted memory
      delete adopted;
      adopted = nullptr;

      // Bucket::~Bucket() will invoke
    }

    Context& operator=(Context&& rhs) noexcept {
      std::swap(mem, rhs.mem);
      std::swap(adopted, rhs.adopted);
      rhs.adopted = nullptr;
      return *this;
    }

    template <typename... Ts>
    const bool has() const {
      return mem.has<Ts...>();
    }

    template <typename... Ts>
    auto read() -> decltype(auto) {
      return mem.read<Ts...>();
    }

    const bool empty() const {
      return mem.empty();
    }

    const std::string& type() const {
      return mem.type();
    }

    std::optional<Context> previous(size_t count = 0) {
      Context* prev = adopted;
      while (count-- > 0 && prev) {
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
    void setView(const sf::Vector2u& size) { view = sf::View (sf::FloatRect ({0.0f, 0.0f}, sf::Vector2f (size))); }
    void setView(const sf::FloatRect& rect) { view = sf::View (rect); }
    void setBGColor(const sf::Color color) { bgColor = color;  }
    const sf::View getView() const { return view; }
    const sf::Color getBGColor() const { return bgColor; }
    ActivityController& getController() { return *controller; }
  };
}
