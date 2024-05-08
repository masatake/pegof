#include "ast/alternation.h"
#include "rule.h"
#include "config.h"
#include "log.h"

Alternation::Alternation(const std::vector<Sequence>& sequences, Node* parent) : Node("Alternation", parent), sequences(sequences) {}
Alternation::Alternation(Parser& p, Node* parent) : Node("Alternation", parent) {
    parse(p);
}
Alternation::Alternation(Node* parent) : Node("Alternation", parent) {}

void Alternation::parse(Parser& p) {
    debug("Parsing Alternation");
    DebugIndent _;
    Parser::State sp = p.save_point();
    parse_comments(p);
    p.skip_space();
    Sequence s = Sequence(p, this);
    if (!s) {
        sp.rollback();
        return;
    }
    while (s) {
        sequences.push_back(s);
        if (!p.match("/")) {
            break;
        }
        s = Sequence(p, this);
    }
    parse_post_comment(p);
    sp.commit();
    valid = true;
}

std::string Alternation::to_string(std::string indent) const {
    bool multiline = sequences.size() > Config::get<int>("wrap-limit");
    std::string delimiter = multiline ? ("\n" + indent + "/ ") : std::string(" / ");
    std::string result;
    if (multiline && comments.empty()) {
        result += indent;
    }
    if (!comments.empty()) {
        result += format_comments(indent) + "\n" + indent;
    }
    result += sequences[0].to_string(indent + "    ");
    for (int i = 1; i < sequences.size(); i++) {
        result += delimiter + sequences[i].to_string(indent + "    ");
    }
    return result;
}

std::string Alternation::dump(std::string indent) const {
    std::string result = indent + "ALTERNATION" + dump_comments() + "\n";
    for (int i = 0; i < sequences.size(); i++) {
        if (i > 0) result += "\n";
        result += sequences[i].dump(indent + "  ");
    }
    return result;
}

Sequence& Alternation::get(int index) {
    return sequences[index];
}

Node* Alternation::operator[](int index) {
    if (index < sequences.size()) {
        return &(sequences[index]);
    } else {
        error("index out of bounds!");
    }
}

long Alternation::size() const {
    return sequences.size();
}

bool Alternation::has_single_term() const {
    return sequences.size() == 1 && sequences[0].has_single_term();
}

const Sequence& Alternation::get_first_sequence() const {
    return sequences[0];
}

void Alternation::insert(int index, const Alternation& a) {
    sequences.insert(sequences.begin() + index, a.sequences.begin(), a.sequences.end());
}

void Alternation::erase(int index) {
    sequences.erase(sequences.begin() + index);
}

void Alternation::erase(Sequence* s) {
    for (int i = 0; i < sequences.size(); i++) {
        if (*s == sequences[i]) {
            sequences.erase(sequences.begin() + i);
            return;
        }
    }
}

bool operator==(const Alternation& a, const Alternation& b) {
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); i++) {
        if (a.sequences[i] != b.sequences[i]) return false;
    }
    return true;
}

bool operator!=(const Alternation& a, const Alternation& b) {
    return !(a == b);
}