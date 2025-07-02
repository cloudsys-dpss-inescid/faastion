import os
import sys

cur_dir = os.path.dirname(os.path.realpath(__file__))

dupes = {}
dupe_names = {
    'lockf', # for some reason lockf appears twice in the AST
    'lockf64',
}
ignore_names = [
    'malloc',
    'realloc',
    'calloc',
    'free',
    'getenv',
    'secure_getenv',
    'pthread_create',
    'localtime',
    'dlsym',
    'ntp_gettimex',                                 # assembler message: already defined
    'sched_yield',                                  #
    'pthread_mutex_consistent',                     #
    'pthread_mutexattr_getrobust',                  #
    'pthread_mutexattr_setrobust',                  #
    'setjmp',       # setjmp is alias for _setjmp
    'basename',     # basename is alias for __xpg_basename
    'isalnum',  # strange compiler error
    'isalpha',
    'iscntrl',
    'isdigit',
    'islower',
    'isgraph',
    'isprint',
    'ispunct',
    'isspace',
    'isupper',
    'isxdigit',
    'isblank',
    'isascii',
    'toascii',
    '_toupper',
    '_tolower',
    'isalnum_l',
    'isalpha_l',
    'iscntrl_l',
    'isdigit_l',
    'islower_l',
    'isgraph_l',
    'isprint_l',
    'ispunct_l',
    'isspace_l',
    'isupper_l',
    'isxdigit_l',
    'isblank_l',
    'obstack_free',
]

def parse_input():
    if len(sys.argv) < 2:
	    sys.exit("Sytanx: " + sys.argv[0] + " <filename>")

def parse_file(lines):
    # for i in range(len(lines)):
    #     line = lines[i]
    #     if line.startswith("used") or line.startswith("space>") or line.startswith("implicit"):
    #         lines[i] = line[line.find(' ') + 1:]
    return list(filter(lambda line: not (line.endswith("static inline") or '...' in line), lines))

def get_name(line):
    return line[:line.find(' ')]

def get_ret_type(line):
    signature_start = line.find('\'') + 1
    params_start = line[signature_start:].find('(') + signature_start
    return line[signature_start:params_start]

def get_params(line):
    open_params = line.find('(') + 1
    end_params = line.find(')\'')
    if '__attribute__' in line[open_params:end_params]:
        end_params = line.find(') __attribute__')
    return line[open_params:end_params]

def get_param(arg_type, name):
    if '(*' in arg_type:
        function_pointer_start = arg_type.find('(*')
        function_pointer_end = arg_type[function_pointer_start:].find(')') + function_pointer_start
        return arg_type[:function_pointer_start] + ' (*' + name + arg_type[function_pointer_end:]
    if '__va_list_tag' in arg_type:
        return 'va_list ' + name
    else:
        return arg_type + ' ' + name

def get_complex(param, name):    
    if '_Complex' in param and name.endswith('f32'):
        return '_Complex _Float32'
    elif '_Complex' in param and name.endswith('f32x'):
        return '_Complex _Float32x'
    elif '_Complex' in param and name.endswith('f64'):
        return '_Complex _Float64'
    elif '_Complex' in param and name.endswith('f64x'):
        return '_Complex _Float64x'
    else:
        return param

def get_param_list(params):
    if params == '':
        return []
    final_list = []
    param_list = params.split(',')
    aux = []
    i = 0
    while i < len(param_list):
        if '(*' in param_list[i]: # this is a function pointer, we need to skip the function parameters 
            aux.append(param_list[i])
            param = param_list[i]
            if ')' not in param[param.find('(*') + 3:]:
                i += 1
                aux.append(param_list[i])
            while ')' not in param_list[i]:
                i += 1
                aux.append(param_list[i])
            final_list.append(', '.join(aux))
            aux = []
        else:
            final_list.append(param_list[i])
        i += 1
    return final_list

def new_wrapper(name, ret_type, params):
    args = []
    param_list = get_param_list(params) 
    arg_cnt = len(param_list)
    for i in range(arg_cnt):
        args.append('arg' + str(i))

    if params.strip().endswith('void') or arg_cnt == 0:
        argnames = ''
    else:
        argnames = ', '.join(args)
        params = ', '.join([get_param(get_complex(param_list[i], name), args[i]) for i in range(arg_cnt)])

    ret_type = get_complex(ret_type, name)
    original_name = 'original_' + name
    
    if ret_type.strip().endswith('void'):
        code = f'''
{ret_type} (*{original_name})({params}) = NULL;

{ret_type} {name}({params}) {{
    unsigned int privileged_domain;
    unsigned int unprivileged_domain;
    
    unprivileged_domain = __rdpkru();
    privileged_domain = unprivileged_domain & 0x55555554;

    __wrpkrumem(privileged_domain);
    char buf[] = "[libc] {name}\\n";
    syscall(__NR_write, 2, buf, sizeof(buf));
    lookup_symbol({name});
    __wrpkrumem(unprivileged_domain);
    (*{original_name})({argnames});
}}  
'''
    else:
        code = f'''
{ret_type} (*{original_name})({params}) = NULL;

{ret_type} {name}({params}) {{
    unsigned int privileged_domain;
    unsigned int unprivileged_domain;
    
    unprivileged_domain = __rdpkru();
    privileged_domain = unprivileged_domain & 0x55555554;

    __wrpkrumem(privileged_domain);
    char buf[] = "[libc] {name}\\n";
    syscall(__NR_write, 2, buf, sizeof(buf));
    lookup_symbol({name});
    __wrpkrumem(unprivileged_domain);
    {ret_type} ret = (*{original_name})({argnames});
    return ret;
}}
'''

    init = f'''
    if ({original_name} == NULL) {{
        {original_name} = dlsym(RTLD_NEXT, "{name}");
    }}
'''

    return code, init

def generate_wrappers(lines):
    for name in dupe_names:
        dupes[name] = False
    file = open(os.path.join(cur_dir, 'loader', 'libc_wrappers.c'), 'a')
    file.write('#define lookup_symbol(sym) if (original_##sym == NULL) original_##sym = dlsym(RTLD_NEXT, #sym)\n')
    init_func = '''
void init() {'''
    for line in lines:
        name = get_name(line)

        if name in dupes:
            if dupes[name]:
                continue
            dupes[name] = True
        elif name in ignore_names:
            continue

        ret_type = get_ret_type(line)
        params = get_params(line)

        if name == 'strerror_r': # for some reason the return type does not match the libc type
            ret_type = 'char *'
        elif name == 'basename':
            params = 'char *'

        wrapper, init = new_wrapper(name, ret_type, params)
        file.write(wrapper)
        # init_func += init
    init_func += '}'
    file.write(init_func)
    file.close

if __name__ == "__main__":
    parse_input()

    file = open(sys.argv[1])
    lines = file.read().split('\n')[:-1]
    file.close()

    lines = parse_file(lines)
    generate_wrappers(lines)
