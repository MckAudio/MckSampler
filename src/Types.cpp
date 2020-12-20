#include "Types.hpp"

void MCK::to_json(nlohmann::json &j, const Connection &c)
{
    j["name"] = c.name;
}
void MCK::from_json(const nlohmann::json &j, Connection &c)
{
    c.name = j.at("name").get<std::string>();
}

void MCK::to_json(nlohmann::json &j, const Port &p)
{
    j["name"] = p.name;
    j["fullName"] = p.fullName;
    j["isInput"] = p.isInput;
    j["connections"] = p.connections;
}
void MCK::from_json(const nlohmann::json &j, Port &p)
{
    p.name = j.at("name").get<std::string>();
    p.fullName = j.at("fullName").get<std::string>();
    p.isInput = j.at("isInput").get<bool>();
    p.connections = j.at("connections").get<std::vector<Connection>>();
}