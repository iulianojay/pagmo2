#!/usr/bin/env python3
"""Transform pagmo2 test files to use new specialized exception types."""

import re
import sys
from pathlib import Path

TESTS_DIR = Path('/home/jay/projects/pagmo2/tests')

# ============================================================
# Helper: extract EXPECT_THROW parts handling nested parens/braces
# ============================================================

def find_matching_paren(text, start):
    """Find matching ')' for '(' at position start."""
    depth = 0
    i = start
    in_str = False
    str_char = None
    while i < len(text):
        c = text[i]
        if in_str:
            if c == '\\':
                i += 2
                continue
            if c == str_char:
                in_str = False
        elif c in ('"', "'"):
            in_str = True
            str_char = c
        elif c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


def parse_expect_throw(text, start):
    """
    Parse EXPECT_THROW starting at 'start' (index of '(' after EXPECT_THROW).
    Returns (stmt, exception_type, end_pos, is_3arg) or None on failure.
    end_pos is position after the closing ')' and ';'.
    """
    # Find content between outer parens
    end = find_matching_paren(text, start)
    if end == -1:
        return None

    inner = text[start + 1:end]

    # Split into arguments at top-level commas
    args = []
    depth = 0
    current = []
    in_str = False
    str_char = None
    i = 0
    while i < len(inner):
        c = inner[i]
        if in_str:
            if c == '\\':
                current.append(c)
                i += 1
                if i < len(inner):
                    current.append(inner[i])
                i += 1
                continue
            if c == str_char:
                in_str = False
            current.append(c)
        elif c in ('"', "'"):
            in_str = True
            str_char = c
            current.append(c)
        elif c in ('(', '{', '[', '<') and c != '<':
            depth += 1
            current.append(c)
        elif c == '<':
            # Template angle brackets - tricky, just track depth loosely
            current.append(c)
        elif c in (')', '}', ']'):
            depth -= 1
            current.append(c)
        elif c == ',' and depth == 0:
            args.append(''.join(current))
            current = []
        else:
            current.append(c)
        i += 1

    if current:
        args.append(''.join(current))

    if len(args) < 2:
        return None

    stmt = args[0].strip()
    exc_type = args[1].strip()
    is_3arg = len(args) >= 3

    # Find end position (after ';')
    pos = end + 1
    while pos < len(text) and text[pos] in (' ', '\t', '\n', '\r'):
        pass
    # Look for semicolon
    semi_pos = text.find(';', end)
    if semi_pos == -1:
        end_pos = end + 1
    else:
        end_pos = semi_pos + 1

    return stmt, exc_type, end_pos, is_3arg


# ============================================================
# Exception type determination per file/statement
# ============================================================

# Multi-objective problem names in test statements
MULTI_OBJ_RE = re.compile(r'\b(zdt|dtlz|wfg|maco|nspso|nsga2|moead|cec2014)\b')
# Constrained problem names  
CONSTRAINED_RE = re.compile(r'\b(hock_schittkowski_71|schwefel_00|cec2006)\b')
# Stochastic problem names
STOCHASTIC_RE = re.compile(r'\binventory\b')
# Small population (1u, 2u, 3u, 4u)
SMALL_POP_RE = re.compile(r'population\{[^}]+,\s*[1-4]u\}')


def determine_algo_type(stmt):
    """Determine exception type for algorithm evolve/ctor calls."""
    if '.evolve(' in stmt:
        if MULTI_OBJ_RE.search(stmt):
            return 'incompatible_problem_error'
        if CONSTRAINED_RE.search(stmt):
            return 'incompatible_problem_error'
        if STOCHASTIC_RE.search(stmt):
            return 'incompatible_problem_error'
        if SMALL_POP_RE.search(stmt):
            return 'insufficient_population_error'
        # Check for pop_lb, pop_ub (infinite bounds in cmaes, sga, etc.)
        if re.search(r'pop_lb|pop_ub|pop_inf', stmt):
            return 'invalid_parameter_error'
        # Default evolve error - usually population size
        return 'insufficient_population_error'
    else:
        return 'invalid_parameter_error'


