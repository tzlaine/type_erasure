#!/usr/bin/env python

import clang
from clang.cindex import Index

import argparse
import re
import sys


indent_spaces = 4
indentation = ' ' * indent_spaces
output = ['']

class client_data:
    def __init__(self):
        self.tu = None
        self.current_namespaces = [] # cursors
        self.current_struct = null_cursor
        self.current_struct_prefix = ''
        # function signature, forwarding call arguments, optional return
        # keyword, function name, and "const"/"" for function constness
        self.member_functions = [] # each element [''] * 5
        self.printed_headers = False
        self.filename = ''
        self.include_guarded = False
        self.form = ''
        self.form_lines = []
        self.headers = ''
        self.copy_on_write = False

def get_tokens (tu, cursor):
    return [x for x in tu.get_tokens(extent=cursor.extent)]

def print_tokens (tu, cursor, tokens_from_include_directive):
    tokens = get_tokens(tu, cursor)
    open_angle = '<'
    open_angle_seen = False
    for token in tokens:
        spelling = token.spelling
        if not open_angle_seen:
            output[0] += ' '
        output[0] += spelling
        if token == open_angle:
            open_angle_seen = True
    output[0] += '\n'

def struct_kind (kind):
    if kind == clang.cindex.CursorKind.CLASS_DECL or \
       kind == clang.cindex.CursorKind.STRUCT_DECL or \
       kind == clang.cindex.CursorKind.CLASS_TEMPLATE or \
       kind == clang.cindex.CursorKind.CLASS_TEMPLATE_PARTIAL_SPECIALIZATION:
        return True
    else:
        return False

def indent (offset=0):
    size = len(data.current_namespaces) + offset
    return ' ' * (size * indent_spaces)

def print_lines (lines):
    for line in lines:
        if line != '':
            output[0] += indent() + indentation + lines[i] + '\n'
        else:
            output[0] += '\n'

def print_headers ():
    if data.printed_headers:
        return
    output[0] += data.headers + '\n'
    data.printed_headers = True

def struct_prefix (struct_cursor):
    retval = ''
    tokens = get_tokens(data.tu, struct_cursor)
    open_brace = '{'
    struct_ = 'struct'
    class_ = 'class'

    for i in range(len(tokens)):
        spelling = tokens[i].spelling
        if spelling == open_brace:
            break
        if spelling == struct_ or spelling == class_:
            retval += '\n' + indent(-1)
        elif i:
            retval += ' '
        retval += spelling

    return retval

def member_params (cursor):
    tokens = get_tokens(data.tu, cursor)

    open_brace = '{'
    semicolon = ';'
    close_paren = ')'
    const_token = 'const'

    str = ''
    constness = ''

    close_paren_seen = False
    for i in range(len(tokens)):
        spelling = tokens[i].spelling
        if close_paren_seen and spelling == const_token:
            constness = 'const'
        if spelling == close_paren:
            close_paren_seen = True
        if spelling == open_brace or spelling == semicolon:
            break
        if i:
            str += ' '
        str += spelling

    args = [x for x in cursor.get_arguments()]
    args_str = ''

    for i in range(len(args)):
        if i:
            args_str += ', '
        arg_cursor = args[i]
        args_str += arg_cursor.spelling

    return_str = cursor.result_type.kind != clang.cindex.TypeKind.VOID and 'return ' or ''

    function_name = cursor.spelling

    return [str, args_str, return_str, function_name, constness]

def indent_lines (lines):
    regex = re.compile(r'\n')
    indentation = indent()
    return regex.sub('\n' + indentation, indentation + lines)

def find_expansion_lines (lines):
    retval = [0] * 3
    for i in range(len(lines)):
        line = lines[i]
        try:
            nonvirtual_pos = line.index('{nonvirtual_members}')
        except:
            nonvirtual_pos = -1
        try:
            pure_virtual_pos = line.index('{pure_virtual_members}')
        except:
            pure_virtual_pos = -1
        try:
            virtual_pos = line.index('{virtual_members}')
        except:
            virtual_pos = -1
        if nonvirtual_pos != -1:
            retval[0] = (i, nonvirtual_pos)
        elif pure_virtual_pos != -1:
            retval[1] = (i, pure_virtual_pos)
        elif virtual_pos != -1:
            retval[2] = (i, virtual_pos)
    return retval

