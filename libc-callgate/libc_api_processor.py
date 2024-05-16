#!/usr/bin/env python3

import sys

def parse_fdecl(line):
    # Remove the new line.
    line = line.strip()
    # line: struct utmp *getutid(const struct utmp *, const struct utmp *);
    splits = line.split('(');
    # splits[0]: struct utmp *getutid
    # splits[1]: const struct utmp *, const struct utmp *);
    args = splits[1].split(')')[0]
    # args: const struct utmp *, const struct utmp *
    args = args.split(',')
    # args: ['const struct utmp *', 'const struct utmp *']
    name = splits[0].strip().split()[-1]
    # name: *getutid
    name = name.replace('*' ,'')
    # name: getutid
    ret = ''.join(splits[0].rsplit(name, 1)).strip()
    # ret: struct utmp *
    return ret, name, args

def gen_original_decl(ret, name, args):
    args = ','.join(args)

    # gcc issues a warning if this modifier is included in the declaration.
    if "_Noreturn" in ret:
        ret = "void"

    print(f'{ret} (*original_{name})({args}) = 0;')

def gen_wrapper(ret, name, args):
    def prepare_args(args):
        argswnames = []
        argsnames = []
        for arg in args:
            # In cases like this:
            #    `char *const []`, the correct placement is
            #    `char *const ai []` and not `char *const [] a1`
            if arg.endswith(']'):
                argswnames.append(arg.replace('[', 'a' + str(len(argswnames)) + ' ['))
            else:
                argswnames.append(arg + ' a' + str(len(argswnames)))
            argsnames.append('a' + str(len(argsnames)))
        argswnames = ', '.join(argswnames)
        argsnames = ', '.join(argsnames)
        return argswnames, argsnames

    # Some functions are like this: void tzset(void);
    if len(args) == 1 and args[0] == "void":
        argswnames = 'void'
        argsnames = ''
    else:
        argswnames, argsnames = prepare_args(args)
    original_name = 'original_' + name

    if ret.endswith('void'):
        code = f'''
{ret} {name}({argswnames}) {{
    int pkru = __rdpkru();
    if (pkru) {{
        __wrpkru(0x0);
    }}

    if ({original_name} == 0) {{
        {original_name} = dlsym(RTLD_NEXT, "{name}");
    }}

    (*{original_name})({argsnames});

    if (pkru) {{
        __wrpkrumem(pkru);
    }}
}}
'''
    else:
        code = f'''
{ret} {name}({argswnames}) {{
    int pkru = __rdpkru();
    if (pkru) {{
        __wrpkru(0x0);
    }}

    if ({original_name} == 0) {{
        {original_name} = dlsym(RTLD_NEXT, "{name}");
    }}

    {ret} ret = (*{original_name})({argsnames});

    if (pkru) {{
        __wrpkrumem(pkru);
    }}

    return ret;
}}
'''

    print(code)

# TODO - use pycparse to parse function declarations?
processed = []
ignore = [
        'dlsym',
        'vfprintf',
        'pthread_sandbox_init',
        'pthread_sandbox_call',
        'pthread_sandbox_enter',
        'pthread_sandbox_leave',
        'malloc',
        'free' ] # TODO - we should check for recursive calls in the preloaded implementation.


# Preamble.
print('''
#define RTLD_NEXT    ((void *)-1)
#include "libc_callgate_pkru.h"''')

for line in sys.stdin:
    # Ignore functions with function pointer arguments.
    if '(*)' in line:
        print ('// Ignoring function w/ function arguments: ' + line.strip())
        continue
    # Ignore variadic functions.
    # Note: Variadic functions are notiriously hard to wrap: https://c-faq.com/varargs/handoff.html
    if '...' in line:
        print ('// Ignoring variadic function: ' + line.strip())
        continue
    # Ignore multi-line function declarations.
    if not line.strip().endswith(';'):
        continue

    ret, name, args = parse_fdecl(line)
    if name not in processed and name not in ignore:
        gen_original_decl(ret, name, args)
        gen_wrapper(ret, name, args)
        processed.append(name)