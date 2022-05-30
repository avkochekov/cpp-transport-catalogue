#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node() = default;

    template<typename T>
    Node(T value) : value_(std::move(value)) {}


    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const; //Возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    bool IsInt() const;
    bool IsDouble() const; // Возвращает true, если в Node хранится int либо double.
    bool IsPureDouble() const; // Возвращает true, если в Node хранится double.
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;

    const Value& GetValue() const;

private:
    Value value_;
};

// Шаблон, подходящий для вывода double и int
template <typename Value>
void PrintValue(const Value& value, std::ostream& out) {
    out << value;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, std::ostream& out);

// Перегрузка функции PrintValue для вывода значений double
void PrintValue(double value, std::ostream& out);

// Перегрузка функции PrintValue для вывода значений int
void PrintValue(int value, std::ostream& out);

// Перегрузка функции PrintValue для вывода значений int
void PrintValue(bool value, std::ostream& out);

// Перегрузка функции PrintValue для вывода значений string
void PrintValue(std::string value, std::ostream& out);

// Перегрузка функции PrintValue для вывода значений Array
void PrintValue(Array value, std::ostream& out);

// Перегрузка функции PrintValue для вывода значений Dict|Map
void PrintValue(Dict value, std::ostream& out);

void PrintNode(const Node& node, std::ostream& out);


class Document {
public:
    explicit Document(Node root);

    bool operator==(const Document& other);
    bool operator!=(const Document& other);

    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json

