#ifndef TEST_ARITH_OPS_HPP
#define TEST_ARITH_OPS_HPP

#include <vector>
#include <map>
#include <cassert>
#include <boost/noncopyable.hpp>
#include <iostream>

#if defined(CPP11)
#   define NOEXCEPT noexcept
#   define CONSTEXPR constexpr
#   define CPP11ONLY(x) x
#else
#   define NOEXCEPT
#   define CONSTEXPR
#   define CPP11ONLY(x)
#endif

struct Resource : private boost::noncopyable
{
    typedef std::size_t handle_t;
    static Resource& instance() {
        static Resource i;
        return i;
    }
    static handle_t allocate() { return instance().get();}
    static handle_t null() { return handle_t() ;}
    static void release(handle_t h) NOEXCEPT {}
    static handle_t current() NOEXCEPT { return instance().m_handle; }

private:
    handle_t get() { return ++m_handle; }
    handle_t m_handle
#if defined(CPP1)
        = 0
#endif
        ;

#if 0
    Resource() = default;
    ~Resource() = default;
#else
    Resource() : m_handle(0) {}
    ~Resource() {}
#endif
};

#if defined(CPP11)
enum class event {
    default_construct, copy_construct, move_construct, copy_assign, move_assign, destruct
        , MAX__
};
typedef event event_t;
#else
namespace event {
    enum type {
        default_construct, copy_construct, move_construct, copy_assign, move_assign, destruct
            , MAX__
    };
}
typedef event::type event_t;
#endif

CONSTEXPR char const* events[] = {
    "dflt_constr", "copy_constr", "move_constr", "copy_assign", "move_assign", "destruction"
};

#if defined(CPP11)
template <typename T, std::size_t N>
constexpr std::size_t size(T (&)[N]) { return N;}
static_assert(size(events) == std::size_t(event::MAX__), "Mismatching enumeration");
#endif

CONSTEXPR char const* to_string(event_t ev) {
    return events[std::size_t(ev)];
}

struct Observable;
struct Observer
{
    // template <typename Observed, typename... EventData>
        // void notify(Observed const& subject, EventData&&... data)
    void notify(Observable const& subject, event_t ev, Resource::handle_t h)
    {
        if (m_listening) {
            std::cout << std::string(m_indent, ' ') << to_string(ev) << " 0x" << &subject << " owner of " << h << "\n";
            ++m_events[ev];
        }
    }

    void start() {
        m_listening = true; inc_indent();
        m_resources_at_begining = Resource::current();
    }
    void stop() {
        m_listening = false; dec_indent();
#if defined(CPP11)
        for (auto&& ev : m_events) {
            std::cout << std::string(m_indent, ' ')
                << ev.second << " " << to_string(ev.first) << "\n";
        }
#else
        for (Events_t::const_iterator b = m_events.begin(), e = m_events.end()
                ; b != e
                ; ++b
            )
        {
            Events_t::value_type const& ev = *b;
            std::cout << std::string(m_indent, ' ')
                << ev.second << " " << to_string(ev.first) << "\n";
        }
#endif
        std::cout << std::string(m_indent, ' ')
            << Resource::current() - m_resources_at_begining
            << " allocations\n";
        m_events.clear();
    }

#if defined(CPP11)
    Observer() = default;
    ~Observer() = default;
    Observer& operator=(Observer const&) = delete;
    Observer(Observer const&) = delete;
    Observer(Observer&&) = default;
    Observer& operator=(Observer &&) = delete;
#else
    Observer()
        : m_indent(0)
        , m_listening(false)
        , m_resources_at_begining(0)
        {}
#endif

private:
    typedef std::map<event_t, std::size_t> Events_t;
    void inc_indent() { m_indent += 2; }
    void dec_indent() { m_indent -= 2; }
    std::size_t        m_indent                CPP11ONLY(= 0);
    bool               m_listening             CPP11ONLY(= false);
    Resource::handle_t m_resources_at_begining CPP11ONLY(= 0);
    Events_t           m_events;
};

struct Focus : private boost::noncopyable
{
    Focus(Observer & obs, std::string const& text) : m_observer(obs) {
        std::cout << "----------[ " << text << "\n";
        m_observer.start();
    }
    ~Focus() { m_observer.stop(); }
private:
    Observer & m_observer;
};

struct Observable
{
protected:
    void register_observer(Observer & o) {
        m_observers.push_back(&o);
    }
#if 0
    template <typename... EventData> void notify(EventData&&... data) {
        for (auto&& obs : m_observers) {
            assert(obs);
            obs->notify(*this, std::forward<EventData>(data)...);
        }
    }
#else
    void notify(event_t ev, Resource::handle_t h) {
        for (std::vector<Observer *>::const_iterator b = m_observers.begin(), e = m_observers.end()
                ; b != e
                ; ++b
            )
        {
            assert(*b);
            (*b)->notify(*this, ev, h);
        }
    }
#endif
private:
    std::vector<Observer *> m_observers;
};



struct Vector : private Observable
{
    Vector(Observer & o)
        : m_resource(Resource::allocate())
    {
        register_observer(o);
        notify(event::default_construct, m_resource);
    }

    ~Vector() {
        notify(event::destruct, m_resource);
        Resource::release(m_resource);
    }

    Vector(Vector const& rhs)
        : Observable(rhs)
        , m_resource(Resource::allocate())
    {
        // m_resource content <- copy of rhs m_resource content
        notify(event::copy_construct, m_resource);
    }

#if defined(CPP11)
    Vector(Vector && rhs) noexcept
        : Observable(rhs)
        , m_resource(rhs.m_resource)
        {
            rhs.m_resource = Resource::null();
            notify(event::move_construct, m_resource);
        }
#endif

    Vector& operator=(Vector const& rhs) {
        // suppose we are reusing if possible ...
        if (! m_resource) {
            m_resource = Resource::allocate();
        }
        // m_resource content <- copy of rhs m_resource content

        notify(event::copy_assign, m_resource);
        return *this;
    }

#if defined(CPP11)
    Vector& operator=(Vector && rhs) noexcept {
        Resource::release(m_resource);
        m_resource = rhs.m_resource;
        rhs.m_resource = Resource::null();
        notify(event::move_assign, m_resource);
        return *this;
    }
#endif

    Vector& operator+=(Vector const& rhs) {return *this;}
private:
    Resource::handle_t m_resource;
};



#endif // TEST_ARITH_OPS_HPP
