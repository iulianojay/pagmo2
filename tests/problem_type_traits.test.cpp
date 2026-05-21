/* Copyright 2017-2021 PaGMO development team

This file is part of the PaGMO library.

The PaGMO library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The PaGMO library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the PaGMO library.  If not,
see https://www.gnu.org/licenses/. */


#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include <pagmo/concepts.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/type_traits.hpp>
#include <pagmo/types.hpp>

using namespace pagmo;

// No fitness.
struct f_00 {
};

// Various types of wrong fitness.
struct f_01 {
    void fitness();
};

struct f_02 {
    void fitness(const vector_double &);
};

struct f_03 {
    vector_double fitness(const vector_double &);
};

struct f_04 {
    vector_double fitness(vector_double &) const;
};

// Good one.
struct f_05 {
    vector_double fitness(const vector_double &) const;
};

TEST(problem_type_traits_test, has_fitness_test)
{
    EXPECT_TRUE((!HasFitness<f_00>));
    EXPECT_TRUE((!HasFitness<f_01>));
    EXPECT_TRUE((!HasFitness<f_02>));
    EXPECT_TRUE((!HasFitness<f_03>));
    EXPECT_TRUE((!HasFitness<f_04>));
    EXPECT_TRUE((HasFitness<f_05>));
}

// No fitness.
struct no_00 {
};

// Various types of wrong get_nobj.
struct no_01 {
    vector_double::size_type get_nobj();
};

struct no_02 {
    int get_nobj() const;
};

// Good one.
struct no_03 {
    vector_double::size_type get_nobj() const;
};

TEST(problem_type_traits_test, has_get_nobj_test)
{
    EXPECT_TRUE((!HasFitness<no_00>));
    EXPECT_TRUE((!HasFitness<no_01>));
    EXPECT_TRUE((!HasFitness<no_02>));
    EXPECT_TRUE((!HasFitness<no_03>));
}

struct db_00 {
};

// The good one.
struct db_01 {
    std::pair<vector_double, vector_double> get_bounds() const;
};

struct db_02 {
    std::pair<vector_double, vector_double> get_bounds();
};

struct db_03 {
    vector_double get_bounds() const;
};

struct db_04 {
};

TEST(problem_type_traits_test, has_bounds_test)
{
    EXPECT_TRUE((!HasBounds<db_00>));
    EXPECT_TRUE((HasBounds<db_01>));
    EXPECT_TRUE((!HasBounds<db_02>));
    EXPECT_TRUE((!HasBounds<db_03>));
    EXPECT_TRUE((!HasBounds<db_04>));
}

struct c_00 {
};

// The good one.
struct c_01 {
    vector_double::size_type get_nec() const;
    vector_double::size_type get_nic() const;
};

struct c_02 {
    vector_double::size_type get_nec();
    vector_double::size_type get_nic() const;
};

struct c_03 {
    int get_nec() const;
    vector_double::size_type get_nic() const;
};

struct c_04 {
    vector_double::size_type get_nec() const;
    vector_double::size_type get_nic();
};

struct c_05 {
    vector_double::size_type get_nec() const;
    void get_nic() const;
};

struct c_06 {
    vector_double::size_type get_nec() const;
};

struct c_07 {
    vector_double::size_type get_nic() const;
};

TEST(problem_type_traits_test, has_e_constraints_test)
{
    EXPECT_TRUE((!HasEConstraints<c_00>));
    EXPECT_TRUE((HasEConstraints<c_01>));
    EXPECT_TRUE((!HasEConstraints<c_02>));
    EXPECT_TRUE((!HasEConstraints<c_03>));
    EXPECT_TRUE((HasEConstraints<c_04>));
    EXPECT_TRUE((HasEConstraints<c_05>));
    EXPECT_TRUE((HasEConstraints<c_06>));
    EXPECT_TRUE((!HasEConstraints<c_07>));
}

TEST(problem_type_traits_test, has_i_constraints_test)
{
    EXPECT_TRUE((!HasIConstraints<c_00>));
    EXPECT_TRUE((HasIConstraints<c_01>));
    EXPECT_TRUE((HasIConstraints<c_02>));
    EXPECT_TRUE((HasIConstraints<c_03>));
    EXPECT_TRUE((!HasIConstraints<c_04>));
    EXPECT_TRUE((!HasIConstraints<c_05>));
    EXPECT_TRUE((!HasIConstraints<c_06>));
    EXPECT_TRUE((HasIConstraints<c_07>));
}

