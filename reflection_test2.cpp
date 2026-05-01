#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <meta>
#include <sstream>

#include <pagmo/reflection.hpp>

// model

namespace model
{

template <std::size_t Idx>
struct FreeVariable {
};

template <std::size_t Idx>
struct Objective {
};

template <std::size_t Idx>
struct Constraint {
};

template <std::size_t Idx>
struct TagAlong {
};

template <std::size_t Idx>
static constexpr auto var = FreeVariable<Idx>();

template <std::size_t Idx>
static constexpr auto obj = Objective<Idx>();

template <std::size_t Idx>
static constexpr auto con = Constraint<Idx>();

template <std::size_t Idx>
static constexpr auto tag = TagAlong<Idx>();

} // namespace model

class ModelBinder
{
    using Setter = std::function<void(double)>;
    using Getter = std::function<double()>;

    std::vector<Setter> freeVarSetters;
    std::vector<Getter> objGetters;
    std::vector<Getter> conGetters;
    std::vector<Getter> tagAlongGetters;

public:
    template <typename T>
    ModelBinder(T &t)
    {
        using namespace std::meta;

        auto bind_setter = [&](std::size_t idx, auto fn) {
            if (idx >= freeVarSetters.size()) {
                freeVarSetters.resize(idx + 1);
            }
            freeVarSetters[idx] = std::bind(fn, std::ref(t), std::placeholders::_1);
        };
        auto bind_getter = [&](std::vector<Getter> &vec, std::size_t idx, auto fn) {
            if (idx >= vec.size()) {
                vec.resize(idx + 1);
            }
            vec[idx] = std::bind(fn, std::ref(t));
        };

        template for (constexpr auto m : std::define_static_array(members_of(^^T, access_context::current())))
        {
            if constexpr (is_function(m) && has_identifier(m)) {
                template for (constexpr auto a : std::define_static_array(annotations_of(m)))
                {
                    constexpr std::size_t idx = [:template_arguments_of(type_of(a))[0]:];
                    if constexpr (template_of(type_of(a)) == ^^model::FreeVariable) {
                        bind_setter(idx, &[:m:]);
                    } else if constexpr (template_of(type_of(a)) == ^^model::Objective) {
                        bind_getter(objGetters, idx, &[:m:]);
                    } else if constexpr (template_of(type_of(a)) == ^^model::Constraint) {
                        bind_getter(conGetters, idx, &[:m:]);
                    } else if constexpr (template_of(type_of(a)) == ^^model::TagAlong) {
                        bind_getter(tagAlongGetters, idx, &[:m:]);
                    }
                }
            }
        }
    }

    void set_free_variables(std::vector<double> vals)
    {
        assert(vals.size() == freeVarSetters.size());
        for (std::size_t ii = 0; ii < freeVarSetters.size(); ++ii) {
            freeVarSetters[ii](vals[ii]);
        }
    }

    std::vector<double> get_objectives() const
    {
        std::vector<double> vals;
        for (const auto &objective : objGetters) {
            vals.push_back(objective());
        }
        return vals;
    }
};

// No type-erasure version: T is a template parameter, calls are direct member function invocations

template <typename U>
void static_set_free_variables(U &u, const std::vector<double> &vals)
{
    using namespace std::meta;
    template for (constexpr auto m : std::define_static_array(members_of(^^U, access_context::current())))
    {
        if constexpr (is_function(m) && has_identifier(m)) {
            template for (constexpr auto a : std::define_static_array(annotations_of(m)))
            {
                if constexpr (template_of(type_of(a)) == ^^model::FreeVariable) {
                    constexpr std::size_t idx = [:template_arguments_of(type_of(a))[0]:];
                    using param_t = typename[:type_of(parameters_of(m)[0]):];
                    (u.*&[:m:])(static_cast<param_t>(vals[idx]));
                }
            }
        }
    }
}

template <typename U>
std::vector<double> static_get_objectives(U &u)
{
    using namespace std::meta;
    std::vector<double> result;
    template for (constexpr auto m : std::define_static_array(members_of(^^U, access_context::current())))
    {
        if constexpr (is_function(m) && has_identifier(m)) {
            template for (constexpr auto a : std::define_static_array(annotations_of(m)))
            {
                if constexpr (template_of(type_of(a)) == ^^model::Objective) {
                    constexpr std::size_t idx = [:template_arguments_of(type_of(a))[0]:];
                    if (idx >= result.size()) result.resize(idx + 1);
                    result[idx] = (u.*&[:m:])();
                }
            }
        }
    }
    return result;
}

struct StaticModelBinderInnerBase {
    virtual void set_free_variables(const std::vector<double> &vals) = 0;
    virtual std::vector<double> get_objectives() = 0;
};

template <typename T>
class StaticModelBinderInner : public StaticModelBinderInnerBase
{
    T t_;

public:
    StaticModelBinderInner(T &t) : t_(t) {}

    void set_free_variables(const std::vector<double> &vals)
    {
        static_set_free_variables(t_, vals);
    }

    std::vector<double> get_objectives()
    {
        return static_get_objectives(t_);
    }
};

class StaticModelBinder
{
public:
    template <typename T>
    StaticModelBinder(T &t) : impl_(std::make_unique<StaticModelBinderInner<T>>(t))
    {
    }

    void set_free_variables(const std::vector<double> &vals)
    {
        impl_->set_free_variables(vals);
    }

    std::vector<double> get_objectives()
    {
        return impl_->get_objectives();
    }

private:
    std::unique_ptr<StaticModelBinderInnerBase> impl_;
};

struct A {

    A(std::vector<double> vals) : a(vals[0]), b(vals[1]) {}

    [[= model::var<0>]] void set_a(double aa)
    {
        a = aa;
    }

    [[= model::var<1>]] void set_b(int bb)
    {
        b = bb;
    }

    [[= model::obj<0>]] double get_a() const
    {
        return a;
    }

    [[= model::obj<1>]] int get_b() const
    {
        return b;
    }

    void print() const
    {
        std::cout << "A{a=" << a << ", b=" << b << "}\n";
    }

private:
    double a;
    int b;
};

void specialize_and_bind()
{
    A a({0.0, 0});
    ModelBinder binder(a);
    a.print();
    binder.set_free_variables({1.0, 1});
    a.print();
    auto vals = binder.get_objectives();
    A a2(vals);
    a2.print();
}

void static_bind_test()
{
    A a({0.0, 0});
    StaticModelBinder binder(a);
    a.print();
    binder.set_free_variables({1.0, 1});
    a.print();
    auto vals = binder.get_objectives();
    A a2(vals);
    a2.print();
}

int main()
{
    // specialize_and_bind();
    // static_bind_test();

    constexpr int N = 100000;

    // Benchmark ModelBinder (type-erased)
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        A a({0.0, 0});
        ModelBinder binder(a);
        binder.set_free_variables({1.0, 1});
        auto vals = binder.get_objectives();
        (void)vals;
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    // Benchmark StaticModelBinder (no type-erasure)
    auto t2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        A a({0.0, 0});
        StaticModelBinder binder(a);
        binder.set_free_variables({1.0, 1});
        auto vals = binder.get_objectives();
        (void)vals;
    }
    auto t3 = std::chrono::high_resolution_clock::now();

    auto ms_erased = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    auto ms_static = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

    std::cout << "ModelBinder (type-erased):    " << ms_erased << " us\n";
    std::cout << "StaticModelBinder (no-erase): " << ms_static << " us\n";
}