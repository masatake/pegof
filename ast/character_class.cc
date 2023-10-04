#include "character_class.h"
#include "packcc_wrapper.h"
#include "utils.h"
#include "log.h"

#include <cstring>
#include <sstream>
#include <iomanip>
#include <numeric>

CharacterClass::CharacterClass(const std::string& content, Node* parent) : Node("CharacterClass", parent), content(content), dash(false), negation(false) {
    Parser p(content);
    parse(p);
}
CharacterClass::CharacterClass(Parser& p, Node* parent) : Node("CharacterClass", parent), dash(false), negation(false) {
    parse(p);
}

static std::string unescape(const std::string& s) {
    char* char_array = new char[s.size() + 1];
    strcpy(char_array, s.c_str());
    unescape_string(char_array, FALSE);
    std::string result = char_array;
    delete[] char_array;
    return result;
}

static std::string to_char(int c) {
    if (c > 127) {
        std::stringstream ss;
        ss << "\\u" << std::setfill('0') << std::setw(4) << std::hex << c;
        return ss.str();
    }
    switch (c) {
    case '\r': return R"(\r)";
    case '\n': return R"(\n)";
    case '\t': return R"(\t)";
    case '\v': return R"(\v)";
    case '\f': return R"(\f)";
    case '[': return R"(\[)";
    case ']': return R"(\])";
    case '^': return R"(\^)";
    case '-': return R"(\-)";
    case '\\': return R"(\\)";
    default: return std::string(1, (char)c);
    }
}

static int get_char(const std::string& s, int& pos) {
    int result;
    pos += utf8_to_utf32(s.c_str() + pos, &result);
    if (result != '\\' || pos == s.size()) {
        return result;
    }
    switch (s[pos]) {
    case '[': pos++; return '[';
    case ']': pos++; return ']';
    case '\\': pos++; return '\\';
    default: return result;
    }
}

void CharacterClass::parse_content(Parser& p) {
    negation = p.match('^');
    dash = p.match('-');
    while (!p.is_eof()) {
        if (p.match(']')) break;
        content += p.current();
        if (p.current() == '\\') {
            p.match_any();
            content += p.current();
        }
        p.match_any();
    }
    content = unescape(content);
    int pos = 0;
    while (pos < content.size()) {
        Token result;
        result.first = get_char(content, pos);
        if (content[pos] == '-') {
            pos++;
            result.second = get_char(content, pos);
        } else {
            result.second = result.first;
        }
        tokens.push_back(result);
    }
    update_content();
}

void CharacterClass::parse(Parser& p) {
    debug("Parsing CharacterClass");
    p.skip_space();
    if (p.match('.')) {
        content = ".";
    } else if (p.match('[')) {
        parse_content(p);
    } else {
        return;
    }
    valid = true;
}

void CharacterClass::update_content() {
    if (content == ".") return;

    content = "";

    for (size_t i = 0; i<tokens.size(); i++) {
        size_t size = tokens[i].second - tokens[i].first;
        switch (size) {
        case 0:
            content += to_char(tokens[i].first);
            break;
        case 1:
            content += to_char(tokens[i].first);
            content += to_char(tokens[i].second);
            break;
        default:
            content += to_char(tokens[i].first);
            content += '-';
            content += to_char(tokens[i].second);
            break;
        }
    }
}

std::string CharacterClass::to_string() const {
    if (content == ".") {
        return ".";
    } else {
        std::string result = "[";
        if(negation) result += '^';
        if(dash) result += '-';
        result += content + ']';
        return result;
    }
}

std::string CharacterClass::dump(std::string indent) const {
    return indent + "CHAR_CLASS " + to_c_string(content);
}

void CharacterClass::normalize() {
    if (tokens.empty()) {
        return;
    }
    sort(tokens.begin(), tokens.end());
    Tokens init(1, tokens[0]);
    tokens = accumulate(tokens.begin() + 1, tokens.end(), init, [](Tokens& acc, const Token& item){
        Token& prev = acc.back();
        if (prev.second + 1 < item.first) {
            acc.push_back(item);
        } else if (item.second > prev.second) {
            prev.second = item.second;
        }
        return acc;
    });
    update_content();
}

bool operator==(const CharacterClass& a, const CharacterClass& b) {
    return a.negation == b.negation && a.dash == b.dash && a.content == b.content;
}

bool operator!=(const CharacterClass& a, const CharacterClass& b) {
    return !(a == b);
}