struct i_00 {
};

// The good one.
struct i_01 {
    vector_double::size_type get_nix() const;
};

struct i_02 {
    vector_double::size_type get_nix();
};

struct i_03 {
    void get_nix() const;
};

struct i_04 {
    vector_double::size_type get_nixx() const;
};

TEST(problem_type_traits_test, has_integer_part_test)
{
    EXPECT_TRUE((!HasIntegerPart<i_00>));
    EXPECT_TRUE((HasIntegerPart<i_01>));
    EXPECT_TRUE((!HasIntegerPart<i_02>));
    EXPECT_TRUE((!HasIntegerPart<i_03>));
    EXPECT_TRUE((!HasIntegerPart<i_04>));
}

struct n_00 {
};

// The good one.
struct n_01 {
    std::string get_name() const;
};

struct n_02 {
    std::string get_name();
};

struct n_03 {
    void get_name() const;
};

TEST(problem_type_traits_test, has_name_test)
{
    EXPECT_TRUE((!HasGetName<n_00>));
    EXPECT_TRUE((HasGetName<n_01>));
    EXPECT_TRUE((!HasGetName<n_02>));
    EXPECT_TRUE((!HasGetName<n_03>));
}

struct ei_00 {
};

// The good one.
struct ei_01 {
    std::string get_extra_info() const;
};

struct ei_02 {
    std::string get_extra_info();
};

struct ei_03 {
    void get_extra_info() const;
};

TEST(problem_type_traits_test, has_extra_info_test)
{
    EXPECT_TRUE((!HasGetExtraInfo<ei_00>));
    EXPECT_TRUE((HasGetExtraInfo<ei_01>));
    EXPECT_TRUE((!HasGetExtraInfo<ei_02>));
    EXPECT_TRUE((!HasGetExtraInfo<ei_03>));
}

struct grad_00 {
};

// The good one.
struct grad_01 {
    vector_double gradient(const vector_double &) const;
};

struct grad_02 {
    vector_double gradient(const vector_double &);
};

struct grad_03 {
    vector_double gradient(vector_double &) const;
};

struct grad_04 {
    void gradient(const vector_double &) const;
};

TEST(problem_type_traits_test, has_gradient_test)
{
    EXPECT_TRUE((!HasGradient<grad_00>));
    EXPECT_TRUE((HasGradient<grad_01>));
    EXPECT_TRUE((!HasGradient<grad_02>));
    EXPECT_TRUE((!HasGradient<grad_03>));
    EXPECT_TRUE((!HasGradient<grad_04>));
}

struct ov_grad_00 {
};

// The good one.
struct ov_grad_01 {
    bool has_gradient() const;
};

struct ov_grad_02 {
    bool has_gradient();
};

struct ov_grad_03 {
    void has_gradient() const;
};

TEST(problem_type_traits_test, override_has_gradient_test)
{
    EXPECT_TRUE((!OverrideHasGradient<ov_grad_00>));
    EXPECT_TRUE((OverrideHasGradient<ov_grad_01>));
    EXPECT_TRUE((!OverrideHasGradient<ov_grad_02>));
    EXPECT_TRUE((!OverrideHasGradient<ov_grad_03>));
}

struct gs_00 {
};

// The good one.
struct gs_01 {
    sparsity_pattern gradient_sparsity() const;
};

struct gs_02 {
    sparsity_pattern gradient_sparsity();
};

struct gs_03 {
    int gradient_sparsity() const;
};

TEST(problem_type_traits_test, has_gradient_sparsity_test)
{
    EXPECT_TRUE((!HasGradientSparsity<gs_00>));
    EXPECT_TRUE((HasGradientSparsity<gs_01>));
    EXPECT_TRUE((!HasGradientSparsity<gs_02>));
    EXPECT_TRUE((!HasGradientSparsity<gs_03>));
}

struct ov_gs_00 {
};

// The good one.
struct ov_gs_01 {
    bool has_gradient_sparsity() const;
};

struct ov_gs_02 {
    bool has_gradient_sparsity();
};

struct ov_gs_03 {
    void has_gradient_sparsity() const;
};

TEST(problem_type_traits_test, override_has_gradient_sparsity_test)
{
    EXPECT_TRUE((!OverrideHasGradientSparsity<ov_gs_00>));
    EXPECT_TRUE((OverrideHasGradientSparsity<ov_gs_01>));
    EXPECT_TRUE((!OverrideHasGradientSparsity<ov_gs_02>));
    EXPECT_TRUE((!OverrideHasGradientSparsity<ov_gs_03>));
}

