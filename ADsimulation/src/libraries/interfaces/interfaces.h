#pragma once
#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;


class ISerializable {
    public:
        virtual json to_json() const = 0;
        virtual void from_json(const json& j) = 0;
};

template <typename T>
void readIfPresent(const json& j, const std::string& key, T& field) {
    if (j.contains(key) && !j.at(key).is_null()) field = T(j.at(key));
}