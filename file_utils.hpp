#ifndef FILE_UTILS_INCLUDED__
#define FILE_UTILS_INCLUDED__

#include <fstream>
#include <string>
#include <vector>


inline std::string file_slurp (const std::string & filename)
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

inline std::vector<std::string> line_break (const std::string & file)
{
    std::vector<std::string> retval;

    std::string::size_type prev_pos = 0;
    while (true) {
        std::string::size_type pos = file.find('\n', prev_pos);
        if (pos == std::string::npos)
            break;
        retval.push_back(file.substr(prev_pos, pos - prev_pos));
        prev_pos = pos + 1;
    }

    retval.push_back(file.substr(prev_pos));

    return retval;
}

#endif
