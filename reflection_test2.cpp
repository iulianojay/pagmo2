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
struct SubObject {
};

template <std::size_t Idx>
static constexpr auto var = FreeVariable<Idx>();

template <std::size_t Idx>
static constexpr auto obj = Objective<Idx>();

template <std::size_t Idx>
static constexpr auto con = Constraint<Idx>();

template <std::size_t Idx>
static constexpr auto tag = TagAlong<Idx>();

template <std::size_t Idx>
static constexpr auto sub = SubObject<Idx>();

} // namespace model

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
                } else if constexpr (template_of(type_of(a)) == ^^model::SubObject) {
                    static_set_free_variables((u.*&[:m:])(), vals);
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
                } else if constexpr (template_of(type_of(a)) == ^^model::SubObject) {
                    auto sub_vals = static_get_objectives((u.*&[:m:])());
                    for (std::size_t i = 0; i < sub_vals.size(); ++i) {
                        if (i >= result.size()) result.resize(i + 1);
                        result[i] = sub_vals[i];
                    }
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
    T &t_;

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

struct DirectModelBinderInnerBase {
    virtual void set_free_variables(const std::vector<double> &vals) = 0;
    virtual std::vector<double> get_objectives() = 0;
};

template <typename T>
class DirectModelBinderInner : public DirectModelBinderInnerBase
{
    T &t_;

public:
    DirectModelBinderInner(T &t) : t_(t) {}

    void set_free_variables(const std::vector<double> &vals)
    {
        set_free_variables_impl(t_, vals);
    }

    template <typename U>
    void set_free_variables_impl(U &u, const std::vector<double> &vals)
    {
        t_.set_free_variables(vals);
    }

    std::vector<double> get_objectives()
    {
        return get_objectives_impl(t_);
    }

    template <typename U>
    std::vector<double> get_objectives_impl(U &t)
    {
        return t.get_objectives();
    }
};

class DirectModelBinder
{
public:
    template <typename T>
    DirectModelBinder(T &t) : impl_(std::make_unique<DirectModelBinderInner<T>>(t))
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
    std::unique_ptr<DirectModelBinderInnerBase> impl_;
};

struct SubA {

    SubA(double cc) : c(cc) {}

    [[= model::var<2>]] void set_c(double cc)
    {
        c = cc;
    }

    [[= model::obj<2>]] double get_c() const
    {
        return c;
    }

    void print() const
    {
        std::cout << "SubA{c=" << c << "}";
    }

private:
    double c;
};

struct ADirect {
    void set_free_variables(const std::vector<double> &vals)
    {
        a = vals[0];
        b = static_cast<int>(vals[1]);
        sub.set_c(vals[2]);
    }

    std::vector<double> get_objectives() const
    {
        return {a, static_cast<double>(b), sub.get_c()};
    }

    void print() const
    {
        std::cout << "A{a=" << a << ", b=" << b << ", sub=";
        sub.print();
        std::cout << "}\n";
    }

    double a;
    int b;
    SubA sub;
};

struct A {

    A(std::vector<double> vals) : a(vals[0]), b(vals[1]), sub(vals[2]) {}

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

    [[= model::sub<0>]] SubA &get_sub()
    {
        return sub;
    }

    void print() const
    {
        std::cout << "A{a=" << a << ", b=" << b << ", sub=";
        sub.print();
        std::cout << "}\n";
    }

private:
    double a;
    int b;
    SubA sub;
};

int main()
{
    constexpr int N = 100000;

    // Benchmark StaticModelBinder (no type-erasure)
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        A a({0.0, 0, 0.0});
        StaticModelBinder binder(a);
        // a.print();
        binder.set_free_variables({1.0, 2, 3.0});
        // a.print();
        auto vals = binder.get_objectives();
        (void)vals;
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    // Benchmark DirectModelBinder (type-erased)
    auto t2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        ADirect a({0.0, 0, 0.0});
        DirectModelBinder binder(a);
        // a.print();
        binder.set_free_variables({1.0, 2, 3.0});
        // a.print();
        auto vals = binder.get_objectives();
        (void)vals;
    }
    auto t3 = std::chrono::high_resolution_clock::now();

    auto ms_static = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    auto ms_direct = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

    std::cout << "StaticModelBinder (static unpacking)   : " << ms_static << " us\n";
    std::cout << "DirectModelBinder (direct type-erasure): " << ms_direct << " us\n";
}