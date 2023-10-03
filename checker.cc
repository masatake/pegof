#include "checker.h"
#include "packcc_wrapper.h"
#include "utils.h"
#include "log.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include <stdio.h>
#include <unistd.h>

namespace fs = std::filesystem;

std::string Stats::compare(const Stats& s) const {
    #define PAD(X) left_pad(std::to_string(X), 8)
    #define PADP(X) left_pad(std::to_string(X * 100 / s.X) + "%", 8)

    std::string result;
    result += "         |   lines  |   bytes  |   rules  |   terms  \n";
    result += "---------+----------+----------+----------+----------\n";
    result += "input    | " + PAD(s.lines) + " | " + PAD(s.bytes) + " | " + PAD(s.rules) + " | " + PAD(s.terms) + "\n";
    result += "output   | " + PAD(lines) + " | " + PAD(bytes) + " | " + PAD(rules) + " | " + PAD(terms) + "\n";
    result += "output % | " + PADP(lines) + " | " + PADP(bytes) + " | " + PADP(rules) + " | " + PADP(terms);
    return result;

    #undef PAD
    #undef PADP
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
    const int buffer_size = 102400;
    char buffer[buffer_size];
    // TODO: fix reading when there is more data then the buffer_size
    read(pipes[0], buffer, buffer_size);

    errors = buffer;
    return result;
}

Stats Checker::stats(Grammar& g) const {
    std::string code = read_file(output + ".c");
    std::size_t lines = std::count(code.begin(), code.end(), '\n');
    int rules = g.find_all<Rule>().size();
    int terms = g.find_all<Term>().size();
    log(2, "Code has %ld bytes and %ld lines", code.size(), lines);
    log(2, "Grammar has %d rules and %d terms", rules, terms);
    return Stats(code.size(), lines, rules, terms);
}

bool Checker::validate(const std::string& input) const {
    std::string errors;
    if (!call_packcc(input, errors)) {
        error("Failed to parse grammar by packcc:\n%s", errors.c_str());
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
