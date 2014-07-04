#include <file_utils.hpp>

#include <clang-c/Index.h>

#include <array>
#include <iostream>
#include <vector>


namespace {

    const unsigned int indent_spaces = 4;
    const std::string indentation(indent_spaces, ' ');

}

CXCursor tu_cursor;

struct client_data
{
    CXTranslationUnit tu;
    std::vector<CXCursor> current_namespaces;
    CXCursor current_struct;
    std::string current_struct_prefix;
    // function signature, forwarding call arguments, optional return
    // keyword, function name, and "const"/"" for function constness
    std::vector<std::array<std::string, 5>> member_functions;
    bool printed_headers;
    const char * filename;
    bool include_guarded;
    std::string form;
    std::string headers;
    bool copy_on_write;
};

std::pair<CXToken*, unsigned int>
get_tokens (CXTranslationUnit tu, CXCursor cursor)
{
    CXSourceRange range = clang_getCursorExtent(cursor);
    CXSourceLocation start = clang_getRangeStart(range);
    CXSourceLocation end = clang_getRangeEnd(range);

    std::pair<CXToken*, unsigned int> retval(0, 0);
    clang_tokenize(tu, range, &retval.first, &retval.second);

    return retval;
}

void
free_tokens (CXTranslationUnit tu, std::pair<CXToken*, unsigned int> tokens)
{ clang_disposeTokens(tu, tokens.first, tokens.second); }

void print_tokens (CXTranslationUnit tu,
                   CXCursor cursor,
                   bool tokens_from_include_directive)
{
    std::pair<CXToken*, unsigned int> tokens = get_tokens(tu, cursor);

    const std::string open_angle = "<";

    const unsigned int num_tokens =
        tokens.second && tokens_from_include_directive ?
        tokens.second - 1 :
        tokens.second;
    bool open_angle_seen = false;
    for (unsigned int i = 0; i < num_tokens; ++i) {
        CXString spelling = clang_getTokenSpelling(tu, tokens.first[i]);
        const char * token = clang_getCString(spelling);
        if (!open_angle_seen)
            std::cout << " ";
        std::cout << token;
        if (token == open_angle)
            open_angle_seen = true;
    }
    std::cout << "\n";

    free_tokens(tu, tokens);
}

bool struct_kind (CXCursorKind kind)
{
    switch (kind) {
    case CXCursor_ClassDecl:
    case CXCursor_StructDecl:
    case CXCursor_ClassTemplate:
    case CXCursor_ClassTemplatePartialSpecialization:
        return true;
    default:
        return false;
    }
    return false;
}

std::string indent (const client_data & data)
{
    std::size_t size = data.current_namespaces.size();
    return std::string(size * indent_spaces, ' ');
}

void print_lines (const client_data & data,
                  const char ** lines,
                  std::size_t num_lines)
{
    for (unsigned int i = 0; i < num_lines; ++i) {
        if (lines[i])
            std::cout << indent(data) << indentation << lines[i] << "\n";
        else
            std::cout << "\n";
    }
}

void print_headers (client_data & data)
{
    if (data.printed_headers)
        return;

    std::cout << data.headers << "\n";

    data.printed_headers = true;
}

std::string struct_prefix (const client_data & data, CXCursor struct_cursor)
{
    std::string retval;

    std::pair<CXToken*, unsigned int> tokens =
        get_tokens(data.tu, struct_cursor);

    const std::string open_brace = "{";
    const std::string struct_ = "struct";
    const std::string class_ = "class";

    for (unsigned int i = 0; i < tokens.second; ++i) {
        CXString spelling = clang_getTokenSpelling(data.tu, tokens.first[i]);

        const char * c_str = clang_getCString(spelling);

        if (c_str == open_brace)
            break;

        if (c_str == struct_ || c_str == class_) {
            retval += "\n";
            retval += indent(data);
        } else if (i) {
            retval += " ";
        }

        retval += c_str;
    }

    free_tokens(data.tu, tokens);

    return retval;
}

