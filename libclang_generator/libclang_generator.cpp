#include <clang-c/Index.h>
#include <cstdio>

#include <iostream>
#include <vector>


namespace {
    const unsigned int indent_spaces = 4;
}

CXCursor tu_cursor;

struct client_data
{
    CXTranslationUnit tu;
    std::vector<CXCursor> current_namespaces;
    CXCursor current_struct;
};

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

void print_tokens (CXTranslationUnit tu, CXCursor cursor)
{
    std::pair<CXToken*, unsigned int> tokens = get_tokens(tu, cursor);

    for (unsigned int i = 0; i < tokens.second; ++i) {
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
    }
    return false;
}

std::string indent (const client_data& data)
{
    std::size_t size = data.current_namespaces.size();
    return std::string(size * indent_spaces, ' ');
}

void open_struct (const client_data& data, CXCursor struct_cursor)
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

    std::cout << "\n"
              << indent(data) << "{\n"
              << indent(data) << "public:\n";

    const char* public_interface[] = {
        0,
        "any_printable () = default;",
        0,
        "template <typename T>",
        "any_printable (T value) :",
        "    handle_ (",
        "        std::make_shared<",
        "            handle<typename std::remove_reference<T>::type>",
        "        >(std::forward<T>(value))",
        "    )",
        "{}",
        0,
        "any_printable (any_printable && rhs) noexcept = default;",
        0,
        "template <typename T>",
        "any_printable & operator= (T value)",
        "{",
        "    if (handle_.unique())",
        "        *handle_ = std::forward<T>(value);",
        "    else if (!handle_)",
        "        handle_ = std::make_shared<T>(std::forward<T>(value));",
        "    return *this;",
        "}",
        0,
        "any_printable & operator= (any_printable && rhs) noexcept = default;"
    };

    std::string padding(indent_spaces, ' ');
    for (unsigned int i = 0;
         i < sizeof(public_interface) / sizeof(const char*);
         ++i) {
        if (public_interface[i])
            std::cout << indent(data) << padding << public_interface[i] << "\n";
        else
            std::cout << "\n";
    }
}

void close_struct (const client_data& data)
{
    std::cout << "\n\n"
              << indent(data) << "private:\n";

    std::cout << "\n"
              << indent(data) << "    // TODO\n";

    std::cout << "\n"
              << indent(data) << "};\n";
}

void print_member_function(const client_data& data, CXCursor cursor)
{
    std::pair<CXToken*, unsigned int> tokens = get_tokens(data.tu, cursor);

    std::cout << "\n"
              << std::string(indent_spaces, ' ') << indent(data);

    const std::string open_brace = "{";
    const std::string semicolon = ";";

    for (unsigned int i = 0; i < tokens.second; ++i) {
        CXString spelling = clang_getTokenSpelling(data.tu, tokens.first[i]);

        const char* c_str = clang_getCString(spelling);

        if (c_str == open_brace || c_str == semicolon)
            break;

        if (i)
            std::cout << " ";

        std::cout << c_str;
    }

    free_tokens(data.tu, tokens);

    const char* return_str =
        clang_getCursorResultType(cursor).kind == CXType_Void ? "" : "return ";

    std::cout << "\n" << std::string(indent_spaces, ' ') << indent(data)
              << "{ assert(handle_); " << return_str << "handle_->"
              << clang_getCString(clang_getCursorSpelling(cursor))
              << "( ";
    const int args = clang_Cursor_getNumArguments(cursor);
    for (int i = 0; i < args; ++i) {
        if (i)
            std::cout << ", ";
        CXCursor arg_cursor = clang_Cursor_getArgument(cursor, i);
        std::cout << clang_getCString(clang_getCursorSpelling(arg_cursor));
    }
    std::cout << " ); }\n";
}

void open_namespace (const client_data& data, CXCursor namespace_)
{
    std::cout
        << "\n"
        << indent(data)
        << "namespace "
        << clang_getCString(clang_getCursorSpelling(namespace_))
        << " {";
}

void close_namespace (const client_data& data)
{ std::cout << "\n" << indent(data) << "}\n"; }

void dump_cursor (const char* name, CXCursor cursor)
{
    CXCursorKind kind = clang_getCursorKind(cursor);
    CXString kind_spelling = clang_getCursorKindSpelling(kind);
    CXString cursor_spelling = clang_getCursorSpelling(cursor);
    std::cout << name << " "
              << clang_getCString(kind_spelling) << " "
              << clang_getCString(cursor_spelling) << " "
              << "\n";
}

CXChildVisitResult
visitor (CXCursor cursor, CXCursor parent, CXClientData data_)
{
    client_data& data = *static_cast<client_data*>(data_);

#if 0
    std::cout << "\n";
    dump_cursor("cursor", cursor);
    dump_cursor("parent", parent);
#endif

    CXCursor null_cursor = clang_getNullCursor();

    // close open namespaces we have left
    CXCursor enclosing_namespace = parent;
    while (!clang_equalCursors(enclosing_namespace, tu_cursor) &&
           clang_getCursorKind(enclosing_namespace) != CXCursor_Namespace) {
        enclosing_namespace =
            clang_getCursorSemanticParent(enclosing_namespace);
    }
#if 0
    dump_cursor("enclosing_namespace", enclosing_namespace);
#endif
    if (!clang_equalCursors(enclosing_namespace, tu_cursor) &&
        clang_getCursorKind(enclosing_namespace) == CXCursor_Namespace) {
        while (!clang_equalCursors(enclosing_namespace,
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
#if 0
    dump_cursor("enclosing_struct", enclosing_struct);
#endif
    if (!clang_Cursor_isNull(data.current_struct) &&
        !clang_equalCursors(enclosing_struct, data.current_struct)) {
        data.current_struct = null_cursor;
        close_struct(data);
    }

    CXCursorKind kind = clang_getCursorKind(cursor);
    if (kind == CXCursor_Namespace) {
        open_namespace(data, cursor);
        data.current_namespaces.push_back(cursor);
        return CXChildVisit_Recurse;
    } else if (struct_kind(kind)) {
        if (clang_Cursor_isNull(data.current_struct)) {
            data.current_struct = cursor;
            open_struct(data, cursor);
            return CXChildVisit_Recurse;
        }
    } else if (kind == CXCursor_CXXMethod) {
        print_member_function(data, cursor);
    }

    return CXChildVisit_Continue;
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
        CXTranslationUnit_None
    );

    client_data data = {tu, {}, clang_getNullCursor()};

    tu_cursor = clang_getTranslationUnitCursor(tu);
    clang_visitChildren(tu_cursor, visitor, &data);

    if (!clang_Cursor_isNull(data.current_struct))
        close_struct(data);

    while (!data.current_namespaces.empty()) {
        data.current_namespaces.pop_back();
        close_namespace(data);
    }

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}