def close_struct ():
    lines = data.form_lines

    expansion_lines = find_expansion_lines(lines)

    lines = map(
        lambda line: line.format(
            struct_prefix=data.current_struct_prefix,
            struct_name=data.current_struct.spelling,
            nonvirtual_members='{nonvirtual_members}',
            pure_virtual_members='{pure_virtual_members}',
            virtual_members='{virtual_members}'
        ),
        lines
    )

    nonvirtual_members = ''
    pure_virtual_members = ''
    virtual_members = ''

    for function in data.member_functions:
        if data.copy_on_write:
            nonvirtual_members += \
                indentation + function[0] + '\n' + \
                indentation + '{ assert(handle_); ' + function[2] + \
                (function[4] == 'const' and 'read().' or 'write().') + \
                function[3] + '(' + function[1] + ' ); }\n'
        else:
            nonvirtual_members += \
                indentation + function[0] + '\n' + \
                indentation + '{ assert(handle_); ' + function[2] + \
                'handle_->' + function[3] + \
                '(' + function[1] + ' ); }\n'

        pure_virtual_members += \
            indentation * 2 + 'virtual ' + function[0] + ' = 0;\n'

        virtual_members += \
            indentation * 2 + 'virtual ' + function[0] + '\n' + \
            indentation * 2 + '{ ' + function[2] + \
            'value_.' + function[3] + \
            '(' + function[1] + ' ); }\n'

    nonvirtual_members = nonvirtual_members[:-1]
    pure_virtual_members = pure_virtual_members[:-1]
    virtual_members = virtual_members[:-1]

    lines[expansion_lines[0][0]] = nonvirtual_members
    lines[expansion_lines[1][0]] = pure_virtual_members
    lines[expansion_lines[2][0]] = virtual_members

    output[0] += '\n'
    for line in lines:
        output[0] += indent_lines(line) + '\n'

def open_namespace (namespace_):
    output[0] += '\n' + indent() + 'namespace ' + namespace_.spelling + ' {'

def close_namespace ():
    output[0] += '\n' + indent() + '}\n'

class child_visit:
    Break = 0
    Continue = 1
    Recurse = 2

def visit_impl (cursor, parent):
    # close open namespaces we have left
    enclosing_namespace = parent
    while enclosing_namespace != data.tu.cursor and \
          enclosing_namespace.kind != clang.cindex.CursorKind.NAMESPACE:
        enclosing_namespace = enclosing_namespace.semantic_parent

    if enclosing_namespace != data.tu.cursor and \
       enclosing_namespace.kind == clang.cindex.CursorKind.NAMESPACE:
        while len(data.current_namespaces) and \
              enclosing_namespace != data.current_namespaces[-1]:
            data.current_namespaces.pop()
            close_namespace()

    # close open struct if we have left it
    enclosing_struct = parent
    while enclosing_struct and \
          enclosing_struct != data.tu.cursor and \
          not struct_kind(enclosing_struct.kind):
        enclosing_struct = enclosing_struct.semantic_parent

    if enclosing_struct and \
       data.current_struct != null_cursor and \
       enclosing_struct != data.current_struct:
        close_struct()
        data.current_struct = null_cursor
        data.member_functions = []

    location = cursor.location
    from_main_file_ = from_main_file(location)

    kind = cursor.kind
    if kind == clang.cindex.CursorKind.NAMESPACE:
        if from_main_file_:
            print_headers()
            open_namespace(cursor)
            data.current_namespaces.append(cursor)
        return child_visit.Recurse
    elif not from_main_file_:
        return child_visit.Continue
    elif struct_kind(kind):
        if data.current_struct == null_cursor:
            print_headers()
            data.current_struct = cursor
            data.current_struct_prefix = struct_prefix(cursor)
            return child_visit.Recurse
    elif kind == clang.cindex.CursorKind.CXX_METHOD:
        data.member_functions.append(member_params(cursor))

    return child_visit.Continue

def visit (cursor, parent=None):
    for child in cursor.get_children():
        result = visit_impl(child, cursor)
        if result == child_visit.Recurse:
            if visit(child, cursor) == child_visit.Break:
                return child_visit.Break
        elif result == child_visit.Break:
            return child_visit.Break
        elif result == child_visit.Continue:
            continue

