#ifndef DATABASE_CONTROLLER_HSE_CALCULATOR_H
#define DATABASE_CONTROLLER_HSE_CALCULATOR_H

#include <string>
#include <vector>
#include <variant>
#include <iostream>
#include "../types.h"
#include <type_traits>
#include <stdexcept>

namespace calculator {

    using Value = database::DBType;

    template<typename T, typename Variant>
    T safeGet(const Variant& var) {
        if (std::holds_alternative<T>(var)) {
            return std::get<T>(var);
        } else if constexpr (std::is_same_v<T, double>) {
            if (std::holds_alternative<int>(var)) {
                return static_cast<double>(std::get<int>(var));
            }
        } else if constexpr (std::is_same_v<T, int>) {
            if (std::holds_alternative<double>(var)) {
                return static_cast<int>(std::get<double>(var));
            }
        }
        throw std::bad_variant_access();
    }

    class Calculator {
    public:
        Value add(const Value& a, const Value& b);
        Value subtract(const Value& a, const Value& b);
        Value multiply(const Value& a, const Value& b);
        Value divide(const Value& a, const Value& b);

        Value evaluate(const std::string& expression);
    
    private:
        std::vector<std::string> tokenize(const std::string& expression);
        int getPrecedence(const std::string& operator_);
        bool isOperator(const std::string& token);
        Value applyOperator(const std::string& op, const Value& a, const Value& b);
        bool applyLogicalOperator(const std::string& op, bool a, bool b);
    };

} // calculator

#endif //DATABASE_CONTROLLER_HSE_CALCULATOR_H