#pragma once
#include <string>
#include <optional>
#include <regex>
#include <variant>

#include "parser.h"

class Node {
public:
    bool valid;
    Node* parent;
    Node(const char* type, Node* parent);

    const char* type;
    std::string comment;

    virtual void parse(Parser2& p) = 0;
    virtual std::string to_string() const = 0;
    virtual std::string dump(std::string indent = "") const = 0;

    operator bool();

    template<class U>
    bool is() const;

    template<class U>
    U* as() const;

    template<class U>
    U* get_parent() const;

    virtual Node* operator[](int index);

    virtual long size() const;

    void update_parents();

    template <class U>
    std::vector<U*> find_all(const std::function<bool(const U&)>& predicate);

    template <class U>
    void find_all(std::vector<U*>& result, const std::function<bool(const U&)>& predicate);

    void map(const std::function<bool(Node&)>& transform);

    void parse_comments(Parser2& p);
    std::string comments(std::string indent = "") const;
};

template<class U>
bool Node::is() const {
    return typeid(*this) == typeid(U);
}

template<class U>
U* Node::as() const {
    if (!is<U>()) return nullptr;
    return (U*)this;
}

template<class U>
U* Node::get_parent() const {
    if (!parent) return nullptr;
    return parent->as<U>();
}

template <class U>
std::vector<U*> Node::find_all(const std::function<bool(const U&)>& predicate) {
    std::vector<U*> result;
    find_all(result, predicate);
    return result;
}

template <class U>
void Node::find_all(std::vector<U*>& result, const std::function<bool(const U&)>& predicate) {
    if (is<U>() && predicate(*(U*)this)) {
        result.push_back((U*)(this));
    }
    for (int i = 1; i < size(); i++) {
        Node* n = (*this)[i];
        n->find_all(result, predicate);
    }
}
