#include "Executor.h"
#include "../Parser/Parser.h"
#include "../../database/Database/Database.h"
#include <gtest/gtest.h>

using namespace database;

class ExecutorTest : public ::testing::Test {
protected:
    Database db;
    Executor executor{db};
};

TEST_F(ExecutorTest, ExecuteCreateTable) {
    auto stmt = Parser::parse("CREATE TABLE Test (ID INT, Name VARCHAR);");
    executor.execute(*stmt);
    const auto& table = db.getTable("Test");
    EXPECT_EQ(table.getName(), "Test");
    ASSERT_EQ(table.getColumns().size(), 2);
    EXPECT_EQ(table.getColumns()[0].name, "ID");
    EXPECT_EQ(table.getColumns()[1].name, "Name");
}

TEST_F(ExecutorTest, ExecuteInsert) {
    auto createStmt = Parser::parse("CREATE TABLE Test (ID INT, Name VARCHAR);");
    executor.execute(*createStmt);

    auto insertStmt = Parser::parse("INSERT INTO Test VALUES (1, \"Alice\");");
    executor.execute(*insertStmt);

    const auto& table = db.getTable("Test");
    const auto& data = table.getData();
    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(std::get<int>(data[0][0]), 1);
    EXPECT_EQ(std::get<std::string>(data[0][1]), "Alice");
}

TEST_F(ExecutorTest, ExecuteMultipleInserts) {
    auto createStmt = Parser::parse("CREATE TABLE Test (ID INT, Name VARCHAR);");
    executor.execute(*createStmt);

    auto insertStmt1 = Parser::parse("INSERT INTO Test VALUES (1, \"Alice\");");
    executor.execute(*insertStmt1);

    auto insertStmt2 = Parser::parse("INSERT INTO Test VALUES (2, \"Bob\");");
    executor.execute(*insertStmt2);

    const auto& table = db.getTable("Test");
    const auto& data = table.getData();
    ASSERT_EQ(data.size(), 2);
    EXPECT_EQ(std::get<int>(data[0][0]), 1);
    EXPECT_EQ(std::get<std::string>(data[0][1]), "Alice");
    EXPECT_EQ(std::get<int>(data[1][0]), 2);
    EXPECT_EQ(std::get<std::string>(data[1][1]), "Bob");
}

TEST_F(ExecutorTest, ExecuteInsertWithExpression) {
    auto createStmt = Parser::parse("CREATE TABLE Test (ID INT, Name VARCHAR, Age INT);");
    executor.execute(*createStmt);

    auto insertStmt = Parser::parse("INSERT INTO Test VALUES (1, \"Alice\", 20 + 5);");
    executor.execute(*insertStmt);

    const auto& table = db.getTable("Test");
    const auto& data = table.getData();
    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(std::get<int>(data[0][0]), 1);
    EXPECT_EQ(std::get<std::string>(data[0][1]), "Alice");
    EXPECT_EQ(std::get<int>(data[0][2]), 25);
}

TEST_F(ExecutorTest, ExecuteInsertWithBoolean) {
    auto createStmt = Parser::parse("CREATE TABLE Test (ID INT, Name VARCHAR, Active BOOL);");
    executor.execute(*createStmt);

    auto insertStmt = Parser::parse("INSERT INTO Test VALUES (1, \"Alice\", true);");
    executor.execute(*insertStmt);

    const auto& table = db.getTable("Test");
    const auto& data = table.getData();
    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(std::get<int>(data[0][0]), 1);
    EXPECT_EQ(std::get<std::string>(data[0][1]), "Alice");
    EXPECT_EQ(std::get<bool>(data[0][2]), true);
}

TEST_F(ExecutorTest, ExecuteInsertWithInvalidType) {
    auto createStmt = Parser::parse("CREATE TABLE Test (ID INT, Name VARCHAR);");
    executor.execute(*createStmt);

    EXPECT_THROW({
        auto insertStmt = Parser::parse("INSERT INTO Test VALUES (\"Alice\", 1);");
        executor.execute(*insertStmt);
    }, std::runtime_error);
}

TEST_F(ExecutorTest, ExecuteCreateTableDuplicate) {
    auto createStmt = Parser::parse("CREATE TABLE Test (ID INT, Name VARCHAR);");
    executor.execute(*createStmt);

    EXPECT_THROW({
        auto duplicateStmt = Parser::parse("CREATE TABLE Test (ID INT, Name VARCHAR);");
        executor.execute(*duplicateStmt);
    }, std::runtime_error);
}

TEST_F(ExecutorTest, ExecuteInsertIntoNonExistentTable) {
    EXPECT_THROW({
        auto insertStmt = Parser::parse("INSERT INTO NonExistent VALUES (1, \"Alice\");");
        executor.execute(*insertStmt);
    }, std::runtime_error);
}