struct hess_00 {
};

// The good one.
struct hess_01 {
    std::vector<vector_double> hessians(const vector_double &) const;
};

struct hess_02 {
    std::vector<vector_double> hessians(const vector_double &);
};

struct hess_03 {
    std::vector<vector_double> hessians(vector_double &) const;
};

struct hess_04 {
    void hessians(const vector_double &) const;
};

TEST(problem_type_traits_test, has_hessians_test)
{
    EXPECT_TRUE((!HasHessians<hess_00>));
    EXPECT_TRUE((HasHessians<hess_01>));
    EXPECT_TRUE((!HasHessians<hess_02>));
    EXPECT_TRUE((!HasHessians<hess_03>));
    EXPECT_TRUE((!HasHessians<hess_04>));
}

struct ov_hess_00 {
};

// The good one.
struct ov_hess_01 {
    bool has_hessians() const;
};

struct ov_hess_02 {
    bool has_hessians();
};

struct ov_hess_03 {
    void has_hessians() const;
};

TEST(problem_type_traits_test, override_has_hessians_test)
{
    EXPECT_TRUE((!OverrideHasHessians<ov_hess_00>));
    EXPECT_TRUE((OverrideHasHessians<ov_hess_01>));
    EXPECT_TRUE((!OverrideHasHessians<ov_hess_02>));
    EXPECT_TRUE((!OverrideHasHessians<ov_hess_03>));
}

struct hs_00 {
};

// The good one.
struct hs_01 {
    std::vector<sparsity_pattern> hessians_sparsity() const;
};

struct hs_02 {
    std::vector<sparsity_pattern> hessians_sparsity();
};

struct hs_03 {
    int hessians_sparsity() const;
};

TEST(problem_type_traits_test, has_hessians_sparsity_test)
{
    EXPECT_TRUE((!HasHessiansSparsity<hs_00>));
    EXPECT_TRUE((HasHessiansSparsity<hs_01>));
    EXPECT_TRUE((!HasHessiansSparsity<hs_02>));
    EXPECT_TRUE((!HasHessiansSparsity<hs_03>));
}

struct ov_hs_00 {
};

// The good one.
struct ov_hs_01 {
    bool has_hessians_sparsity() const;
};

struct ov_hs_02 {
    bool has_hessians_sparsity();
};

struct ov_hs_03 {
    void has_hessians_sparsity() const;
};

TEST(problem_type_traits_test, override_has_hessians_sparsity_test)
{
    EXPECT_TRUE((!OverrideHasHessiansSparsity<ov_hs_00>));
    EXPECT_TRUE((OverrideHasHessiansSparsity<ov_hs_01>));
    EXPECT_TRUE((!OverrideHasHessiansSparsity<ov_hs_02>));
    EXPECT_TRUE((!OverrideHasHessiansSparsity<ov_hs_03>));
}

struct hss_00 {
};

// The good one.
struct hss_01 {
    void set_seed(unsigned);
};

struct hss_02 {
    void set_seed(unsigned) const;
};

struct hss_03 {
    void set_seed(int);
};

struct hss_04 {
    double set_seed(unsigned);
};

TEST(problem_type_traits_test, has_set_seed_test)
{
    EXPECT_TRUE((!HasSetSeed<hss_00>));
    EXPECT_TRUE((HasSetSeed<hss_01>));
    EXPECT_TRUE((HasSetSeed<hss_02>));
    EXPECT_TRUE((HasSetSeed<hss_03>));
    EXPECT_TRUE((!HasSetSeed<hss_04>));
}

struct ov_hss_00 {
};

// The good one.
struct ov_hss_01 {
    bool has_set_seed() const;
};

struct ov_hss_02 {
    bool has_set_seed();
};

struct ov_hss_03 {
    void has_set_seed() const;
};

TEST(problem_type_traits_test, override_has_set_seed_test)
{
    EXPECT_TRUE((!OverrideHasSetSeed<ov_hss_00>));
    EXPECT_TRUE((OverrideHasSetSeed<ov_hss_01>));
    EXPECT_TRUE((!OverrideHasSetSeed<ov_hss_02>));
    EXPECT_TRUE((!OverrideHasSetSeed<ov_hss_03>));
}
