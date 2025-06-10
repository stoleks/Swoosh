#pragma once

#include <Swoosh/Events/Events.h>

#include <SFML/Graphics.hpp>

#include <assert.h>
#include <functional>
#include <list>
#include <optional>
#include <type_traits>

using sw::events::IDispatcher;
using sw::events::ISubscriber;

namespace sw {
  class IRenderer; /* forward declare */

  // anonymous namespace helper util structs
  namespace {
    using CtorFn = std::function<IRenderer*()>;
    using DtorFn = void (*)(void*);

    namespace detail {
      template <class T, class Tuple, std::size_t... I>
      constexpr T* make_from_tuple_impl(Tuple&& t, std::index_sequence<I...>)
      {
        return new T(std::get<I>(std::forward<Tuple>(t))...);
      }
    }

    template <class T, class Tuple>
    constexpr T* make_ptr_from_tuple(Tuple&& t)
    {
      return detail::make_from_tuple_impl<T>(std::forward<Tuple>(t),
        std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
    }
  }

  /**
    @class RenderEntry
    @brief Simple aggregate that houses a render agent and its name
  */
  class RenderEntry {
  private:
    const char* name{ nullptr };
    IRenderer* ptr{ nullptr };
    size_t idx{};
    const std::string error;
    DtorFn deleter;
  public:
    RenderEntry(const char* name, IRenderer* ptr, size_t idx, const std::string& error, DtorFn deleter) 
      : name(name), ptr(ptr), idx(idx), error(error), deleter(deleter) {}
    ~RenderEntry() { free(); }

   inline std::string getName() const { return name; };
   inline const char* getNameCStr() const { return name; }
   inline IRenderer& getInstance() { return *ptr; }
   inline size_t getIndex() const { return idx; }
   inline const bool hasError() const { return error.empty() == false; }
   inline const std::string& getError() const { return error; };

   void free() {
     if (!ptr) return;
     (*deleter)(ptr);
     ptr = nullptr;
   }
  };

  /**
    @class RenderSource
    @brief Base event class type that has a reference to an SFML primitive
    @note Inherit from this class to design new event types
  */
  class RenderSource {
  private:
    const sf::Drawable* dptr{nullptr}; //!< pointer to SFML primitive
    const sf::RenderStates statesIn; //!< Copy of the render states to draw the primitive

  public:
    /**
      @brief constructs a RenderSource event from an SFML primitive and optional render states
      @example renderer.submit(RenderSource(sprite, states));
    */
    explicit RenderSource(const sf::Drawable* src, const sf::RenderStates& states = sf::RenderStates())
      : dptr(src), statesIn(states) {}

    virtual ~RenderSource() {}

    /**
      @brief Returns a pointer to the SFML primitive
    */
    const sf::Drawable* drawable() const { return dptr; }

    /**
      @brief Returns the render states info
    */
    const sf::RenderStates& states() const { return statesIn; }
  };

  /**
    @class Immediate
    @brief A lite event type used to distinguish drawables for a composite renderer pass
  */
  class Immediate : public RenderSource {
  public:
    /**
      @brief constructs an Immediate render event type
      @example renderer.submit(Immediate(&sprite, states));
    */
    Immediate(const sf::Drawable* src, const sf::RenderStates& states = sf::RenderStates()) 
      : RenderSource(src, states) {}
  };

  // internal utility structs
  namespace {
    // utility to clone event data
    template<typename T, bool isRenderEvent>
    struct usefulCopier_t {};

    // alias
    template<typename T>
    using usefulCopier = usefulCopier_t<T, std::is_base_of_v<T, RenderSource>>;
    
    // clone event data from other event types and put onto the heap
    template<typename T>
    struct usefulCopier_t<T, true> {
      static RenderSource* exec(const T& from, sf::Drawable* dptr, const char** tname) { 
        *tname = typeid(T).name(); 
        return new T(from); 
      }
    };
    
    // clone event data from SFML primitives, wrapping it in a RenderSource on the heap
    template<typename T>
    struct usefulCopier_t<T, false> {
      static RenderSource* exec(const T& from, sf::Drawable** dptr, const char** tname) { 
        *tname = typeid(RenderSource).name();
        *dptr = new T(from);
        return new RenderSource(*dptr);
      }
    };
  }

  // shorthand SFNAE for Swoosh render events
  template<typename T>
  constexpr bool is_render_event_v = std::is_base_of<RenderSource, std::remove_pointer_t<T>>::value;

