#include "json.h"
#include <cctype>
#include <sstream>

namespace {

// Simple recursive-descent parser over a std::string, tracking a cursor
// position. Each parseX() function assumes leading whitespace has already
// been skipped and leaves the cursor just past what it consumed.
class Parser {
public:
    explicit Parser(const std::string &text) : s(text), pos(0) {}

    JsonValue parseValue() {
        skipWhitespace();
        if (pos >= s.size()) throw std::runtime_error("Unexpected end of JSON");
        char c = s[pos];
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') return parseString();
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') return parseNull();
        return parseNumber();
    }

private:
    const std::string &s;
    size_t pos;

    void skipWhitespace() {
        while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) pos++;
    }

    char peek() { return pos < s.size() ? s[pos] : '\0'; }

    void expect(char c) {
        if (pos >= s.size() || s[pos] != c) {
            throw std::runtime_error(std::string("Expected '") + c + "' at position " + std::to_string(pos));
        }
        pos++;
    }

    JsonValue parseObject() {
        JsonValue val;
        val.type = JsonType::Object;
        expect('{');
        skipWhitespace();
        if (peek() == '}') { pos++; return val; }
        while (true) {
            skipWhitespace();
            JsonValue key = parseString();
            skipWhitespace();
            expect(':');
            JsonValue value = parseValue();
            val.objectValue[key.stringValue] = value;
            skipWhitespace();
            if (peek() == ',') { pos++; continue; }
            break;
        }
        skipWhitespace();
        expect('}');
        return val;
    }

    JsonValue parseArray() {
        JsonValue val;
        val.type = JsonType::Array;
        expect('[');
        skipWhitespace();
        if (peek() == ']') { pos++; return val; }
        while (true) {
            val.arrayValue.push_back(parseValue());
            skipWhitespace();
            if (peek() == ',') { pos++; continue; }
            break;
        }
        skipWhitespace();
        expect(']');
        return val;
    }

    JsonValue parseString() {
        JsonValue val;
        val.type = JsonType::String;
        expect('"');
        std::string out;
        while (pos < s.size() && s[pos] != '"') {
            char c = s[pos++];
            if (c == '\\' && pos < s.size()) {
                char esc = s[pos++];
                switch (esc) {
                    case 'n': out += '\n'; break;
                    case 't': out += '\t'; break;
                    case 'r': out += '\r'; break;
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    default: out += esc; break;
                }
            } else {
                out += c;
            }
        }
        expect('"');
        val.stringValue = out;
        return val;
    }

    JsonValue parseBool() {
        JsonValue val;
        val.type = JsonType::Bool;
        if (s.compare(pos, 4, "true") == 0) { val.boolValue = true; pos += 4; }
        else if (s.compare(pos, 5, "false") == 0) { val.boolValue = false; pos += 5; }
        else throw std::runtime_error("Invalid literal near position " + std::to_string(pos));
        return val;
    }

    JsonValue parseNull() {
        JsonValue val;
        val.type = JsonType::Null;
        if (s.compare(pos, 4, "null") == 0) { pos += 4; }
        else throw std::runtime_error("Invalid literal near position " + std::to_string(pos));
        return val;
    }

    JsonValue parseNumber() {
        size_t start = pos;
        if (peek() == '-') pos++;
        while (pos < s.size() && (std::isdigit(static_cast<unsigned char>(s[pos])) || s[pos] == '.' ||
                                   s[pos] == 'e' || s[pos] == 'E' || s[pos] == '+' || s[pos] == '-')) {
            pos++;
        }
        if (pos == start) throw std::runtime_error("Invalid number at position " + std::to_string(pos));
        JsonValue val;
        val.type = JsonType::Number;
        val.numberValue = std::stod(s.substr(start, pos - start));
        return val;
    }
};

} // namespace

JsonValue parseJson(const std::string &text) {
    Parser parser(text);
    return parser.parseValue();
}

std::string jsonEscape(const std::string &s) {
    std::ostringstream out;
    for (char c : s) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n"; break;
            case '\t': out << "\\t"; break;
            case '\r': out << "\\r"; break;
            default: out << c;
        }
    }
    return out.str();
}
