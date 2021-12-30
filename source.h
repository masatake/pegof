#ifndef INCLUDED_SOURCE_H
#define INCLUDED_SOURCE_H

#include <string>
#include <utility>

using std::string;
using std::string_view;

class Source {
    string path;
    size_t filesize;
    void *mapped;
    string_view text;
    size_t pos;

    void open();
    void close();

public:
    int read();
    std::pair<size_t, size_t> compute_position(size_t start) const;

    Source(const char* filepath);
    ~Source();
};

#endif
