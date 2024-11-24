//
// Created by QUASARITY on 17.11.2024.
//

#ifndef DATABASE_CONTROLLER_HSE_TYPES_H
#define DATABASE_CONTROLLER_HSE_TYPES_H

#include <string>
#include <unordered_map>
#include <vector>
#include <variant>

namespace database {
    using DBType = std::variant<int, double, bool, std::string>;

    using ResultRowType = std::unordered_map<std::string, DBType>;

    using RowType = std::vector<DBType>;

    using RowType = std::vector<DBType>;
} //database

#endif //DATABASE_CONTROLLER_HSE_TYPES_H
