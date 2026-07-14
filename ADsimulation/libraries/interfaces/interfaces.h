#pragma once
#include <nlohmann/json.hpp>

using json = nlohmann::json;


class ISerializable {
    public:
        virtual json to_json() {
            return NULL;
        };
        virtual void from_json(json& j) {};
};