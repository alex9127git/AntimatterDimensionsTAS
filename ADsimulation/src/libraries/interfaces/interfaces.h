#pragma once
#include <nlohmann/json.hpp>
#include <string>
using namespace std;
using json = nlohmann::json;


class ISerializable {
    public:
        virtual json to_json() = 0;
        virtual void from_json(json& j) = 0;
};

template <typename T>
void readIfPresent(json& j, const string& key, T& field) {
    if (j.contains(key) && !j[key].is_null()) field = T(j[key]);
}