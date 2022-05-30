#include "json.h"

#include <unordered_map>
#include <unordered_set>

using namespace std;

namespace json {

namespace {


static const unordered_set<char> remove_escape_set = { '\n', '\r', '\t' };

std::string& RemoveEscapeSequencies(std::string &str)
{
    for (size_t i = 0; i < str.size(); ++i){
        if (remove_escape_set.count(str[i])){
            str.erase(i);
        }
    }
    return str;
}

std::string& RemoveBorderSpaces(std::string &str)
{
    auto left_border = str.find_first_not_of(' ');
    auto right_border = str.find_last_not_of(' ') + 1;
    str = str.substr(left_border, right_border);
    return str;
}

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    char c;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (c != ']')
        throw ParsingError("JSON array load error");

    return Node(move(result));
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::move(std::stoi(parsed_num)));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::move(std::stod(parsed_num)));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(std::move(s));
}

Node LoadDict(istream& input) {
    Dict result;

    char c;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if (c != '}')
        throw ParsingError("JSON dict|map load error");

    return Node(move(result));
}

Node LoadState(istream& input) {
    string line;
    char c;
    input >> c;
    while (std::isalpha(c)){
        line.push_back(c);
        input >> c;
    }
    input.putback(c);
    RemoveEscapeSequencies(line);
    RemoveBorderSpaces(line);
    if (line == "null")
        return Node(Node::Value());
    else if (line == "true")
        return Node(true);
    else if (line == "false")
        return Node(false);
    else
        throw ParsingError("null|true|false load error");
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (std::isalpha(c)) {
        input.putback(c);
        return LoadState(input);
    } else if (std::isdigit(c) || c == '-'){
        input.putback(c);
        return LoadNumber(input);
    } else {
        return LoadNode(input);
    }
}

}  // namespace

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}

bool Node::IsDouble() const {
    return IsPureDouble() || IsInt();
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(value_);
}

bool Node::IsNull() const {
    return std::holds_alternative<nullptr_t>(value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(value_);
}

bool Node::operator==(const Node &other) const {
    return value_ == other.value_;
}

bool Node::operator!=(const Node &other) const {
    return value_ != other.value_;
}

const Node::Value &Node::GetValue() const {
    return value_;
}

int Node::AsInt() const {
    if (IsInt())
        return std::get<int>(value_);
    throw std::logic_error("invalid value - int");
}

bool Node::AsBool() const {
    if (IsBool())
        return std::get<bool>(value_);
    throw std::logic_error("invalid value - bool");
}

double Node::AsDouble() const {
    if (IsPureDouble())
        return std::get<double>(value_);
    else if (IsInt())
        return std::get<int>(value_) * 1.;
    throw std::logic_error("invalid value - double");
}

const string &Node::AsString() const {
    if (IsString())
        return std::get<std::string>(value_);
    throw std::logic_error("invalid value - string");
}

const Array& Node::AsArray() const {
    if (IsArray())
        return std::get<Array>(value_);
    throw std::logic_error("invalid value - Array");
}

const Dict& Node::AsMap() const {
    if (IsMap())
        return std::get<Dict>(value_);
    throw std::logic_error("invalid value - Map|Dict");
}

Document::Document(Node root)
    : root_(move(root)) {
}

bool Document::operator==(const Document &other)
{
    return this->GetRoot() == other.GetRoot();
}

bool Document::operator!=(const Document &other)
{
    return this->GetRoot() != other.GetRoot();
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
}

void PrintValue(std::nullptr_t, std::ostream &out) {
    out << "null";
}

void PrintValue(double value, std::ostream &out) {
    out << value;
}

void PrintValue(int value, std::ostream &out) {
    out << value;
}

void PrintValue(bool value, std::ostream &out) {
    out << (value ? "true" : "false");
}

void PrintValue(std::string value, std::ostream &out) {
    out << "\"";
    for (char c : value) {
        switch (c) {
            case '\n':
                out << "\\n"sv;
                break;
            case '\"':
                out << "\\\""sv;
                break;
            case '\r':
                out << "\\r"sv;
                break;
            case '\\':
                out << "\\\\"sv;
                break;
            default:
                out << c;
        }
    }
    out << "\"";
}

void PrintValue(Array value, std::ostream &out) {
    out << "[";
    size_t count = value.size();
    for (const Node &node: value){
        PrintNode(node, out);
        if (--count)
            out << ',';
    }
    out << "]";
}

void PrintValue(Dict value, std::ostream &out) {
    out << "{";
    size_t count = value.size();
    for (const auto &[key, val]: value){
        out << "\"" << key << "\": ";
        PrintNode(val, out);
        if (--count)
            out << ", ";
    }
    out << "}";
}

void PrintNode(const Node &node, std::ostream &out) {
    std::visit(
                [&out](const auto& value){ PrintValue(value, out); },
    node.GetValue());
}

}  // namespace json