def get_file_rules(filename):
    """
    Return per-file rules: list of (stmt_pattern, new_exception_type) plus a default type.
    Rules are applied in order; first match wins.
    Returns (rules, default_type) where rules is list of (compiled_re, type_str).
    """
    f = filename

    # ---- Simple single-type files ----
    single_type_files = {
        'ackley.cpp': 'problem_config_error',
        'cec2006.cpp': 'problem_config_error',
        'cec2009.cpp': 'problem_config_error',
        'cec2013.cpp': 'problem_config_error',
        'cec2014.cpp': 'problem_config_error',
        'griewank.cpp': 'problem_config_error',
        'rastrigin.cpp': 'problem_config_error',
        'rosenbrock.cpp': 'problem_config_error',
        'schwefel.cpp': 'problem_config_error',
        'wfg.cpp': 'problem_config_error',
        'minlp_rastrigin.cpp': 'problem_config_error',
        'zdt.cpp': 'problem_config_error',
        'fully_connected.cpp': 'index_error',
        'free_form.cpp': 'invalid_value_error',
        'ring.cpp': 'index_error',
        'unconnected.cpp': 'index_error',
        'rng.cpp': None,  # no pagmo exceptions expected
        'rng_serialization.cpp': None,
        'type_traits.cpp': None,
        'algorithm_type_traits.cpp': None,
        'problem_type_traits.cpp': None,
        'io.cpp': None,
        'threading.cpp': None,
        'eigen3_serialization.cpp': None,
    }

    if f in single_type_files:
        t = single_type_files[f]
        return [], t

    # ---- Algorithm files (ctor → invalid_parameter_error, evolve → determined by stmt) ----
    algo_files = {
        'bee_colony.cpp', 'compass_search.cpp', 'cmaes.cpp', 'de.cpp', 'de1220.cpp',
        'gaco.cpp', 'gwo.cpp', 'ihs.cpp', 'maco.cpp', 'mbh.cpp', 'moead.cpp',
        'moead_gen.cpp', 'nspso.cpp', 'nsga2.cpp', 'pso.cpp', 'pso_gen.cpp',
        'sade.cpp', 'sea.cpp', 'sga.cpp', 'simulated_annealing.cpp', 'xnes.cpp',
        'cstrs_self_adaptive.cpp', 'custom_comparisons.cpp',
    }
    if f in algo_files:
        return None, 'ALGO'  # Special marker

    # ---- Files with mixed types ----
    if f == 'archipelago.cpp':
        return [
            (re.compile(r'\[\d+\]|get_island_idx|get_migrants|set_migrants'), 'index_error'),
            (re.compile(r'push_back'), 'size_limit_error'),
            # wait_check/get_champions: these test propagation through archipelago;
            # the source now throws pagmo exceptions which derive from pagmo_exception
            (re.compile(r'wait_check|get_champions'), 'pagmo_exception'),
            (re.compile(r'pthrower'), 'pagmo_exception'),
        ], 'index_error'

    if f == 'population.cpp':
        return [
            (re.compile(r'best_idx|worst_idx'), 'empty_collection_error'),
            (re.compile(r'champion_f|champion_x'), 'incompatible_problem_error'),
            (re.compile(r'set_xf\(\s*\d+,'), 'index_error'),  # set_xf with index 2 (out of range)
            # set_xf with wrong sizes → dimension_mismatch_error
            (re.compile(r'set_xf\(1,\s*\{'), 'dimension_mismatch_error'),
            (re.compile(r'push_back.*overflow|numeric_limits.*max'), 'size_limit_error'),
            (re.compile(r'push_back'), 'dimension_mismatch_error'),
        ], 'dimension_mismatch_error'

    if f == 'problem.cpp':
        return [
            (re.compile(r'gradient_sparsity|hessians_sparsity'), 'sparsity_pattern_error'),
            (re.compile(r'gradient_sparsity|hessians_sparsity'), 'sparsity_pattern_error'),
            (re.compile(r'null_problem\{0'), 'problem_config_error'),
            (re.compile(r'null_problem\{2.*3.*4.*2|null_problem\{[,\d\s]*\}.*nix'), 'problem_config_error'),
            (re.compile(r'nobj.*max|nec.*max|nic.*max|numeric_limits.*max'), 'size_limit_error'),
            (re.compile(r'gradient\(|hessians\(|fitness\('), 'dimension_mismatch_error'),
            (re.compile(r'set_c_tol.*NaN|set_c_tol.*nan|quiet_NaN'), 'invalid_value_error'),
            (re.compile(r'set_c_tol\(-'), 'invalid_value_error'),
            (re.compile(r'set_c_tol\('), 'dimension_mismatch_error'),
            (re.compile(r'feasibility_f'), 'dimension_mismatch_error'),
            (re.compile(r'minlp\{5'), 'bounds_constraint_error'),
            # grad/hess sparsity out of bounds or repeats
            (re.compile(r'grads_2_out|grads_2_rep|hesss_22_out|hesss_22_not|hesss_22_rep'), 'sparsity_pattern_error'),
            # base_p(1,0,0,...) with size issues
            (re.compile(r'base_p\(0,'), 'bounds_constraint_error'),  # nobj==0
            (re.compile(r'base_p\(1,\s*0,\s*0,.*ub_2.*lb_2|ub_2.*lb_2'), 'bounds_constraint_error'),
            (re.compile(r'base_p\(1,.*lb_3|lb_3|ub_3'), 'invalid_value_error'),
            (re.compile(r'batch_fitness'), 'batch_eval_error'),
        ], 'invalid_value_error'

    if f == 'base_bgl_topology.cpp':
        return [
            (re.compile(r'set_weight.*-1\.|set_all_weights.*-1\.|set_weight.*infinity|set_all_weights.*infinity'), 'invalid_value_error'),
        ], 'index_error'

    if f == 'base_sr_policy.cpp':
        return [
            (re.compile(r'bsrp\(-1\)|bsrp\(2\.\)|bsrp.*infinity'), 'policy_config_error'),
            (re.compile(r'bsrp\(-1\b'), 'policy_config_error'),
        ], 'policy_config_error'

    if f == 'topology.cpp':
        return [
            (re.compile(r'topology_check_edge_weight|set_weight|weight.*nan|weight.*inf|non.?finite'), 'invalid_value_error'),
            (re.compile(r'get_connections'), 'dimension_mismatch_error'),
        ], 'invalid_value_error'

    if f == 'decompose.cpp':
        return [
            (re.compile(r'not.*multi.*obj|incompatible.*multi|single.*obj|decompose.*single'), 'incompatible_problem_error'),
            (re.compile(r'has.*constraint|constrained'), 'incompatible_problem_error'),
            (re.compile(r'unknown.*method|method.*unknown'), 'decomposition_error'),
            (re.compile(r'weight.*sum|sum.*weight|negative.*weight|weight.*negative'), 'decomposition_error'),
            (re.compile(r'weight.*size|ref.*point.*size|dim.*weight'), 'dimension_mismatch_error'),
            (re.compile(r'non.?finite|NaN|infinity|nan'), 'invalid_value_error'),
            (re.compile(r'decompose\{'), 'incompatible_problem_error'),
            (re.compile(r'weight_vector\(|ref_point\(|\.decompose\('), 'decomposition_error'),
        ], 'incompatible_problem_error'

    if f == 'unconstrain.cpp':
        return [
            (re.compile(r'unconstrained|no.*constraint|without.*constraint'), 'incompatible_problem_error'),
            (re.compile(r'weight.*size|weights.*dim'), 'dimension_mismatch_error'),
            (re.compile(r'unknown.*method|method.*unknown|invalid.*method'), 'invalid_parameter_error'),
            (re.compile(r'non.empty.*weight|weight.*non.empty'), 'invalid_parameter_error'),
            (re.compile(r'unconstrain\{'), 'incompatible_problem_error'),
        ], 'incompatible_problem_error'

    if f == 'translate.cpp':
        return [], 'dimension_mismatch_error'

    if f == 'constrained.cpp':
        return [
            (re.compile(r'fitness.*size|tolerance.*size|dim.*mismatch'), 'dimension_mismatch_error'),
            (re.compile(r'fitness.*dim.*<.*1|dim.*<.*1|multi.*obj.*<.*1'), 'multi_objective_error'),
            (re.compile(r'neq.*>|>.*neq'), 'bounds_constraint_error'),
        ], 'dimension_mismatch_error'

    if f == 'discrepancy.cpp':
        return [], 'utility_error'

    if f == 'hypervolume.cpp':
        return [
            (re.compile(r'pop2|pop_empty|pop_wrong'), 'incompatible_problem_error'),
            (re.compile(r'quiet_NaN|NaN|nan'), 'invalid_value_error'),
            (re.compile(r'empty.*empty|\{\s*empty'), 'empty_collection_error'),
            (re.compile(r'emptyempty|\{\}'), 'empty_collection_error'),
            (re.compile(r'least_contributor|exclusive\(200'), 'index_error'),
            (re.compile(r'hvwfg\(0\)|hvwfg\(1\)'), 'invalid_parameter_error'),
            (re.compile(r'bf_fpras|bf_approx'), 'invalid_parameter_error'),
            (re.compile(r'compute\(\{.*\}.*hv_algo_3d|compute\(\{.*\}.*hv_algo_2d'), 'problem_config_error'),
            (re.compile(r'compute\(\{[0-9., ]+\}\)'), 'dimension_mismatch_error'),
            (re.compile(r'\{\{.*\},.*\{.*,.*\}.*\}'), 'dimension_mismatch_error'),
            (re.compile(r'al\.compute|al\.exclusive|al\.least|al\.greatest|al\.contributions'), 'multi_objective_error'),
        ], 'dimension_mismatch_error'

    if f == 'genetic_operators.cpp':
        return [
            (re.compile(r'sbx|uniform_integer|crossover|mutation'), 'dimension_mismatch_error'),
            (re.compile(r'infinite|infinity|nan|NaN'), 'invalid_value_error'),
        ], 'dimension_mismatch_error'

    if f == 'generic.cpp':
        return [
            (re.compile(r'kNN|knn.*dim|heterogeneous'), 'dimension_mismatch_error'),
            (re.compile(r'numeric_limits.*max|overflow'), 'size_limit_error'),
        ], 'utility_error'

    if f == 'gradients_and_hessians.cpp':
        return [], 'dimension_mismatch_error'

    if f == 'multi_objective.cpp':
        return [
            (re.compile(r'decompose.*method|unknown.*method'), 'decomposition_error'),
            (re.compile(r'<.*2.*points|<.*2.*obj|single.*obj|one.*obj'), 'multi_objective_error'),
            (re.compile(r'dim.*mismatch|size.*mismatch'), 'dimension_mismatch_error'),
        ], 'dimension_mismatch_error'

    if f == 'dtlz.cpp':
        return [
            (re.compile(r'p_distance.*wrong|p_distance.*size'), 'dimension_mismatch_error'),
        ], 'problem_config_error'

    if f == 'golomb_ruler.cpp':
        return [
            (re.compile(r'numeric_limits.*max|overflow'), 'size_limit_error'),
        ], 'problem_config_error'

    if f == 'lennard_jones.cpp':
        return [
            (re.compile(r'numeric_limits.*max|overflow'), 'size_limit_error'),
        ], 'problem_config_error'

    if f == 'bfe.cpp':
        return [
            (re.compile(r'bfe0.*rosenbrock|bfe.*problem.*rosenbrock'), 'batch_eval_error'),
            (re.compile(r'bfe1|bfe2|default_bfe|thread_bfe'), 'batch_eval_error'),
        ], 'batch_eval_error'

    if f == 'default_bfe.cpp':
        return [], 'batch_eval_error'

    if f == 'thread_bfe.cpp':
        return [
            (re.compile(r'overflow|numeric_limits.*max|size.*max'), 'size_limit_error'),
        ], 'batch_eval_error'

    if f == 'member_bfe.cpp':
        return [], 'batch_eval_error'

    if f == 'fork_island.cpp':
        return [], 'system_error'

    if f == 'thread_island.cpp':
        return [], 'incompatible_problem_error'

    if f == 'island.cpp':
        return [], 'pagmo_exception'

    if f == 'fair_replace.cpp':
        return [], 'policy_config_error'

    if f == 'select_best.cpp':
        return [], 'policy_config_error'

    if f == 'r_policy.cpp':
        return [], 'policy_config_error'

    if f == 's_policy.cpp':
        return [], 'policy_config_error'

    if f == 'algorithm.cpp':
        return [], 'not_implemented_error'

    if f == 'luksan_vlcek1.cpp':
        return [], 'problem_config_error'

    # Default: just use pagmo_exception as a safe fallback
    return [], 'pagmo_exception'


