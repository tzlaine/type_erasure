#include <clang-c/Index.h>

#include <array>
#include <fstream>
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
    // keyword, and function name
    std::vector<std::array<std::string, 4>> member_functions;
    bool printed_headers;
    const char* filename;
    bool include_guarded;
    std::string form;
    std::string headers;
};

std::string file_slurp (const std::string & filename)
{
    std::string retval;

    std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
    ifs.seekg(0, std::ifstream::end);
    if (0 < ifs.tellg())
        retval.resize(ifs.tellg());
    ifs.seekg(0);

    const std::streamsize read_size = 64 * 1024; // 64k per read
    char* retval_pos = &retval[0];
    std::streamsize bytes_read = 0;
    do {
        ifs.read(retval_pos, read_size);
        bytes_read = ifs.gcount();
        retval_pos += bytes_read;
    } while (bytes_read == read_size);

    return retval;
}

std::pair<CXToken*, unsigned int>
get_tokens (CXTranslationUnit tu, CXCursor cursor)
{
    CXSourceRange range = clang_getCursorExtent(cursor);
    CXSourceLocation start = clang_getRangeStart(range);
    CXSourceLocation end = clang_getRangeEnd(range);
    unsigned int start_offset, end_offset;

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
    const std::string close_angle = ">";

    const unsigned int num_tokens =
        tokens.second && tokens_from_include_directive ?
        tokens.second - 1 :
        tokens.second;
    const char* prev_token = 0;
    for (unsigned int i = 0; i < num_tokens; ++i) {
        CXString spelling = clang_getTokenSpelling(tu, tokens.first[i]);
        const char* token = clang_getCString(spelling);
        if (i && prev_token != open_angle && token != close_angle)
            std::cout << " ";
        std::cout << token;
        prev_token = token;
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
                  const char** lines,
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

        const char* c_str = clang_getCString(spelling);

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

std::array<std::string, 4>
member_params (const client_data & data, CXCursor cursor)
{
    std::pair<CXToken*, unsigned int> tokens = get_tokens(data.tu, cursor);

    const std::string open_brace = "{";
    const std::string semicolon = ";";

    std::string str;

    for (unsigned int i = 0; i < tokens.second; ++i) {
        CXString spelling =
            clang_getTokenSpelling(data.tu, tokens.first[i]);

        const char* c_str = clang_getCString(spelling);

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

    const char* return_str =
        clang_getCursorResultType(cursor).kind == CXType_Void ?
        "" :
        "return ";

    const char* function_name =
        clang_getCString(clang_getCursorSpelling(cursor));

    free_tokens(data.tu, tokens);

    return {str, args, return_str, function_name};
}

std::vector<std::string> line_break (const std::string & form)
{
    std::vector<std::string> retval;

    std::string::size_type prev_pos = 0;
    while (true) {
        std::string::size_type pos = form.find('\n', prev_pos);
        if (pos == std::string::npos)
            break;
        retval.push_back(form.substr(prev_pos, pos - prev_pos));
        prev_pos = pos + 1;
    }

    retval.push_back(form.substr(prev_pos));

    return retval;
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
        nonvirtual_members +=
            spacing + function[0] + "\n" +
            spacing + "{ assert(handle_); " + function[2] +
            "handle_->" + function[3] +
            "(" + function[1] + " ); }\n";

        spacing = std::string(expansion_lines[1].second, ' ') + indent(data);
        pure_virtual_members +=
            spacing + "virtual " + function[0] + " = 0;\n";

        spacing = std::string(expansion_lines[2].second, ' ') + indent(data);
        virtual_members +=
            spacing + function[0] + "\n" +
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
        const char* guard = clang_getCString(clang_getCursorSpelling(cursor));
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

int main (int argc, char* argv[])
{
    std::string form_filename = "form.hpp";
    std::string headers_filename = "headers.hpp";
    std::vector<char*> argv_copy(argc);
    std::vector<char*>::iterator argv_it = argv_copy.begin();
    const std::string form_token = "--form";
    const std::string headers_token = "--headers";
    for (int i = 0; i < argc; ++i) {
        if (argv[i] == form_token) {
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

    const char* filename =
        clang_getCString(clang_getTranslationUnitSpelling(tu));

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
        headers
    };

    tu_cursor = clang_getTranslationUnitCursor(tu);

    clang_visitChildren(tu_cursor, visit_preprocessor_defines, &data);

    CXCursorAndRangeVisitor visitor = {tu, visit_includes};
    clang_findIncludesInFile(tu, file, visitor);
    std::cout << "\n";

    clang_visitChildren(tu_cursor, visit, &data);

    if (!clang_Cursor_isNull(data.current_struct))
        close_struct(data);

    while (!data.current_namespaces.empty()) {
        data.current_namespaces.pop_back();
        close_namespace(data);
    }

    if (include_guarded)
        std::cout << "\n#endif\n";

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}