std::array<std::string, 5>
member_params (const client_data & data, CXCursor cursor)
{
    std::pair<CXToken*, unsigned int> tokens = get_tokens(data.tu, cursor);

    const std::string open_brace = "{";
    const std::string semicolon = ";";
    const std::string close_paren = ")";
    const std::string const_token = "const";

    std::string str;
#if 0 // TODO: This is the Clang 3.5 way of doing things...
    std::string constness = clang_CXXMethod_isConst(cursor) ? "const" : "";
#else
    std::string constness;
#endif

    bool close_paren_seen = false;
    for (unsigned int i = 0; i < tokens.second; ++i) {
        CXString spelling =
            clang_getTokenSpelling(data.tu, tokens.first[i]);

        const char * c_str = clang_getCString(spelling);

        if (close_paren_seen && c_str == const_token)
            constness = "const";

        if (c_str == close_paren)
            close_paren_seen = true;

        if (c_str == open_brace || c_str == semicolon)
            break;

        if (i)
            str += " ";

        str += c_str;
    }

    std::string args;

    const int num_args = clang_Cursor_getNumArguments(cursor);
    for (int i = 0; i < num_args; ++i) {
        if (i)
            args += ", ";
        CXCursor arg_cursor = clang_Cursor_getArgument(cursor, i);
        args += clang_getCString(clang_getCursorSpelling(arg_cursor));
    }

    const char * return_str =
        clang_getCursorResultType(cursor).kind == CXType_Void ?
        "" :
        "return ";

    const char * function_name =
        clang_getCString(clang_getCursorSpelling(cursor));

    free_tokens(data.tu, tokens);

    return {str, args, return_str, function_name, constness};
}

void indent (std::vector<std::string> & lines, const client_data & data)
{
    for (std::string & line : lines) {
        line = indent(data) + line;
    }
}

void replace (std::string & line,
              const std::string & tag,
              const std::string & value)
{
    std::string copy;
    while (true) {
        std::string::size_type pos = line.find(tag);
        if (pos == std::string::npos)
            break;
        copy = line.substr(0, pos);
        copy += value;
        copy += line.substr(pos + tag.size());
        copy.swap(line);
    }
}

void replace (std::vector<std::string> & lines,
              const std::string & tag,
              const std::string & value)
{
    for (std::string & line : lines) {
        replace(line, tag, value);
    }
}

std::array<std::pair<int, std::size_t>, 3>
find_expansion_lines (const std::vector<std::string> & lines)
{
    std::array<std::pair<int, std::size_t>, 3> retval = {};
    std::size_t i = 0;
    for (const std::string & line : lines) {
        std::string::size_type pos;
        if ((pos = line.find("%nonvirtual_members%")) !=
            std::string::npos) {
            retval[0] = std::pair<int, std::size_t>(i, pos);
        } else if ((pos = line.find("%pure_virtual_members%")) !=
                   std::string::npos) {
            retval[1] = std::pair<int, std::size_t>(i, pos);
        } else if ((pos = line.find("%virtual_members%")) !=
                   std::string::npos) {
            retval[2] = std::pair<int, std::size_t>(i, pos);
        }
        ++i;
    }
    return retval;
}

void close_struct (const client_data & data)
{
    std::vector<std::string> lines = line_break(data.form);

    std::array<std::pair<int, std::size_t>, 3> expansion_lines =
        find_expansion_lines(lines);

    indent(lines, data);

    replace(lines, "%struct_prefix%", data.current_struct_prefix);

    replace(lines,
            "%struct_name%",
            clang_getCString(clang_getCursorSpelling(data.current_struct)));

    std::string nonvirtual_members;
    std::string pure_virtual_members;
    std::string virtual_members;

    for (const auto & function : data.member_functions) {
        std::string spacing;

        spacing = std::string(expansion_lines[0].second, ' ') + indent(data);

        if (data.copy_on_write) {
            nonvirtual_members +=
                spacing + function[0] + "\n" +
                spacing + "{ assert(handle_); " + function[2] +
                (function[4] == "const" ? "read()." : "write().") +
                function[3] + "(" + function[1] + " ); }\n";
        } else {
            nonvirtual_members +=
                spacing + function[0] + "\n" +
                spacing + "{ assert(handle_); " + function[2] +
                "handle_->" + function[3] +
                "(" + function[1] + " ); }\n";
        }

        spacing = std::string(expansion_lines[1].second, ' ') + indent(data);
        pure_virtual_members +=
            spacing + "virtual " + function[0] + " = 0;\n";

        spacing = std::string(expansion_lines[2].second, ' ') + indent(data);
        virtual_members +=
            spacing + "virtual " + function[0] + "\n" +
            spacing + "{ " + function[2] +
            "value_." + function[3] +
            "(" + function[1] + " ); }\n";
    }

    nonvirtual_members.resize(nonvirtual_members.size() - 1);
    pure_virtual_members.resize(pure_virtual_members.size() - 1);
    virtual_members.resize(virtual_members.size() - 1);

    lines[expansion_lines[0].first] = nonvirtual_members;
    lines[expansion_lines[1].first] = pure_virtual_members;
    lines[expansion_lines[2].first] = virtual_members;

    std::cout << "\n";
    for (std::string & line : lines) {
        std::cout << line << "\n";
    }
}