  // shorthand SFNAE for SFML primitives
  template<typename T>
  constexpr bool is_sfml_primitive_v = std::is_base_of<sf::Drawable, std::remove_pointer_t<T>>::value;

  /**
    @class ClonedSource
    @brief Retains heap memory for SFML drawables and swoosh events that were cloned
    Memory is deleted at the end of the `draw()` routine in the ActivityController
  */
  struct ClonedSource : RenderSource {
    class DeletePolicy {
    public:
      virtual ~DeletePolicy() {}
      virtual void free(void*) = 0;
    };

    template<typename T>
    struct Deleter : public ClonedSource::DeletePolicy {
      void free(void* mem) override {
        T* asT = (T*)mem;
        delete asT;
      }
    };

    const char* name{ 0 }; //!< typename
    void* mem{ nullptr }; //!< type-erased memory
    sf::Drawable* dptr{ nullptr }; //!< ptr to SFML drawable (may be nullptr)
    DeletePolicy* deleter{ nullptr };

    void freeMemory() {
      delete dptr;

      if (!deleter) return;
      deleter->free(mem);
    }

    ClonedSource(void* memIn, sf::Drawable* dptr, const char* nameIn, DeletePolicy* policy = nullptr) : 
      RenderSource(dptr), name(nameIn), mem(memIn), dptr(dptr), deleter(policy) {}
  };

  /**
    @brief Clones render events and SFML primitives. Useful when re-using the same drawable variables in your scene.
    @warning T's copy constructor is invoked and the data is allocated onto the heap to be deleted for you.
    @param t The resource to make a copy of
    @return ClonedSource has a copy of `t` stored on the heap. Any underlying event will be unwrapped and re-submitted.
  */
  template<typename T>
  ClonedSource Clone(const T& t) {
    static ClonedSource::Deleter<T> deleter_type_policy;

    const char* tname {0};
    sf::Drawable* dptr {nullptr};
    RenderSource* ptr = usefulCopier<T>::exec(t, &dptr, &tname);
    return ClonedSource((void*)ptr, dptr, tname, &deleter_type_policy);
  }

  /**
  @enum SystemCompatibilityScore
  @brief A ranked enum used by implementations of IRender to inform the user
  */
  enum class SystemCompatibilityScore : uint8_t {
    build_error,      // The renderer could not test because it could not build
    insufficient,     // This system fails to meet mininum criteria to render
    unstable,         // This system meets some criteria and might render
    sufficient        // This system meets the minimum criteria to render
  };

  /**
    @class IRenderer
    @brief RenderSource event dispatcher used internally by the ActivityController to replace the old draw pipeline
  */
  class IRenderer : public IDispatcher<RenderSource> {
    friend class ActivityController;

    /**
      @brief Free any allocated memory used by the specialization of this class
    */
    virtual void flushMemory() = 0;

  public:
    virtual ~IRenderer() { }

    /**
      @brief Submits a custom render event
      @param event A custom event object to be handled by the renderer
    */
    template<typename Event, typename use = 
      std::enable_if_t<(is_render_event_v<Event> || !is_sfml_primitive_v<Event>)>>
    void submit(const Event& event) {
      IDispatcher::submit(event);
    }

    /**
      @brief This shortcut for SFML users submits any drawable as a basic render event
      @param drawable a pointer to any class that inherits from SFML drawable
      @param states RenderState info for this graphic
    */
    void submit(const sf::Drawable* drawable, const sf::RenderStates& states = sf::RenderStates()) {
      IDispatcher::submit(RenderSource(drawable, states));
    }

    /**
      @brief Implementation defined. The ActivityController draw step invokes this callback.
      @note This function must be used to compose the final texture drawn to the screen.
    */
    virtual void draw() = 0;

    /**
      @brief Implementation defined. The ActivityController draw step invokes this callback.
      @note This function must be used to clear all render surfaces before drawing to them.
    */
    virtual void clear(sf::Color color = sf::Color::Transparent) = 0;

    /**
      @brief Implementation defined. The ActivityController update step invokes this callback.
      @note This function must be used to update all render surfaces with the new screen resolution.
    */
    virtual void setView(const sf::View& view) = 0;

    /**
      @brief Return the render texture target that represents the screen's fully composed output
      @return sf::RenderTexture& a reference to the render texture target
    */
    virtual sf::RenderTexture& getRenderTextureTarget() = 0;

    /**
      @brief Allows programmer to implement evaluation of compatibility score.
      @return SystemCompatibilityScore which is cached for quick fetches.
      @note use `Renderer::getSystemCompatibilityScore()` to fetch the score.
    */
    virtual SystemCompatibilityScore checkSystemCompatibility() const = 0;

