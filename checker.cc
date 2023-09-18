#include "checker.h"
#include "utils.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include <stdio.h>
#include <unistd.h>

namespace fs = std::filesystem;

extern "C" {
    typedef enum bool_tag {
        FALSE = 0,
        TRUE
    } bool_t;

    typedef struct options_tag {
        bool_t ascii;
        bool_t lines;
        bool_t debug;
    } options_t;

    void *create_context(const char *iname, const char *oname, const options_t *opts);
    void destroy_context(void *ctx);

    bool parse(void *ctx);
    bool generate(void *ctx);
}

Checker::Checker() {
    fs::path tmp_dir = fs::temp_directory_path() / ("pegof_" + std::to_string(time(0)));
    output = (tmp_dir / "output").native();
    tmp = tmp_dir.native();
    fs::create_directory(tmp_dir);
}

Checker::~Checker() {
    fs::remove_all(tmp);
}

bool Checker::call_packcc(const std::string& input, std::string& errors) const {
    // Flush stderr first if you've previously printed something
    fflush(stderr);

    // Save stderr so it can be restored later
    int temp_stderr;
    temp_stderr = dup(fileno(stderr));

    // Redirect stderr to a new pipe
    int pipes[2];
    pipe(pipes);
    dup2(pipes[1], fileno(stderr));

    // Call PackCC
    options_t opts = {};
    void *const ctx = create_context(input.c_str(), output.c_str(), &opts);
    bool result = parse(ctx) && generate(ctx);
    destroy_context(ctx);

    // Terminate captured output with a zero
    write(pipes[1], "", 1);

    // Restore stderr
    fflush(stderr);
    dup2(temp_stderr, fileno(stderr));

    // Read the stderr from the pipe
    const int buffer_size = 10240;
    char buffer[buffer_size];
    read(pipes[0], buffer, buffer_size);

    errors = buffer;
    return result;
}

void Checker::stats() const {
        std::string code = read_file(output + ".c");
        std::size_t lines = std::count(code.begin(), code.end(), '\n');
        //~ printf("Resulting code has %ld bytes and %ld lines\n", code.size(), lines);
}

bool Checker::validate(const std::string& input) const {
        std::string errors;
        if (!call_packcc(input, errors)) {
            printf("ERROR: Failed to parse grammar by packcc:\n%s\n", errors.c_str());
            return false;
        }
        return true;
}

bool Checker::validate(const std::string& filename, const std::string& content) const {
        if (filename.empty()) {
            return validate_string("stdin", content);
        } else {
            return validate_file(filename);
        }
}

bool Checker::validate_string(const std::string& filename, const std::string& peg) const {
        fs::path input = fs::path(tmp) / filename;
        write_file(input.native(), peg);
        return validate(input.native());
}

bool Checker::validate_file(const std::string& filename) const {
        return validate(filename);
}
