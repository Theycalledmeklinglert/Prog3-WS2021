#include "BoardRepository.hpp"
#include "Core/Exception/NotImplementedException.hpp"
#include <filesystem>
#include <string.h>

using namespace Prog3::Repository::SQLite;
using namespace Prog3::Core::Model;
using namespace Prog3::Core::Exception;
using namespace std;

#ifdef RELEASE_SERVICE
string const BoardRepository::databaseFile = "./data/kanban-board.db";
#else
string const BoardRepository::databaseFile = "../data/kanban-board.db";
#endif

BoardRepository::BoardRepository() : database(nullptr) {

    string databaseDirectory = filesystem::path(databaseFile).parent_path().string();

    if (filesystem::is_directory(databaseDirectory) == false) {
        filesystem::create_directory(databaseDirectory);
    }

    int result = sqlite3_open(databaseFile.c_str(), &database);

    if (SQLITE_OK != result) {
        cout << "Cannot open database: " << sqlite3_errmsg(database) << endl;
    }

    initialize();
}

BoardRepository::~BoardRepository() {
    sqlite3_close(database);
}

void BoardRepository::initialize() {
    int result = 0;
    char *errorMessage = nullptr;

    string sqlCreateTableColumn =
        "create table if not exists column("
        "id integer not null primary key autoincrement,"
        "name text not null,"
        "position integer not null UNIQUE);";

    string sqlCreateTableItem =
        "create table if not exists item("
        "id integer not null primary key autoincrement,"
        "title text not null,"
        "date text not null,"
        "position integer not null,"
        "column_id integer not null,"
        "unique (position, column_id),"
        "foreign key (column_id) references column (id));";

    result = sqlite3_exec(database, sqlCreateTableColumn.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
    result = sqlite3_exec(database, sqlCreateTableItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    // only if dummy data is needed ;)
    //createDummyData();
}

Board BoardRepository::getBoard() {
    Board board(boardTitle);
    vector<Column> colVec = getColumns();
    board.setColumns(colVec);
    return board;
}

std::vector<Column> BoardRepository::getColumns() {
    string sqlGetAllCol = "SELECT id, name, position FROM column";
    int result = 0;
    char *errorMessage = nullptr;
    vector<Column> cols;
    result = sqlite3_exec(database, sqlGetAllCol.c_str(), queryCallbackAllColumns, &cols, &errorMessage);
    handleSQLError(result, errorMessage);
    if (SQLITE_OK == result) {
        if (!cols.empty()) {
            vector<Item> itemVec;
            for (Column c : cols) {
                itemVec = getItems(c.getId());
                for (Item i : itemVec)
                    c.addItem(i);
            }
            return cols;
        }
    }
    vector<Column> emptyVec;
    return emptyVec;
}

std::optional<Column> BoardRepository::getColumn(int id) {
    string sqlGetCol = "SELECT * FROM column WHERE id = '" + to_string(id) + "'";
    string sqlGetColItems = "SELECT * FROM item WHERE column_id = '" + to_string(id) + "'";
    int result = 0;
    char *errorMessage = nullptr;
    Column test(-1, "", -1);
    result = sqlite3_exec(database, sqlGetCol.c_str(), queryCallbackColumn, &test, &errorMessage);
    handleSQLError(result, errorMessage);
    if (SQLITE_OK == result && (test.getId() != -1)) {
        vector<Item> itemVec;
        result = sqlite3_exec(database, sqlGetColItems.c_str(), queryCallbackItems, &itemVec, &errorMessage);
        handleSQLError(result, errorMessage);
        if (SQLITE_OK == result) {
            for (Item i : itemVec) {
                test.addItem(i);
            }
            return test;
        }
    }
    return std::nullopt;
}

std::optional<Column> BoardRepository::postColumn(std::string name, int position) {
    string sqlPostItem =
        "INSERT INTO column('name', 'position') "
        "VALUES('" +
        name + "', '" + to_string(position) + "')";

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlPostItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    if (SQLITE_OK == result) {
        int columnId = sqlite3_last_insert_rowid(database);
        return Column(columnId, name, position);
    }

    return std::nullopt;
}

std::optional<Prog3::Core::Model::Column> BoardRepository::putColumn(int id, std::string name, int position) {
    string sqlUpdateColumnRequest = "UPDATE column SET name = '" + name + "', position = '" + to_string(position) + "' WHERE id = '" + to_string(id) + "'"; // Beachte die "'" bei UPDATE SQL Requests
    string sqlSelectItems = "SELECT * FROM item WHERE column_id = '" + to_string(id) + "'";
    int result = 0;
    char *errorMessage = nullptr;

    //Update des Columns
    result = sqlite3_exec(database, sqlUpdateColumnRequest.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
    if (SQLITE_OK == result) { // Result ebenfalls gueltig wenn Column nicht in DB vorhanden ist.
        if (sqlite3_changes(database) == 1) {
            cout << "Column with " + to_string(id) + " updated successfully" << endl;
            vector<Item> itemVec;
            result = sqlite3_exec(database, sqlSelectItems.c_str(), queryCallbackItems, &itemVec, &errorMessage);
            handleSQLError(result, errorMessage);
            Column out2(id, name, position);
            for (Item item : itemVec) {
                out2.addItem(item);
            }
            return out2;
        }
    }
    cout << "Could not edit Column with ID: " + to_string(id) << endl;
    return std::nullopt;
}

void BoardRepository::deleteColumn(int id) {
    string deleteRequest = "DELETE FROM column WHERE id = " + to_string(id);
    char *errorMessage = nullptr;
    int result = sqlite3_exec(database, deleteRequest.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}

std::vector<Item> BoardRepository::getItems(int columnId) {
    std::string sqlGetColumn = "SELECT * FROM item WHERE column_id = " + to_string(columnId);
    int result = 0;
    char *errorMessage = nullptr;
    vector<Item> itemVec;
    result = sqlite3_exec(database, sqlGetColumn.c_str(), queryCallbackItems, &itemVec, &errorMessage);
    handleSQLError(result, errorMessage);
    if (SQLITE_OK == result) {
        if (!itemVec.empty()) {
            return itemVec;
        }
    }
    vector<Item> failVec;
    return failVec;
}

std::optional<Item> BoardRepository::getItem(int columnId, int itemId) {
    std::string sqlGetItem = "SELECT * FROM item WHERE column_id = " + to_string(columnId) + " AND id = " + to_string(itemId);
    char *errorMessage;
    int result = 0;
    Item test(-1, "", -1, "");
    result = sqlite3_exec(database, sqlGetItem.c_str(), queryCallbackSingleItem, &test, &errorMessage);
    handleSQLError(result, errorMessage);
    if (SQLITE_OK == result) {
        if (test.getId() != -1 && test.getId() != NULL)
            return test;
    }
    return std::nullopt;
}

std::optional<Item> BoardRepository::postItem(int columnId, std::string title, int position) {

    time_t now = time(0);
    char *datetime = ctime(&now);

    string sqlPostItem =
        "INSERT INTO item ('title', 'date', 'position', 'column_id') "
        "VALUES ('" +
        title + "', '" + datetime + "', " + to_string(position) + ", " + to_string(columnId) + ");";

    int result = 0;
    char *errorMessage = nullptr;

    result = sqlite3_exec(database, sqlPostItem.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    int itemId = INVALID_ID;
    if (SQLITE_OK == result) {
        itemId = sqlite3_last_insert_rowid(database);
        return Item(itemId, title, position, datetime);
    }
    return std::nullopt;
}

std::optional<Prog3::Core::Model::Item> BoardRepository::putItem(int columnId, int itemId, std::string title, int position) {
    string sqlUpdateItemRequest = "UPDATE item SET title = '" + title + "', position = '" + to_string(position) + "' WHERE column_id = '" + to_string(columnId) + "' AND id = '" + to_string(itemId) + "'";
    string sqlGetItem = "SELECT * FROM item WHERE id = " + to_string(itemId) + " AND column_id = " + to_string(columnId);
    int result = 0;
    char *errorMessage = nullptr;

    // Check if Item in DB
    result = sqlite3_exec(database, sqlUpdateItemRequest.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
    if (SQLITE_OK == result) {
        // Get updated Item
        Item out(-1, "", -1, "");
        result = sqlite3_exec(database, sqlGetItem.c_str(), queryCallbackSingleItem, &out, &errorMessage);
        handleSQLError(result, errorMessage);
        if (SQLITE_OK == result) {
            if (out.getId() != -1)
                return out;
        }
    }
    return std::nullopt;
}

void BoardRepository::deleteItem(int columnId, int itemId) {
    string deleteRequest = "DELETE FROM item WHERE id = " + to_string(itemId) + " AND column_id = " + to_string(columnId);
    char *errorMessage = nullptr;
    int result = sqlite3_exec(database, deleteRequest.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}

void BoardRepository::handleSQLError(int statementResult, char *errorMessage) {

    if (statementResult != SQLITE_OK) {
        cout << "SQL error: " << errorMessage << endl;
        sqlite3_free(errorMessage);
    }
}

void BoardRepository::createDummyData() {

    cout << "creatingDummyData ..." << endl;

    int result = 0;
    char *errorMessage;
    string sqlInsertDummyColumns =
        "insert into column (name, position)"
        "VALUES"
        "(\"prepare\", 1),"
        "(\"running\", 2),"
        "(\"finished\", 3);";

    result = sqlite3_exec(database, sqlInsertDummyColumns.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);

    string sqlInserDummyItems =
        "insert into item (title, date, position, column_id)"
        "VALUES"
        "(\"in plan\", date('now'), 1, 1),"
        "(\"some running task\", date('now'), 1, 2),"
        "(\"finished task 1\", date('now'), 1, 3),"
        "(\"finished task 2\", date('now'), 2, 3);";

    result = sqlite3_exec(database, sqlInserDummyItems.c_str(), NULL, 0, &errorMessage);
    handleSQLError(result, errorMessage);
}

/*
  I know source code comments are bad, but this one is to guide you through the use of sqlite3_exec() in case you want to use it.
  sqlite3_exec takes a "Callback function" as one of its arguments, and since there are many crazy approaches in the wild internet,
  I want to show you how the signature of this "callback function" may look like in order to work with sqlite3_exec()
*/
int BoardRepository::queryCallbackColumn(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {
    Column *sPointer = static_cast<Column *>(data);
    sPointer->setID(stoi(fieldValues[0]));
    sPointer->setName(fieldValues[1]);
    sPointer->setPos(stoi(fieldValues[2]));
    return 0;
}

int BoardRepository::queryCallbackItems(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {
    vector<Item> *itemList = static_cast<std::vector<Item> *>(data);
    Item temp(stoi(fieldValues[0]), fieldValues[1], stoi(fieldValues[3]), "TimestampPlaceholder");
    (*itemList).push_back(temp);
    return 0;
}
int BoardRepository::queryCallbackSingleItem(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {
    Item *itemP = static_cast<Item *>(data);
    Item item(stoi(fieldValues[0]), fieldValues[1], stoi(fieldValues[3]), "TimestampPlaceholder");
    *itemP = item;
    return 0;
}
int BoardRepository::queryCallbackAllColumns(void *data, int numberOfColumns, char **fieldValues, char **columnNames) {
    vector<Column> *colVecP = static_cast<vector<Column> *>(data);
    Column temp(stoi(fieldValues[0]), fieldValues[1], stoi(fieldValues[2]));
    (*colVecP).push_back(temp);
    return 0;
}
