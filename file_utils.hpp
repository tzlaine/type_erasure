#ifndef FILE_UTILS_INCLUDED__
#define FILE_UTILS_INCLUDED__

#include <fstream>
#include <string>
#include <vector>


inline std::string file_slurp (const std::string & filename)
{
    std::string retval;
    std::ifstream ifs(filename);
    std::getline(ifs, retval, '\0');
    return retval;
}

inline std::vector<std::string> line_break (const std::string & file)
{
    std::vector<std::string> retval;

    std::string::size_type prev_pos = 0;
    while (true) {
        std::string::size_type pos_1 = file.find("\r\n", prev_pos);
        std::string::size_type pos_2 = file.find_first_of("\r\n", prev_pos);
        std::string::size_type pos = pos_2;
        std::string::size_type match_size = 1;
        if (pos_1 != std::string::npos) {
            pos = pos_1;
            match_size = 2;
        } else if (pos_2 == std::string::npos) {
	        break;
        }
        retval.push_back(file.substr(prev_pos, pos - prev_pos));
        prev_pos = pos + match_size;
    }

    retval.push_back(file.substr(prev_pos));

    return retval;
}

#endif