    /**
      @brief Prepares the render texture target for displaying on the screen by invoking `display()`
    */
    void display() { getRenderTextureTarget().display(); }

    /**
      @brief Creates a texture copy of the current renderer's output for the scene. 
        Useful for displaying or doing post-processing effects.
      @return sf::Texture
    */
    sf::Texture getTexture() { return getRenderTextureTarget().getTexture(); }
  };

  /**
    @class Renderer<...Ts>
    @brief All renderers must implement this class
    Given a list of render event types [Ts...], require an `onEvent(e)` function to be implemented for each
    @example class MyRenderer : Renderer<UI, Layers, Particles>{ ... }
  */
  template<typename... Ts>
  class Renderer : public IRenderer, public ISubscriber<RenderSource, Immediate, ClonedSource, Ts...> {
  private:
    std::vector<ClonedSource> clonedMem; //!< Track ClonedSource objects
    std::optional<SystemCompatibilityScore> cachedScore; // !< Last-ran score

    /**
      @brief forwards the broadcasted render event to through the ISubscriber<> implementation
    */
    void broadcast(const char* name, void* src, bool is_base) override {
      this->redirect(name, src, is_base);
    }

    /**
      @brief Built-in event handler for cloned render source events that unpacks and re-submits contained data
    */
    void onEvent(const ClonedSource& event) override {
      ClonedSource& ref = clonedMem.emplace_back(std::move(event));
      this->redirect(ref.name, ref.mem, true);
    }

    /**
    @brief Built-in event handler for immediate render events that draw directly to the 
      assigned render target at the time of call
    */
    void onEvent(const Immediate& event) override {
      getRenderTextureTarget().draw(*event.drawable(), event.states());
    }

    /**
      @brief Free's mem and dptr from cloned sources
    */
    void flushMemory() override {
      for(ClonedSource& c : clonedMem) {
        c.freeMemory();
      }
      clonedMem.clear();
    }

  public:
    /**
      @brief deconstructor gaurantees `flushMemory` function is called
    */
    virtual ~Renderer() { flushMemory(); }

    /**
      @brief Returns the cached score or runs it for the first time and caches.
      @return SystemCompatibilityScore
    */
    SystemCompatibilityScore getSystemCompatibilityScore() {
      if (cachedScore.has_value()) return *cachedScore;
      cachedScore.reset();
      checkSystemCompatibility();
    }
  };

  /**
    @class RenderEntries
    @brief short-hand list of RenderEntry records
  */
  class RenderEntries {
  private:
    friend class ActivityController;
    std::list<std::pair<CtorFn, DtorFn>> ctorDtor; // !< Deferred init
    std::list<RenderEntry> entries;                // !< Evaluated render entries
    std::list<std::string_view> pending;           // !< Name of enrolled render instances
    size_t valid{};                                // !< Counter for well-built instances
    bool ready{};                                  // !< Denotes if instances are built

    // Builds render entries from enrollment list
    void buildEntries() {
      if (ready) return;

      assert(ctorDtor.size() == pending.size()
        && "Ctors and names are out of sync!");

      auto iter = pending.begin();
      for (auto& [ctor, dtor] : ctorDtor) {
        IRenderer* ptr{ nullptr };
        std::string msg;
        try {
          ptr = ctor();
          ptr->checkSystemCompatibility();
          valid++;
        }
        catch (const std::runtime_error& err) {
          msg = err.what();
        }
        entries.emplace_back(iter->data(), ptr, entries.size(), msg, dtor);
        iter = std::next(iter);
      }

      // Entries are built, evaluated, and ready to be used
      ready = true;
    }
  public:
    // Registers renderers, their ctor arguments, and deffers construction
    template<typename T, typename... Args>
    RenderEntries& enroll(const std::string_view& name, Args&&...args) {
      assert(!ready && "Cannot enroll new renderers after they are built!");

      auto ctor = [t = std::make_tuple(std::forward<decltype(args)>(args)...)]() {
        return make_ptr_from_tuple<T>(t);
      };

      static DtorFn dtor = +[](void* data) {
        delete reinterpret_cast<T*>(data);
      };

      ctorDtor.push_back({ ctor, dtor });

      pending.emplace_back(name);

      return *this;
    }

    const bool built() const { return ready; }
    const size_t count() const { return pending.size(); }
    const size_t countValid() const { return valid; }
    const std::list<RenderEntry>& list() const { return entries; }

    // non const
    std::list<RenderEntry>& list() { return entries; }
  };

}
