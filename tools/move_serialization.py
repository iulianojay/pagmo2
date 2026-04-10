#!/usr/bin/env python3
"""
Move serialization template bodies from .cpp files to inline definitions in .hpp files.

Also:
- Removes 'unsigned' version parameter from serialize/save/load signatures
- Converts ar << x;  ->  ar(x);
- Converts ar >> x;  ->  ar(x);
- Simplifies versioned load (if version > 0) blocks
"""

import os
import re
import glob
import sys


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def find_closing_brace(text, open_pos):
    """Return index of the closing '}' that matches text[open_pos] == '{'."""
    assert text[open_pos] == '{', f"Expected '{{' at {open_pos}, got '{text[open_pos]}'"
    depth = 0
    i = open_pos
    while i < len(text):
        if text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError(f"No matching '}}' found for '{{' at position {open_pos}")


def fix_ar_stream_ops(code):
    """Replace  ar << expr;  and  ar >> expr;  with  ar(expr);"""
    # Handle multi-arg chains like  ar << a << b;  -> ar(a, b);  (rare but handle)
    # First handle single-arg: ar << expr;
    code = re.sub(r'\bar\s*(?:<<|>>)\s*([^;<>\n][^;\n]*);', lambda m: f'ar({m.group(1).rstrip()});', code)
    return code


def remove_unsigned_from_sig(sig):
    """Strip the ', unsigned X' or ', unsigned' version parameter from a function signature."""
    # (Archive &ar, unsigned version)  ->  (Archive &ar)
    # (Archive &ar, unsigned)          ->  (Archive &ar)
    # (Archive &, unsigned)            ->  (Archive &)
    sig = re.sub(r',\s*unsigned(?:\s+\w+)?\s*\)', ')', sig)
    return sig


def re_indent(body, indent='    '):
    """Re-indent all lines of body by one level (4 spaces)."""
    lines = body.split('\n')
    result = []
    for line in lines:
        if line.strip():
            result.append(indent + line)
        else:
            result.append('')
    return '\n'.join(result)


# ---------------------------------------------------------------------------
# Extraction from .cpp
# ---------------------------------------------------------------------------

TEMPLATE_FUNC_RE = re.compile(
    r'(?P<comment>(?:[ \t]*//[^\n]*\n)*)'          # optional leading comments
    r'(?P<template>template\s*<\s*typename\s+Archive\s*>\s*\n)'
    r'(?P<ret_type>void\s+)'
    r'(?P<classname>\w+)'
    r'::'
    r'(?P<funcname>serialize|save|load)'
    r'\s*\((?P<params>[^)]*)\)\s*(?P<const_qual>const\s*)?\n'
    r'\{',
    re.MULTILINE
)


def extract_template_bodies(cpp_text, expected_classname):
    """
    Find all template serialize/save/load function definitions in cpp_text
    for the given class name.

    Returns list of dicts:
      {
        'funcname': 'serialize'|'save'|'load',
        'params':   original params string (e.g. 'Archive &ar, unsigned'),
        'is_const': bool,
        'body':     text between outer braces (not including the braces),
        'full_match_start': int,   # start of comment line(s)
        'full_match_end':   int,   # one past the closing '}'
      }
    """
    results = []
    for m in TEMPLATE_FUNC_RE.finditer(cpp_text):
        if m.group('classname') != expected_classname:
            continue
        open_brace = m.end() - 1  # position of '{'
        close_brace = find_closing_brace(cpp_text, open_brace)
        body = cpp_text[open_brace + 1: close_brace]
        results.append({
            'funcname': m.group('funcname'),
            'params': m.group('params'),
            'is_const': bool(m.group('const_qual')),
            'body': body,
            'full_match_start': m.start('comment') if m.group('comment') else m.start('template'),
            'full_match_end': close_brace + 1,
        })
    return results


# ---------------------------------------------------------------------------
# Build inline definition for .hpp
# ---------------------------------------------------------------------------