TEST_F(ExecutorTest, ExecuteComplexInsert) {
    auto createStmt = Parser::parse("CREATE TABLE Test (ID INT, Name VARCHAR, Age INT, Active BOOL);");
    executor.execute(*createStmt);

    auto insertStmt = Parser::parse("INSERT INTO Test VALUES (1, \"Alice\", 20 + 5, true && false);");
    executor.execute(*insertStmt);

    const auto& table = db.getTable("Test");
    const auto& data = table.getData();
    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(std::get<int>(data[0][0]), 1);
    EXPECT_EQ(std::get<std::string>(data[0][1]), "Alice");
    EXPECT_EQ(std::get<int>(data[0][2]), 25);
    EXPECT_EQ(std::get<bool>(data[0][3]), false);
}

TEST_F(ExecutorTest, ComplexScenario) {
    // Создаем таблицу со множеством колонок разных типов
    auto createStmt = Parser::parse(
        "CREATE TABLE Employees ("
        "    ID INT,"
        "    FirstName VARCHAR,"
        "    LastName VARCHAR,"
        "    Age INT,"
        "    Salary DOUBLE,"
        "    IsManager BOOL,"
        "    IsFullTime BOOL,"
        "    YearsOfService DOUBLE,"
        "    PerformanceScore INT"
        ");"
    );
    executor.execute(*createStmt);

    const auto& table = db.getTable("Employees");
    EXPECT_EQ(table.getName(), "Employees");
    ASSERT_EQ(table.getColumns().size(), 9);

    std::vector<std::string> insertStatements = {
        "INSERT INTO Employees VALUES (1, \"John\", \"Doe\", 30, 50000.50, true, true, 5.5, 95);",
        
        "INSERT INTO Employees VALUES (2, \"Jane\", \"Smith\", 25 + 3, 60000.0 * 1.1, false, true, (2.5 + 1.5) * 2, 100 - 10);",
        
        "INSERT INTO Employees VALUES (3, \"Bob\", \"Johnson\", ((40 - 5) + 2) * 1, 45000.0 + (1000.0 * 12), true && false, true || false, ((1.5 + 2.5) * 2) / 2, (85 + 5) * 1);",
        
        "INSERT INTO Employees VALUES (4, \"Alice\", \"Williams\", 20 + (2 * 5), 55000.0, true && (false || true), (true && true) || false, 3.5, (90 + 5) - (10 / 2));"
    };

    for (const auto& sql : insertStatements) {
        auto stmt = Parser::parse(sql);
        executor.execute(*stmt);
    }

    const auto& data = table.getData();
    ASSERT_EQ(data.size(), 4);

    EXPECT_EQ(std::get<int>(data[0][0]), 1);
    EXPECT_EQ(std::get<std::string>(data[0][1]), "John");
    EXPECT_EQ(std::get<std::string>(data[0][2]), "Doe");
    EXPECT_EQ(std::get<int>(data[0][3]), 30);
    EXPECT_DOUBLE_EQ(std::get<double>(data[0][4]), 50000.50);
    EXPECT_EQ(std::get<bool>(data[0][5]), true);
    EXPECT_EQ(std::get<bool>(data[0][6]), true);
    EXPECT_DOUBLE_EQ(std::get<double>(data[0][7]), 5.5);
    EXPECT_EQ(std::get<int>(data[0][8]), 95);

    EXPECT_EQ(std::get<int>(data[1][0]), 2);
    EXPECT_EQ(std::get<std::string>(data[1][1]), "Jane");
    EXPECT_EQ(std::get<std::string>(data[1][2]), "Smith");
    EXPECT_EQ(std::get<int>(data[1][3]), 28);
    EXPECT_DOUBLE_EQ(std::get<double>(data[1][4]), 66000.0);
    EXPECT_EQ(std::get<bool>(data[1][5]), false);
    EXPECT_EQ(std::get<bool>(data[1][6]), true);
    EXPECT_DOUBLE_EQ(std::get<double>(data[1][7]), 8.0);
    EXPECT_EQ(std::get<int>(data[1][8]), 90);

    EXPECT_EQ(std::get<int>(data[2][0]), 3);
    EXPECT_EQ(std::get<std::string>(data[2][1]), "Bob");
    EXPECT_EQ(std::get<std::string>(data[2][2]), "Johnson");
    EXPECT_EQ(std::get<int>(data[2][3]), 37);
    EXPECT_DOUBLE_EQ(std::get<double>(data[2][4]), 57000.0);
    EXPECT_EQ(std::get<bool>(data[2][5]), false);
    EXPECT_EQ(std::get<bool>(data[2][6]), true);
    EXPECT_DOUBLE_EQ(std::get<double>(data[2][7]), 4.0);
    EXPECT_EQ(std::get<int>(data[2][8]), 90);

    EXPECT_EQ(std::get<int>(data[3][0]), 4);
    EXPECT_EQ(std::get<std::string>(data[3][1]), "Alice");
    EXPECT_EQ(std::get<std::string>(data[3][2]), "Williams");
    EXPECT_EQ(std::get<int>(data[3][3]), 30);
    EXPECT_DOUBLE_EQ(std::get<double>(data[3][4]), 55000.0);
    EXPECT_EQ(std::get<bool>(data[3][5]), true);
    EXPECT_EQ(std::get<bool>(data[3][6]), true);
    EXPECT_DOUBLE_EQ(std::get<double>(data[3][7]), 3.5);
    EXPECT_EQ(std::get<int>(data[3][8]), 90);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 