# ============================================================
# Main transformation logic
# ============================================================

OLD_TYPES = [
    'std::invalid_argument',
    'std::out_of_range',
    'std::overflow_error',
    'std::runtime_error',
]

OLD_TYPE_PATTERN = re.compile(
    r'\b(std::invalid_argument|std::out_of_range|std::overflow_error|std::runtime_error)\b'
)


def determine_type(stmt, rules, default_type):
    """Determine the new exception type based on statement patterns."""
    if rules is None or default_type == 'ALGO':
        return determine_algo_type(stmt)

    if rules:
        for pattern, new_type in rules:
            if pattern.search(stmt):
                return new_type

    return default_type


def transform_expect_throws(content, rules, default_type):
    """Transform all EXPECT_THROW calls in the content."""
    result = []
    i = 0

    while i < len(content):
        # Look for EXPECT_THROW(
        m = re.search(r'\bEXPECT_THROW\s*\(', content[i:])
        if not m:
            result.append(content[i:])
            break

        # Append everything before the match
        result.append(content[i:i + m.start()])
        paren_start = i + m.end() - 1  # position of '('

        parsed = parse_expect_throw(content, paren_start)
        if parsed is None:
            # Could not parse - keep as-is
            result.append(content[i + m.start():i + m.end()])
            i = i + m.end()
            continue

        stmt, exc_type, end_pos, is_3arg = parsed

        # Only transform if it uses an old exception type
        if not OLD_TYPE_PATTERN.search(exc_type):
            # Keep as-is - may use not_implemented_error or pagmo types already
            result.append(content[i + m.start():end_pos])
            i = end_pos
            continue

        # Determine new exception type
        new_type = determine_type(stmt, rules, default_type)

        if new_type is None:
            # Keep as-is
            result.append(content[i + m.start():end_pos])
            i = end_pos
            continue

        # Build replacement
        # Preserve original indentation of EXPECT_THROW
        orig_text = content[i + m.start():end_pos]

        # Get indent
        line_start = content.rfind('\n', 0, i + m.start())
        indent = ''
        if line_start != -1:
            line = content[line_start + 1:i + m.start()]
            indent = re.match(r'^(\s*)', line).group(1)

        replacement = f'EXPECT_THROW({stmt}, {new_type});'
        result.append(replacement)
        i = end_pos

    return ''.join(result)


