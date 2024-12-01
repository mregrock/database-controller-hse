#ifndef DATABASE_CONTROLLER_HSE_SQLSTATEMENT_H
#define DATABASE_CONTROLLER_HSE_SQLSTATEMENT_H

#include <memory>
#include <string>
#include <vector>

#include "../../types.h"

namespace database {
class SQLStatement {
   public:
    virtual ~SQLStatement() = default;
    virtual std::string toString() const = 0;
};

class CreateTableStatement : public SQLStatement {
   public:
    std::string tableName;
    SchemeType columns;

    std::string toString() const override {
        std::string result = "CREATE TABLE " + tableName + " (";
        for (size_t i = 0; i < columns.size(); ++i) {
            result += columns[i].toString();
            if (i != columns.size() - 1) {
                result += ", ";
            }
        }
        result += ");";
        return result;
    }
};

class InsertStatement : public SQLStatement {
   public:
    std::string tableName;
    std::vector<std::string> values;
    std::unordered_map<std::string, std::string> columnValuePairs;
    bool isMapFormat = false;

    std::string toString() const override {
        std::string result = "INSERT INTO " + tableName + " VALUES (";
        for (size_t i = 0; i < values.size(); ++i) {
            result += values[i];
            if (i != values.size() - 1) {
                result += ", ";
            }
        }
        result += ");";
        return result;
    }
};

class SelectStatement : public SQLStatement {
   public:
    std::string tableName;
    std::vector<std::string> columnNames;
    std::string predicate;

    std::string toString() const override {
        std::string result = "SELECT ";
        for (size_t i = 0; i < columnNames.size(); ++i) {
            result += columnNames[i];
            if (i != columnNames.size() - 1) {
                result += ", ";
            }
        }
        result += " FROM " + tableName;
        if (!predicate.empty()) {
            result += " WHERE " + predicate;
        }
        result += ";";
        return result;
    }
};

class UpdateStatement : public SQLStatement {
   public:
    std::string tableName;
    std::unordered_map<std::string, std::string> newValues;
    std::string predicate;

    std::string toString() const override {
        std::string result = "UPDATE ";
        bool isFirst = true;
        for (const auto& [column, value] : newValues) {
            if (!isFirst) {
                result += ", ";
            }
            result += column + " = " + value;
            isFirst = false;
        }
        result += " FROM " + tableName;
        if (!predicate.empty()) {
            result += " WHERE " + predicate;
        }
        result += ";";
        return result;
    }
};

class DeleteStatement : public SQLStatement {
   public:
    std::string tableName;
    std::string predicate;

    std::string toString() const override {
        std::string result = "DELETE FROM " + tableName;
        if (!predicate.empty()) {
            result += " WHERE " + predicate;
        }
        result += ";";
        return result;
    }
};

}  // namespace database

#endif  // DATABASE_CONTROLLER_HSE_SQLSTATEMENT_H