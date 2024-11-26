//
// Created by QUASARITY on 06.11.2024.
//

#include "Table.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "../../Calculator/Calculator.h"

namespace database {

std::string dBTypeToString(DBType value) {
    std::string str_value;
    if (std::holds_alternative<int>(value)) {
        str_value = std::to_string(std::get<int>(value));
    } else if (std::holds_alternative<double>(value)) {
        str_value = std::to_string(std::get<double>(value));
    } else if (std::holds_alternative<bool>(value)) {
        str_value = std::to_string(std::get<bool>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        str_value = std::get<std::string>(value);
    }
    return str_value;
}

std::string Table::convert_to_byte_buffer() {
    std::string buffer;
    for (auto& row : rows_) {
        int current_column_index = 0;
        for (auto& cell : row) {
            union {
                int intValue;
                double doubleValue;
                bool boolValue;
                char charArray4[4];
                char charArray8[8];
            } converter{};

            if (std::holds_alternative<int>(cell)) {
                converter.intValue = std::get<int>(cell);
                buffer.append(converter.charArray4, 4);
            }
            if (std::holds_alternative<double>(cell)) {
                converter.doubleValue = std::get<double>(cell);
                buffer.append(converter.charArray8, 8);
            }
            if (std::holds_alternative<bool>(cell)) {
                converter.boolValue = std::get<bool>(cell);
                buffer.append(converter.charArray4, 4);
            }
            if (std::holds_alternative<std::string>(cell)) {
                std::string str = std::get<std::string>(cell);
                str.resize(row_sizes_[current_column_index]);
                buffer.append(str);
            }
            // TODO add buffer type;

            ++current_column_index;
        }
    }
    return buffer;
}

void Table::load_from_byte_buffer(const std::string& buffer) {
    std::vector<RowType> rows;
    size_t offset = 0;
    while (offset < buffer.size()) {
        RowType row;
        for (size_t size : row_sizes_) {
            if (size == 4) {
                union {
                    int intValue;
                    char charArray4[4];
                } converter{};
                std::memcpy(converter.charArray4, buffer.data() + offset, 4);
                row.emplace_back(converter.intValue);
                offset += 4;
            } else if (size == 8) {
                union {
                    double doubleValue;
                    char charArray8[8];
                } converter{};
                std::memcpy(converter.charArray8, buffer.data() + offset, 8);
                row.emplace_back(converter.doubleValue);
                offset += 8;
            } else {
                std::string str(buffer.data() + offset, size);
                row.emplace_back(str);
                offset += size;
            }
        }
        rows.push_back(row);
    }
    rows_ = rows;
}

std::vector<RowType> Table::filter(
    const std::function<bool(const RowType&)>& predicate) {
    std::vector<RowType> result;
    for (auto& row : rows_) {
        if (predicate(row)) {
            result.push_back(row);
        }
    }
    return result;
}

void Table::update_many(
    const std::function<void(std::vector<DBType>&)>& updater,
    const std::function<bool(const std::vector<DBType>&)>& predicate) {
    for (auto row : rows_) {
        if (predicate(row)) {
            updater(row);
        }
    }
}

void Table::remove_many(
    const std::function<bool(const std::vector<DBType>&)>& predicate) {
    std::vector<RowType> rows_to_remove;
    for (auto& row : rows_) {
        if (predicate(row)) {
            rows_to_remove.push_back(row);
        }
    }
    for (auto& row : rows_to_remove) {
        rows_.erase(std::find(rows_.begin(), rows_.end(), row));
    }
}

void Table::addUniqueConstraint(const std::string& columnName) {
    auto it = std::find_if(
        scheme_.begin(), scheme_.end(),
        [&](const ColumnDefinition& col) { return col.name == columnName; });
    if (it == scheme_.end()) {
        throw std::runtime_error("Column not found: " + columnName);
    }
    it->isUnique = true;
}

void Table::insert_row(RowType row) {
    if (row.size() > scheme_.size()) {
        throw std::runtime_error("Number of values exceeds number of columns.");
    }

    for (size_t i = 0; i < scheme_.size(); ++i) {
        if (scheme_[i].isAutoIncrement) {
            if (!std::holds_alternative<int>(row[i])) {
                throw std::runtime_error(
                    "AutoIncrement is only applicable to integer columns.");
            }

            int value = std::get<int>(row[i]);
            if (value == 0) {
                row[i] = autoIncrementValues_[scheme_[i].name]++;
            } else {
                if (value >= autoIncrementValues_[scheme_[i].name]) {
                    autoIncrementValues_[scheme_[i].name] = value + 1;
                } else {
                    throw std::runtime_error(
                        "Cannot set AUTOINCREMENT value less than current "
                        "sequence: " +
                        scheme_[i].name);
                }
            }
        }
    }

    for (size_t i = 0; i < scheme_.size(); ++i) {
        if (scheme_[i].isUnique) {
            for (const auto& existing_row : rows_) {
                if (existing_row[i] == row[i]) {
                    throw std::runtime_error(
                        "Unique constraint violated for column: " +
                        scheme_[i].name);
                }
            }
        }

        if (scheme_[i].isKey) {
            if (indexes_[scheme_[i].name].count(row[i])) {
                throw std::runtime_error(
                    "Key constraint violated for column: " + scheme_[i].name);
            }
            indexes_[scheme_[i].name].insert(row[i]);
        }
    }

    rows_.push_back(row);
}

void Table::addAutoIncrement(const std::string& columnName) {
    auto it = std::find_if(
        scheme_.begin(), scheme_.end(),
        [&](const ColumnDefinition& col) { return col.name == columnName; });
    if (it == scheme_.end()) {
        throw std::runtime_error("Column not found: " + columnName);
    }
    if (it->type != "INT") {
        throw std::runtime_error(
            "AutoIncrement is only applicable to integer columns.");
    }
    it->isAutoIncrement = true;
    autoIncrementValues_[columnName] = 0;
}

void Table::addKeyConstraint(const std::string& columnName) {
    addUniqueConstraint(columnName);
    indexes_[columnName] = {};
}

}  // namespace database
