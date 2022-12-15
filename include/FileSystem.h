//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#ifndef PANTHER_FILESYSTEM_H
#define PANTHER_FILESYSTEM_H

#include <iostream>
#include <fstream>

namespace Panther {

    class FileSystem {
    public:
        FileSystem(const std::string &);

        void open_output_stream();
        void open_output_stream(const std::ios_base::openmode &);
        std::ofstream &get_output_stream();
        void close_output_stream();
        void open_input_stream();
        void open_input_stream(const std::ios_base::openmode &);
        std::ifstream &get_input_stream();
        void close_input_stream();

    private:
        std::string filesystem_path;
        std::ifstream input_file_stream;
        std::ofstream output_file_stream;
    };

}

#endif //PANTHER_FILESYSTEM_H