def build_inline_def(funcname, params, is_const, body):
    """
    Build the inline template function definition to insert into the .hpp.

    Produces:
        template <typename Archive>
        void funcname(Archive &ar) const
        {
            body (re-indented)
        }
    """
    # Fix the params string
    clean_params = remove_unsigned_from_sig(params)
    # Normalize: if Archive & with no name, give it 'ar' so the body works
    # (body references 'ar')
    if re.match(r'\s*Archive\s*&\s*,', clean_params) or re.match(r'\s*Archive\s*&\s*$', clean_params.strip()):
        clean_params = re.sub(r'(Archive\s*&)\s*', r'\1ar', clean_params, count=1)

    const_str = ' const' if is_const else ''

    # Fix body: convert ar << / ar >> and strip trailing whitespace
    fixed_body = fix_ar_stream_ops(body)
    # Strip the version-check pattern:
    #   if (version > 0u) { ... } else { ... }
    # simplify to just the 'then' branch
    fixed_body = re.sub(
        r'if\s*\(\s*version\s*[><=!]+\s*\w+\s*\)\s*\{([^}]*)\}\s*else\s*\{[^}]*\}',
        lambda m: m.group(1),
        fixed_body,
        flags=re.DOTALL,
    )

    # Re-indent body lines +4 spaces
    indented_body = re_indent(fixed_body.rstrip('\n'))

    lines = []
    lines.append('    template <typename Archive>')
    lines.append(f'    void {funcname}({clean_params}){const_str}')
    lines.append('    {')
    lines.append(indented_body)
    lines.append('    }')
    return '\n'.join(lines)


# ---------------------------------------------------------------------------
# Patch .hpp: replace declaration with inline definition
# ---------------------------------------------------------------------------

DECL_RE_TEMPLATE = (
    r'([ \t]*)template\s*<\s*typename\s+Archive\s*>\s*\n'
    r'\s*void\s+{funcname}\s*\([^)]*\)\s*(?:const\s*)?;'
)


def patch_hpp(hpp_text, funcname, inline_def):
    """Find the declaration of `funcname` in hpp_text and replace with inline_def."""
    pattern = re.compile(DECL_RE_TEMPLATE.format(funcname=funcname), re.MULTILINE)
    new_text, count = pattern.subn(inline_def, hpp_text)
    if count == 0:
        print(f"  WARNING: declaration of '{funcname}' not found in .hpp", file=sys.stderr)
    elif count > 1:
        print(f"  WARNING: multiple declarations of '{funcname}' found, replaced first", file=sys.stderr)
    return new_text


# ---------------------------------------------------------------------------
# Patch .cpp: remove the template function body
# ---------------------------------------------------------------------------

def patch_cpp(cpp_text, start, end):
    """Remove the template function body from start to end (inclusive)."""
    # Also eat one leading blank line before the block if present
    prefix = cpp_text[:start]
    suffix = cpp_text[end:]
    # Remove trailing newline of block + optional blank line after
    suffix = suffix.lstrip('\n')
    return prefix.rstrip('\n') + '\n\n' + suffix


# ---------------------------------------------------------------------------
# Process a single (.hpp, .cpp) pair
# ---------------------------------------------------------------------------