void open_namespace (const client_data & data, CXCursor namespace_)
{
    std::cout
        << "\n"
        << indent(data)
        << "namespace "
        << clang_getCString(clang_getCursorSpelling(namespace_))
        << " {";
}

void close_namespace (const client_data & data)
{ std::cout << "\n" << indent(data) << "}\n"; }

CXChildVisitResult
visit_preprocessor_defines (CXCursor cursor, CXCursor parent, CXClientData data_)
{
    client_data & data = *static_cast<client_data*>(data_);

    if (!data.include_guarded)
        return CXChildVisit_Break;

    CXSourceLocation location = clang_getCursorLocation(cursor);
    const bool from_main_file = clang_Location_isFromMainFile(location);

    CXCursorKind kind = clang_getCursorKind(cursor);
    if (!from_main_file) {
        return CXChildVisit_Continue;
    } else if (kind == CXCursor_MacroDefinition) {
        const char * guard = clang_getCString(clang_getCursorSpelling(cursor));
        std::cout << "#ifndef " << guard << "\n"
                  << "#define " << guard << "\n"
                  << "\n";
        return CXChildVisit_Break;
    }
    return CXChildVisit_Recurse;
}

CXChildVisitResult
visit (CXCursor cursor, CXCursor parent, CXClientData data_)
{
    client_data & data = *static_cast<client_data*>(data_);

    CXCursor null_cursor = clang_getNullCursor();

    // close open namespaces we have left
    CXCursor enclosing_namespace = parent;
    while (!clang_equalCursors(enclosing_namespace, tu_cursor) &&
           clang_getCursorKind(enclosing_namespace) != CXCursor_Namespace) {
        enclosing_namespace =
            clang_getCursorSemanticParent(enclosing_namespace);
    }
    if (!clang_equalCursors(enclosing_namespace, tu_cursor) &&
        clang_getCursorKind(enclosing_namespace) == CXCursor_Namespace) {
        while (!data.current_namespaces.empty() &&
               !clang_equalCursors(enclosing_namespace,
                                   data.current_namespaces.back())) {
            data.current_namespaces.pop_back();
            close_namespace(data);
        }
    }

    // close open struct if we have left it
    CXCursor enclosing_struct = parent;
    while (!clang_equalCursors(enclosing_struct, tu_cursor) &&
           !struct_kind(clang_getCursorKind(enclosing_struct))) {
        enclosing_struct = clang_getCursorSemanticParent(enclosing_struct);
    }
    if (!clang_Cursor_isNull(data.current_struct) &&
        !clang_equalCursors(enclosing_struct, data.current_struct)) {
        close_struct(data);
        data.current_struct = null_cursor;
        data.member_functions.clear();
    }

    CXSourceLocation location = clang_getCursorLocation(cursor);
    const bool from_main_file = clang_Location_isFromMainFile(location);

    CXCursorKind kind = clang_getCursorKind(cursor);
    if (kind == CXCursor_Namespace) {
        if (from_main_file) {
            print_headers(data);
            open_namespace(data, cursor);
            data.current_namespaces.push_back(cursor);
        }
        return CXChildVisit_Recurse;
    } else if (!from_main_file) {
        return CXChildVisit_Continue;
    } else if (struct_kind(kind)) {
        if (clang_Cursor_isNull(data.current_struct)) {
            print_headers(data);
            data.current_struct = cursor;
            data.current_struct_prefix = struct_prefix(data, cursor);
            return CXChildVisit_Recurse;
        }
    } else if (kind == CXCursor_CXXMethod) {
        data.member_functions.push_back(member_params(data, cursor));
    }

    return CXChildVisit_Continue;
}

CXVisitorResult visit_includes (void * context, CXCursor cursor, CXSourceRange range)
{
    print_tokens(static_cast<CXTranslationUnit>(context), cursor, true);
    return CXVisit_Continue;
}

const std::string long_help_token = "--help";
const std::string short_help_token = "-h";
const std::string manual_token = "--manual";
const std::string form_token = "--form";
const std::string headers_token = "--headers";
const std::string long_cow_token = "--copy-on-write";
const std::string short_cow_token = "-c";

