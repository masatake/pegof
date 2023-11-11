#pragma once
#include "ast/node.h"

class Alternation;

class Group : public Node {
public:
    std::shared_ptr<Alternation> expression;

    Group(const Alternation& expression, Node* parent);
    Group(Parser& p, Node* parent);

    virtual void parse(Parser& p);
    virtual std::string to_string(std::string indent = "") const override;
    virtual std::string dump(std::string = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;

    bool has_single_term() const;
};

bool operator==(const Group& a, const Group& b);
bool operator!=(const Group& a, const Group& b);