def process_pair(hpp_path, cpp_path):
    classname = os.path.splitext(os.path.basename(cpp_path))[0]
    # e.g.  bee_colony.cpp  ->  bee_colony

    with open(hpp_path) as f:
        hpp_text = f.read()
    with open(cpp_path) as f:
        cpp_text = f.read()

    bodies = extract_template_bodies(cpp_text, classname)
    if not bodies:
        # Try finding classname via the PAGMO_S11N_*_IMPLEMENT macro
        m = re.search(r'PAGMO_S11N_\w+_IMPLEMENT\(pagmo::(\w+)\)', cpp_text)
        if m:
            classname = m.group(1)
            bodies = extract_template_bodies(cpp_text, classname)

    if not bodies:
        print(f"  No template serialization bodies found in {cpp_path}")
        return False

    print(f"  {cpp_path}: found {len(bodies)} function(s): {[b['funcname'] for b in bodies]}")

    # Sort in reverse order so we can splice cpp_text without offset invalidation
    bodies_sorted = sorted(bodies, key=lambda b: b['full_match_start'], reverse=True)

    modified_hpp = hpp_text
    modified_cpp = cpp_text

    for body_info in bodies_sorted:
        inline_def = build_inline_def(
            body_info['funcname'],
            body_info['params'],
            body_info['is_const'],
            body_info['body'],
        )
        modified_hpp = patch_hpp(modified_hpp, body_info['funcname'], inline_def)
        modified_cpp = patch_cpp(modified_cpp, body_info['full_match_start'], body_info['full_match_end'])

    # Final global cleanup on cpp: remove double blank lines
    modified_cpp = re.sub(r'\n{3,}', '\n\n', modified_cpp)

    with open(hpp_path, 'w') as f:
        f.write(modified_hpp)
    with open(cpp_path, 'w') as f:
        f.write(modified_cpp)

    return True


# ---------------------------------------------------------------------------
# Also fix ar << / ar >> remaining in .hpp files (not_population_based, archipelago)
# ---------------------------------------------------------------------------

def fix_hpp_ar_ops(hpp_path):
    with open(hpp_path) as f:
        text = f.read()
    new_text = fix_ar_stream_ops(text)
    if new_text != text:
        with open(hpp_path, 'w') as f:
            f.write(new_text)
        print(f"  Fixed ar << / ar >> in {hpp_path}")


# ---------------------------------------------------------------------------
# Also remove 'unsigned' from all remaining .hpp save/load/serialize declarations
# and .cpp definitions that weren't caught above
# ---------------------------------------------------------------------------

def fix_unsigned_in_file(path):
    with open(path) as f:
        text = f.read()
    new_text = re.sub(
        r'((?:serialize|save|load)\s*\([^)]*)(,\s*unsigned(?:\s+\w+)?\s*)(\))',
        r'\1\3',
        text,
    )
    if new_text != text:
        with open(path, 'w') as f:
            f.write(new_text)
        print(f"  Removed 'unsigned' param from {path}")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

PAGMO_DIR = '/home/jay/projects/pagmo2/pagmo'

SUBDIRS = ['algorithms', 'problems', 'islands', 'batch_evaluators', 'topologies',
           'r_policies', 's_policies']


def main():
    print("=== Moving serialization bodies from .cpp to .hpp ===\n")

    # 1. Process each UDX subdir
    for subdir in SUBDIRS:
        subdir_path = os.path.join(PAGMO_DIR, subdir)
        if not os.path.isdir(subdir_path):
            continue
        cpp_files = sorted(glob.glob(os.path.join(subdir_path, '*.cpp')))
        for cpp_path in cpp_files:
            base = os.path.splitext(cpp_path)[0]
            hpp_path = base + '.hpp'
            if not os.path.exists(hpp_path):
                print(f"  Skipping {cpp_path}: no matching .hpp found")
                continue
            process_pair(hpp_path, cpp_path)

    print("\n=== Fixing ar << / ar >> in standalone .hpp files ===\n")

    # 2. Fix ar << / ar >> in files that have inline save/load bodies already
    standalone_hpps = [
        os.path.join(PAGMO_DIR, 'algorithms', 'not_population_based.hpp'),
        os.path.join(PAGMO_DIR, 'archipelago.hpp'),
    ]
    for p in standalone_hpps:
        if os.path.exists(p):
            fix_hpp_ar_ops(p)

    print("\n=== Removing remaining 'unsigned' params in all pagmo files ===\n")

    # 3. Final sweep: remove any remaining 'unsigned' version params
    all_files = (
        glob.glob(os.path.join(PAGMO_DIR, '**', '*.hpp'), recursive=True) +
        glob.glob(os.path.join(PAGMO_DIR, '**', '*.cpp'), recursive=True)
    )
    for p in all_files:
        fix_unsigned_in_file(p)

    print("\nDone.")


if __name__ == '__main__':
    main()
