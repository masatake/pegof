#pragma once
#include <variant>

#include "node.h"
#include "string.h"
#include "reference.h"
#include "character_class.h"
#include "expand.h"
#include "action.h"
#include "group.h"
#include "capture.h"

using Primary = std::variant<String, Reference, CharacterClass2, Expand, Action, Group, Capture>;

class Term : public Node {
    char prefix;
    char quantifier;
    Primary primary;
public:
    Term(char prefix, char quantifier, const Primary& primary);
    Term(Parser2& p);

    template<class T>
    bool parse(Parser2& p);

    virtual void parse(Parser2& p);

    std::string to_string(const Primary& x) const;
    virtual std::string to_string() const override;
    std::string dump(const Primary& x, std::string indent) const;
    virtual std::string dump(std::string indent = "") const override;

    virtual Node* operator[](int index);
    virtual long size() const;
};