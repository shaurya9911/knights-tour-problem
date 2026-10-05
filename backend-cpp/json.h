#pragma once
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <stdexcept>

// A tiny hand-rolled JSON parser/serializer -- just enough to read the flat
// request bodies this API receives ({"boardSize":8,"startX":0,...}) and to
// build the response bodies it sends back. Not a general-purpose library;
// written from scratch so the whole HTTP+JSON path has no external
// dependencies beyond the C++ standard library.

enum class JsonType { Null, Bool, Number, String, Array, Object };

struct JsonValue {
    JsonType type = JsonType::Null;
    bool boolValue = false;
    double numberValue = 0;
    std::string stringValue;
    std::vector<JsonValue> arrayValue;
    std::map<std::string, JsonValue> objectValue;

    bool isNull() const { return type == JsonType::Null; }
    double asNumber(double def = 0) const { return type == JsonType::Number ? numberValue : def; }
    std::string asString(const std::string &def = "") const { return type == JsonType::String ? stringValue : def; }

    // Object field access; returns a Null JsonValue if the key is absent.
    const JsonValue &get(const std::string &key) const {
        static JsonValue nullVal;
        auto it = objectValue.find(key);
        return it == objectValue.end() ? nullVal : it->second;
    }

    bool has(const std::string &key) const {
        return objectValue.find(key) != objectValue.end();
    }
};

// Parses a JSON document from text. Throws std::runtime_error on malformed input.
JsonValue parseJson(const std::string &text);

// --- Small builder helpers used to hand-construct response JSON ---
std::string jsonEscape(const std::string &s);