def transform_catch_blocks(content, filename):
    """Transform catch (const std::invalid_argument &) blocks."""
    # For thread torture tests and similar, replace with pagmo_exception
    for old_type in OLD_TYPES:
        # catch (const std::invalid_argument &)
        # catch (const std::invalid_argument &e)
        old_catch = re.compile(
            r'\bcatch\s*\(\s*const\s+' + re.escape(old_type) + r'\s*(?:&)?\s*(?:\w+)?\s*\)'
        )
        content = old_catch.sub('catch (const pagmo_exception &)', content)

        # Also handle: } catch (const std::invalid_argument &) {
        # Already covered above
    return content


def add_include(content):
    """Add #include <pagmo/exceptions.hpp> if not present."""
    if '#include <pagmo/exceptions.hpp>' in content:
        return content

    # Find a good place to insert - after last pagmo include or after gtest include
    # Look for last #include <pagmo/...>
    pagmo_includes = list(re.finditer(r'#include\s+<pagmo/[^>]+>', content))
    if pagmo_includes:
        last_include = pagmo_includes[-1]
        insert_pos = last_include.end()
        return content[:insert_pos] + '\n#include <pagmo/exceptions.hpp>' + content[insert_pos:]

    # Look for gtest include
    gtest_include = re.search(r'#include\s+<gtest/gtest\.h>', content)
    if gtest_include:
        insert_pos = gtest_include.end()
        return content[:insert_pos] + '\n#include <pagmo/exceptions.hpp>' + content[insert_pos:]

    return content


