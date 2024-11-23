#include "Parser.h"
#include <algorithm>

namespace database {

    std::unique_ptr<SQLStatement> Parser::parse(const std::string& sql) {
        Parser parser(sql);
        return parser.parseStatement();
    }

    Parser::Parser(const std::string& sql) : sql_(sql), pos_(0) {}

    std::unique_ptr<SQLStatement> Parser::parseStatement() {
        skipWhitespace();
        if (matchKeyword("CREATE")) {
            if (!matchKeyword("TABLE")) {
                throw std::runtime_error("Expected TABLE after CREATE");
            }
            return parseCreateTable();
        } else if (matchKeyword("INSERT")) {
            if (!matchKeyword("INTO")) {
                throw std::runtime_error("Expected INTO after INSERT");
            }
            return parseInsert();
        } else {
            throw std::runtime_error("Unsupported SQL statement.");
        }
    }

    std::unique_ptr<CreateTableStatement> Parser::parseCreateTable() {
        skipWhitespace();
        std::string tableName = parseIdentifier();
        skipWhitespace();
        
        if (sql_[pos_] != '(') {
            throw std::runtime_error("Expected '(' after table name.");
        }
        pos_++;
        skipWhitespace();

        auto createStmt = std::make_unique<CreateTableStatement>();
        createStmt->tableName = tableName;

        while (!isEnd()) {
            skipWhitespace();
            
            if (sql_[pos_] == ')') {
                pos_++;
                break;
            }

            std::string columnName = parseIdentifier();
            skipWhitespace();

            std::string columnType;
            while (pos_ < sql_.size() && 
                   sql_[pos_] != ',' && 
                   sql_[pos_] != ')' && 
                   !std::isspace(sql_[pos_])) {
                columnType += sql_[pos_++];
            }

            if (columnType.empty()) {
                throw std::runtime_error("Expected column type after column name");
            }

            createStmt->columns.push_back(ColumnDefinition{columnName, columnType});

            skipWhitespace();
            if (sql_[pos_] == ',') {
                pos_++;
                continue;
            }
        }

        skipWhitespace();
        if (pos_ >= sql_.size() || sql_[pos_] != ';') {
            throw std::runtime_error("Expected ';' at the end of CREATE TABLE statement.");
        }
        pos_++;

        return createStmt;
    }

    std::unique_ptr<InsertStatement> Parser::parseInsert() {
        skipWhitespace();
        std::string tableName = parseIdentifier();
        skipWhitespace();
        if (!matchKeyword("VALUES")) {
            throw std::runtime_error("Expected VALUES after table name.");
        }
        skipWhitespace();
        if (sql_[pos_] != '(') {
            throw std::runtime_error("Expected '(' after VALUES.");
        }
        pos_++;
        skipWhitespace();

        auto insertStmt = std::make_unique<InsertStatement>();
        insertStmt->tableName = tableName;

        while (sql_[pos_] != ')' && !isEnd()) {
            std::string value;
            if (sql_[pos_] == '"') {
                pos_++;
                while (pos_ < sql_.size() && sql_[pos_] != '"') {
                    value += sql_[pos_++];
                }
                if (sql_[pos_] != '"') {
                    throw std::runtime_error("Unterminated string literal.");
                }
                pos_++;
                value = "\"" + value + "\"";
            }
            else {
                value = parseToken();
            }
            insertStmt->values.push_back(value);
            skipWhitespace();
            if (sql_[pos_] == ',') {
                pos_++;
                skipWhitespace();
            }
        }

        if (sql_[pos_] != ')') {
            throw std::runtime_error("Expected ')' at the end of VALUES.");
        }
        pos_++;
        skipWhitespace();
        if (sql_[pos_] != ';') {
            throw std::runtime_error("Expected ';' at the end of INSERT statement.");
        }
        pos_++;
        return insertStmt;
    }

    void Parser::skipWhitespace() {
        while (pos_ < sql_.size() && std::isspace(sql_[pos_])) {
            pos_++;
        }
    }

    std::string Parser::parseIdentifier() {
        std::string identifier;
        skipWhitespace();
        
        if (pos_ >= sql_.size()) {
            throw std::runtime_error("Unexpected end of input while parsing identifier");
        }

        char firstChar = sql_[pos_];
        if (!(std::isalpha(firstChar) || firstChar == '_')) {
            throw std::runtime_error("Invalid identifier: must start with a letter or underscore");
        }

        identifier += sql_[pos_++];
        while (pos_ < sql_.size() && (std::isalnum(sql_[pos_]) || sql_[pos_] == '_')) {
            identifier += sql_[pos_++];
        }

        return identifier;
    }

    std::string Parser::parseToken() {
        std::string token;
        skipWhitespace();
        
        while (pos_ < sql_.size() && !std::isspace(sql_[pos_]) && sql_[pos_] != ',' && 
               sql_[pos_] != ')' && sql_[pos_] != ';' && sql_[pos_] != '"') {
            token += sql_[pos_++];
        }

        if (token.empty()) {
            throw std::runtime_error("Empty token");
        }

        return token;
    }

    bool Parser::matchKeyword(const std::string& keyword) {
        skipWhitespace();
        size_t len = keyword.length();
        
        if (pos_ + len > sql_.size()) {
            return false;
        }

        if (sql_.substr(pos_, len) == keyword) {
            if (pos_ + len == sql_.size() || std::isspace(sql_[pos_ + len]) || 
                sql_[pos_ + len] == '(' || sql_[pos_ + len] == ')' || 
                sql_[pos_ + len] == ',' || sql_[pos_ + len] == ';') {
                pos_ += len;
                skipWhitespace();
                return true;
            }
        }
        return false;
    }

    bool Parser::isEnd() const {
        return pos_ >= sql_.size();
    }

} // namespace database 