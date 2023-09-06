#pragma once
#include "node.h"

class Directive : public Node {
    std::string name;
    std::string value;
    bool code;
public:
    Directive(const std::string& name, const std::string& value, bool code);
    Directive(Parser2& p);

    virtual void parse(Parser2& p);
    virtual std::string to_string() const override;
    virtual std::string dump(std::string indent = "") const override;
};