manual = '''emtypen Users' Manual

emtypen generates type erasure C++ code.  It does this to automate much of the
drudgery of creating such types by hand.

Some of this might not make sense if you don't know how type erasure works.
See http://tzlaine.github.io/type_erasure if this is the case.

At the highest level of abstraction, emtypen takes three input files
containing code and generates a single output source file.  It uses libclang,
a wrapper around the Clang front end, to do this.

The three input files are the "archetype" file, the "form" file, and the
"header" file.  The archetype must always be specified.  There are implicit
defaults for the form and header.


The Archetype File

The archetype file contains one or more structs, struct templates, classes
and/or class templates (hereafter generically referred to just as
"archetypes"). Archetypes that are templates produce generated types
("erased types" hereafter) that are also templates.

Each archetype defines the public API that the erased type requires of all the
types that it can hold.  The erased type will also contain all the
contructors, assignment operators and other operators defined in the form
provided.  It is an error to define any of these fundamental operations in the
archetype; they go in the form instead.  Here is an example archetype file:

#ifndef LOGGABLE_INTERFACE_INCLUDED__
#define LOGGABLE_INTERFACE_INCLUDED__

#include <iostream>


struct loggable
{
    std::ostream & log (std::ostream & os) const;
};

#endif

Note that this is a complete and valid C++ header.  You can syntax check it
with your favorite compiler if you like.  emtypen will preserve the include
guard, if any, include directives, if any, and the namespaces in which the
archetypes are declared, if any.

IMPORTANT: Give each function parameter a name.  If the parameters in an
archetype's functions are left unnamed, the generated forwarding functions
will be malformed.

Due to libclang limitations, macros and comments are not preserved.

Declarations other than the ones listed above are not preserved (for instance,
function declarations).


The Form File

The form file contains a template-like form that gets filled in with
repetitive code generated from an archetype.  The form will be repeated in the
output once for each archetype.

There are certain magic strings in the form that are replaced with generated
code.  If you want to create a new form or modify an existing one, you need to
include:

%struct_prefix% - This is replaced with the tokens that introduce the
archetype by name, along with "struct", "class", "template <...>" etc.

%struct_name% - This is replaced with only the archetype's name.

%nonvirtual_members% - This is the generated portion of the API of the erased
type.  It is replaced with a version of the functions in the archetype that
forwards each call to the virtual functions in the handle object.

%pure_virtual_members% - This is the generated portion of the API of the
handle base class. It is replaced with pure virtual declarations of the
functions in the archetype.

%virtual_members% - This is the generated portion of the API of the derived
handle class. It is replaced with virtual function definitions of the
functions in the archetype that forward to the underlying held value.

Within the constraints implied by the pattern of code generation outlined
above, the form can include anything you like.

However, the forwarding function code generation needs to know if your form
uses copy-on-write in order to perform the copy on mutating function calls.
If you specify on the command line that emtypen should generate code usable
with a copy-on-write form (see emtypen --help for details), the generated code
will rely on two functions that must be in the form: read() and write().  They
must return const and non-const references respectively to the underlying
handle.  They may be public or private.  read() will be called in every const
member function in the archetype's API, and write() will be called in every
non-const member function.


The Header file

The header file should contain all headers, macros, forward declarations,
etc. required by the code in the form.  Headers required by the code in an
archetype file should be included there, not in the header file.


Command Line Options

An alternate form file and/or header file can be specified on the command
line.  Also, you will probably need to generate slightly different code for
forms that use copy-on-write.  See emtypen --help for details.

'''

def prepare_form_impl (form):
    form = form.replace('{', '{{')
    form = form.replace('}', '}}')
    regex = re.compile(r'%(\w+)%')
    return regex.sub(r'{\1}', form)[:-1]

def prepare_form (form):
    if type(form) == str:
        return prepare_form_impl(form)
    else:
        for i in range(len(form)):
            form[i] = prepare_form_impl(form[i])
        return form

# main

if '--manual' in sys.argv:
    print manual
    exit(0)

parser = argparse.ArgumentParser(description='Generates type erased C++ code.')
parser.add_argument('--form', type=str, required=True, help='form used to generate code')
parser.add_argument('--headers', type=str, required=False, help='file containing headers to prepend to the generated code')
parser.add_argument('--copy-on-write', type=str, required=False, help='generate code suitable for a COW implementation')
parser.add_argument('--out-file', type=str, required=False, help='write output to given file')
parser.add_argument('--clang-path', type=str, required=False, help='path to libclang library')
parser.add_argument('--manual', action='store_true', required=False, help='print a much longer manual to the terminal')
parser.add_argument('file', type=str, help='the input file containing archetypes')
parser.add_argument('clang_args', metavar='Clang-arg', type=str, nargs=argparse.REMAINDER,
                    help='additional args to pass to Clang')
args = parser.parse_args()

if args.clang_path:
    clang.cindex.Config.set_library_path(args.clang_path)

null_cursor = clang.cindex.conf.lib.clang_getNullCursor()
from_main_file = clang.cindex.conf.lib.clang_Location_isFromMainFile

data = client_data()

data.form = prepare_form(open(args.form).read())
data.form_lines = prepare_form(open(args.form).readlines())

data.headers = args.headers and open(args.headers).read() or ''

include_guarded = False
archetypes = open(args.file).read()
archetypes_lines = open(args.file).readlines()
guard_regex = re.compile(r'#ifndef\s+([^\s]+)[^\n]*\n#define\s+\1')
match = guard_regex.search(archetypes)
if match and match.start() == archetypes.index('#'):
    include_guarded = True
    output[0] += '''#ifndef {0}
#define {0}

'''.format(match.group(1))

all_clang_args = [args.file]
all_clang_args.extend(args.clang_args)

index = Index.create()
data.tu = index.parse(None, all_clang_args, options=clang.cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
data.filename = data.tu.spelling

if data.filename == '':
    exit(1)

includes = [archetypes_lines[x.location.line - 1] for x in data.tu.get_includes() if x.depth == 1]
for include in includes:
    output[0] += include

visit(data.tu.cursor)

if data.current_struct != null_cursor:
    close_struct()

while len(data.current_namespaces):
    data.current_namespaces.pop()
    close_namespace()

if include_guarded:
    output[0] += '#endif\n'

if not args.out_file:
    print output[0]
else:
    ofs = open(args.out_file, 'w')
    ofs.write(output[0])