def transform_file(filepath):
    """Transform a single test file."""
    filename = Path(filepath).name
    rules, default_type = get_file_rules(filename)

    content = Path(filepath).read_text()
    original = content

    # Check if file has anything to transform
    if not OLD_TYPE_PATTERN.search(content):
        return False, 0

    # Count original occurrences
    orig_count = len(list(OLD_TYPE_PATTERN.finditer(content)))

    # Transform EXPECT_THROW calls
    content = transform_expect_throws(content, rules, default_type)

    # Transform catch blocks
    content = transform_catch_blocks(content, filename)

    # Add include if we made changes and have pagmo exceptions
    if content != original:
        content = add_include(content)

    if content != original:
        Path(filepath).write_text(content)
        # Count remaining
        remaining = len(list(OLD_TYPE_PATTERN.finditer(content)))
        return True, orig_count - remaining

    return False, 0


# ============================================================
# Entry point
# ============================================================

def main():
    test_files = sorted(TESTS_DIR.glob('*.cpp'))

    total_changed = 0
    total_replacements = 0
    changed_files = []

    for f in test_files:
        changed, count = transform_file(f)
        if changed:
            changed_files.append((f.name, count))
            total_changed += 1
            total_replacements += count

    print(f"Changed {total_changed} files, ~{total_replacements} replacements")
    print("\nChanged files:")
    for fname, count in changed_files:
        print(f"  {fname}: ~{count} replacements")


if __name__ == '__main__':
    main()
