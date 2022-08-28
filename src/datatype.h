#pragma once

#include "fmt/core.h"
#include <string>
#include <vector>
#include <unordered_map>

struct DataType {
    std::string name { "Uninitalized" };
    std::size_t size { 0 };

    std::string to_string() {
        return fmt::format("DataType: .name {{ {} }}, .size {{ {} }}", name, size);
    }

};

inline bool operator == (const DataType& lhs, const DataType& rhs) {
    return lhs.name == rhs.name && lhs.size == rhs.size;
}

inline bool operator != (const DataType& lhs, const DataType& rhs) {
    return !(lhs == rhs);
}

static std::unordered_map<std::string_view, DataType> availableDataTypes {
    { "Uninitialized", { .name = "Uninitialized", .size = 0 } },
    { "Int", { .name = "Int", .size = 4 } },
    { "Bool", { .name = "Bool", .size = 4 } },
};
