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
    // function signature, forwarding call arguments, optional return
    // keyword, and function name
    std::vector<std::array<std::string, 4>> member_functions;
    bool printed_headers;
    const char* filename;
    bool include_guarded;
};

std::string file_slurp (const std::string & filename)
{
    std::string retval;

    std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
    ifs.seekg(0, std::ifstream::end);
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

void print_tokens (CXTranslationUnit tu, CXCursor cursor, bool elide_final_token)
{
    std::pair<CXToken*, unsigned int> tokens = get_tokens(tu, cursor);

    const unsigned int num_tokens =
        tokens.second && elide_final_token ? tokens.second - 1 : tokens.second;
    for (unsigned int i = 0; i < num_tokens; ++i) {
        if (i)
            std::cout << " ";
        CXString spelling = clang_getTokenSpelling(tu, tokens.first[i]);
        std::cout << clang_getCString(spelling);
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

    std::cout << "#include <algorithm>\n"
              << "#include <functional>\n"
              << "#include <memory>\n"
              << "#include <type_traits>\n"
              << "#include <utility>\n"
              << "\n";

    data.printed_headers = true;
}

void open_struct (const client_data & data, CXCursor struct_cursor)
{
    std::pair<CXToken*, unsigned int> tokens =
        get_tokens(data.tu, struct_cursor);

    std::cout << "\n" << indent(data);

    const std::string open_brace = "{";
    const std::string struct_ = "struct";
    const std::string class_ = "class";

    for (unsigned int i = 0; i < tokens.second; ++i) {
        CXString spelling = clang_getTokenSpelling(data.tu, tokens.first[i]);

        const char* c_str = clang_getCString(spelling);

        if (c_str == open_brace)
            break;

        if (c_str == struct_ || c_str == class_) {
            std::cout << "\n" << indent(data);
        } else if (i) {
            std::cout << " ";
        }

        std::cout << c_str;
    }

    free_tokens(data.tu, tokens);

    const std::string struct_name =
        clang_getCString(clang_getCursorSpelling(data.current_struct));

    const std::string ctor_0 =
        struct_name + " () = default;";
    const std::string ctor_1 =
        struct_name + " (T_T__ value) :";
    const std::string ctor_2 =
        struct_name + " (any_printable && rhs) noexcept = default;";
    const std::string ctor_3 =
        struct_name + " & operator= (T_T__ value)";
    const std::string ctor_4 =
        struct_name + " & operator= (any_printable && rhs) noexcept = default;";

    std::cout << "\n"
              << indent(data) << "{\n"
              << indent(data) << "public:\n";

    const char* public_interface[] = {
        0,
        ctor_0.c_str(),
        0,
        "template <typename T_T__>",
        ctor_1.c_str(),
        "    handle_ (",
        "        std::make_shared<",
        "            handle<typename std::remove_reference<T_T__>::type>",
        "        >(std::forward<T_T__>(value))",
        "    )",
        "{}",
        0,
        ctor_2.c_str(),
        0,
        "template <typename T_T__>",
        ctor_3.c_str(),
        "{",
        "    if (handle_.unique())",
        "        *handle_ = std::forward<T_T__>(value);",
        "    else if (!handle_)",
        "        handle_ = std::make_shared<T_T__>(std::forward<T_T__>(value));",
        "    return *this;",
        "}",
        0,
        ctor_4.c_str()
    };

    print_lines(data,
                public_interface,
                sizeof(public_interface) / sizeof(const char*));
}

void close_struct (const client_data & data)
{
    std::cout << "\n"
              << indent(data) << "private:\n";

    const char* handle_base_preamble[] = {
        0,
        "struct handle_base",
        "{",
        "    virtual ~handle_base () {}",
        "    virtual std::shared_ptr<handle_base> close () const = 0;",
        0
    };

    print_lines(data,
                handle_base_preamble,
                sizeof(handle_base_preamble) / sizeof(const char*));

    for (auto & member : data.member_functions) {
        std::cout << indent(data) << indentation << indentation
                  << "virtual " << member[0] << " = 0;\n";
    }

    const char* handle_preamble[] = {
        0,
        "};",
        0,
        "template <typename T_T__>",
        "struct handle :",
        "    handle_base",
        "{",
        "    template <typename T_T__>",
        "    handle (T_T__ value,",
        "            typename std::enable_if<",
        "                std::is_reference<U_U__>::value",
        "            >::type* = 0) :",
        "        value_ (value)",
        "    {}",
        0,
        "    template <typename U_U__ = T_T__>",
        "    handle (T_T__ value,",
        "            typename std::enable_if<",
        "                !std::is_reference<U_U__>::value,",
        "                int",
        "            >::type* = 0) noexcept :",
        "        value_ (std::move(value))",
        "    {}",
        0,
        "    virtual std::shared_ptr<handle_base> clone () const",
        "    { return std::make_shared<handle>(value_); }"
    };

    print_lines(data,
                handle_preamble,
                sizeof(handle_preamble) / sizeof(const char*));

    for (auto & member : data.member_functions) {
        std::cout << "\n"
                  << indent(data) << indentation << indentation
                  << "virtual " << member[0] << "\n"
                  << indent(data) << indentation << indentation
                  << "{ " << member[2] << "value_." << member[3]
                  << "( " << member[1] << " ); }\n";
    }

    const char* handle_postamble[] = {
        0,
        "    T_T__ value_;",
        "};",
        0,
        "template <typename T_T__>",
        "struct handle<std::reference_wrapper<T_T__>> :",
        "    handle<T_T__ &>",
        "{",
        "    handle (std::reference_wrapper<T_T__> ref) :",
        "        handle<T_T__ &> (ref.get())",
        "    {}",
        "};",
        0,
        "const handle_base & read () const",
        "{ return *handle_; }",
        0,
        "handle_base & write ()",
        "{",
        "    if (!handle_.unique())",
        "        handle_ = handle_->clone();",
        "    return *handle_;",
        "}",
        0,
        "std::shared_ptr<handle_base> handle_;"
    };

    print_lines(data,
                handle_postamble,
                sizeof(handle_postamble) / sizeof(const char*));

    std::cout << "\n"
              << indent(data) << "};\n";
}

void print_member_function (const client_data & data, CXCursor cursor)
{
    std::cout << "\n"
              << std::string(indent_spaces, ' ') << indent(data)
              << data.member_functions.back()[0];

    std::cout << "\n" << std::string(indent_spaces, ' ') << indent(data)
              << "{ assert(handle_); " << data.member_functions.back()[2]
              << "handle_->"
              << clang_getCString(clang_getCursorSpelling(cursor))
              << "( " << data.member_functions.back()[1] << " ); }\n";
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
        data.current_struct = null_cursor;
        close_struct(data);
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
            open_struct(data, cursor);
            return CXChildVisit_Recurse;
        }
    } else if (kind == CXCursor_CXXMethod) {
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

        data.member_functions.push_back({str, args, return_str, function_name});

        print_member_function(data, cursor);
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
    CXIndex index = clang_createIndex(0, 1);
    CXTranslationUnit tu = clang_parseTranslationUnit(
        index,
        0,
        argv,
        argc,
        0,
        0,
        CXTranslationUnit_DetailedPreprocessingRecord
    );

    const char* filename =
        clang_getCString(clang_getTranslationUnitSpelling(tu));

    CXFile file = clang_getFile(tu, filename);

    const bool include_guarded = clang_isFileMultipleIncludeGuarded(tu, file);

    client_data data =
        {tu, {}, clang_getNullCursor(), {}, false, filename, include_guarded};

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