void print_help (const char * process_path)
{
    std::string process_name = process_path;
    std::string::size_type pos = process_name.find_last_of("\\/");
    if (pos != std::string::npos)
        process_name = process_name.substr(pos + 1);
    std::cerr
        << process_name << "\n\n"
        << "Usage: " << process_name << " [options] [clang-args]\n\n"
        << "Options:\n"
        << "-h,--help          Print this help message.\n"
        << "--manual           Print a much longer manual to the terminal.\n"
        << "--form f           Use form 'f' to generate code.\n"
        << "--headers h        Prepend file 'h' to the generated code.\n"
        << "-c,--copy-on-write Generate code suitable for a COW implementation.\n"
        << "\n"
        ;
}

void print_manual ()
{
    std::string manual_text =
R"(emtypen Users' Manual

emtypen generates type erasure C++ code.  It does this to automate much of the
drudgery of creating such types by hand.

Some of this might not make sense if you don't know how type erasure works.
See http://tzlaine.github.io/type_erasure if this is the case.

At the highest level of abstraction, emptypen takes three input files
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

)";

    std::cout << manual_text;
}

int main (int argc, char * argv[])
{
    std::string binary_path = argv[0];
    binary_path.resize(binary_path.find_last_of("/\\") + 1);

    std::string form_filename = binary_path + RELATIVE_DATA_DIR "form.hpp";
    std::string headers_filename = binary_path + RELATIVE_DATA_DIR "headers.hpp";
    bool copy_on_write = false;

    std::vector<char*> argv_copy(argc);
    std::vector<char*>::iterator argv_it = argv_copy.begin();

    for (int i = 0; i < argc; ++i) {
        if (argv[i] == short_help_token || argv[i] == long_help_token) {
            print_help(argv[0]);
            return 0;
        } else if (argv[i] == manual_token) {
            print_manual();
            return 0;
        } else if (argv[i] == form_token) {
            ++i;
            if (argc <= i) {
                std::cerr << argv[0]
                          << ": Form-file must be specified as --form [filename].\n";
                return 1;
            }
            form_filename = argv[i];
            argv_copy.resize(argv_copy.size() - 2);
        } else if (argv[i] == headers_token) {
            ++i;
            if (argc <= i) {
                std::cerr << argv[0]
                          << ": Headers-file must be specified as --headers [filename].\n";
                return 1;
            }
            headers_filename = argv[i];
            argv_copy.resize(argv_copy.size() - 2);
        } else if (argv[i] == short_cow_token || argv[i] == long_cow_token) {
            copy_on_write = true;
            argv_copy.resize(argv_copy.size() - 1);
        } else {
            *argv_it++ = argv[i];
        }
    }

    const std::string form = file_slurp(form_filename);
    const std::string headers = file_slurp(headers_filename);

    if (form.empty()) {
        std::cerr << argv[0]
                  << ": Unable to read form-file '" << form_filename << "'.\n";
        return 1;
    }

    if (headers.empty()) {
        std::cerr << argv[0]
                  << ": Unable to read headers-file '" << headers_filename << "'.\n";
        return 1;
    }

    CXIndex index = clang_createIndex(0, 1);
    CXTranslationUnit tu = clang_parseTranslationUnit(
        index,
        0,
        &argv_copy[0],
        argv_copy.size(),
        0,
        0,
        CXTranslationUnit_DetailedPreprocessingRecord
    );

    const char * filename =
        clang_getCString(clang_getTranslationUnitSpelling(tu));

    if (!filename || !filename[0]) {
        print_help(argv[0]);
        return 1;
    }

    CXFile file = clang_getFile(tu, filename);

    const bool include_guarded = clang_isFileMultipleIncludeGuarded(tu, file);

    client_data data = {
        tu,
        {},
        clang_getNullCursor(),
        "",
        {},
        false,
        filename,
        include_guarded,
        form,
        headers,
        copy_on_write
    };

    tu_cursor = clang_getTranslationUnitCursor(tu);

    clang_visitChildren(tu_cursor, visit_preprocessor_defines, &data);

    CXCursorAndRangeVisitor visitor = {tu, visit_includes};
    clang_findIncludesInFile(tu, file, visitor);

    clang_visitChildren(tu_cursor, visit, &data);

    if (!clang_Cursor_isNull(data.current_struct))
        close_struct(data);

    while (!data.current_namespaces.empty()) {
        data.current_namespaces.pop_back();
        close_namespace(data);
    }

    if (include_guarded)
        std::cout << "#endif\n";

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}
