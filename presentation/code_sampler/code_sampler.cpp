#include <file_utils.hpp>

#include <cassert>
#include <map>
#include <vector>


void find_samples (const std::vector<std::string> & lines,
                   std::map<std::string, std::string> & samples)
{
    const std::string sample_str = "sample";
    const std::string end_str = "end-sample";

    std::string current_sample_name;
    for (const std::string & line : lines) {
        const bool comment_line = line.find("//") == 0;
        std::string::size_type sample_pos = line.find(sample_str);
        std::string::size_type end_pos = line.find(end_str);
        if (comment_line && end_pos != std::string::npos) {
            samples[current_sample_name] += "```";
            current_sample_name = "";
        } else if (comment_line && sample_pos != std::string::npos) {
            sample_pos += sample_str.size() + 1;
            std::string::size_type close_paren = line.find(")", sample_pos);
            current_sample_name =
                line.substr(sample_pos, close_paren - sample_pos);
            std::string::size_type size_minus_3 =
                samples[current_sample_name].size() - 3;
            if (samples[current_sample_name].rfind("```") == size_minus_3) {
                samples[current_sample_name].resize(
                    samples[current_sample_name].size() - 3
                );
            } else {
                samples[current_sample_name] += "```cpp\n";
            }
        } else if (current_sample_name != "") {
            samples[current_sample_name] += line + '\n';
        }
    }
}


int main (int argc, char * argv[])
{
    assert(3 <= argc);

    const char * in_filename = argv[1];

    std::vector<std::string> file_lines = line_break(file_slurp(in_filename));

    std::map<std::string, std::string> samples;
    std::for_each(argv + 3, argv + argc,
                  [&](const char * filename) {
                      find_samples(line_break(file_slurp(filename)), samples);
                  });

    for (std::string & line : file_lines) {
        std::string::size_type sample_ref_pos = line.find("%%");
        if (sample_ref_pos != std::string::npos) {
            sample_ref_pos += 2;
            std::string::size_type sample_ref_end_pos =
                line.find("%%", sample_ref_pos);
            const std::string sample_name = line.substr(
                sample_ref_pos,
                sample_ref_end_pos - sample_ref_pos
            );
            line = samples[sample_name];
        }
    }

    const char * out_filename = argv[2];
    std::ofstream ofs(out_filename);
    for (const std::string & line : file_lines) {
        ofs << line << "\n";
    }

    return 0;
}
