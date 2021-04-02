#include "Types.hpp"

void mck::to_json(nlohmann::json &j, const Connection &c)
{
    j["name"] = c.name;
}
void mck::from_json(const nlohmann::json &j, Connection &c)
{
    c.name = j.at("name").get<std::string>();
}

void mck::to_json(nlohmann::json &j, const Port &p)
{
    j["name"] = p.name;
    j["fullName"] = p.fullName;
    j["isInput"] = p.isInput;
    j["connections"] = p.connections;
}
void mck::from_json(const nlohmann::json &j, Port &p)
{
    p.name = j.at("name").get<std::string>();
    p.fullName = j.at("fullName").get<std::string>();
    p.isInput = j.at("isInput").get<bool>();
    p.connections = j.at("connections").get<std::vector<Connection>>();
}

void mck::to_json(nlohmann::json &j, const Message &m)
{
    j["section"] = m.section;
    j["msgType"] = m.msgType;
    j["data"] = m.data;
}
void mck::from_json(const nlohmann::json &j, Message &m)
{
    m.section = j.at("section").get<std::string>();
    m.msgType = j.at("msgType").get<std::string>();
    m.data = j.at("data").get<std::string>();
}

void mck::to_json(nlohmann::json &j, const TriggerData &t)
{
    j["index"] = t.index;
    j["strength"] = t.strength;
}
void mck::from_json(const nlohmann::json &j, TriggerData &t)
{
    t.index = j.at("index").get<int>();
    t.strength = j.at("strength").get<double>();
}

void mck::to_json(nlohmann::json &j, const PadData &p)
{
    j["type"] = p.type;
    j["index"] = p.index;
    j["value"] = p.value;
}
void mck::from_json(const nlohmann::json &j, PadData &p)
{
    p.type = j.at("type").get<std::string>();
    p.index = j.at("index").get<int>();
    p.value = j.at("value").get<double>();
}

void mck::to_json(nlohmann::json &j, const SamplePackSample &s)
{
    j["path"] = s.path;
    j["name"] = s.name;
    j["type"] = s.type;
    j["index"] = s.index;
}
void mck::from_json(const nlohmann::json &j, SamplePackSample &s)
{
    s.path = j.at("path").get<std::string>();
    s.name = j.at("name").get<std::string>();
    s.type = j.at("type").get<unsigned>();
    s.index = j.at("index").get<unsigned>();
}

void mck::to_json(nlohmann::json &j, const SamplePack &s)
{
    j["name"] = s.name;
    j["picture"] = s.picture;
    j["sampleRate"] = s.sampleRate;
    j["numBits"] = s.numBits;
    j["numChannels"] = s.numChannels;
    j["categories"] = s.categories;
    j["samples"] = s.samples;
}
void mck::from_json(const nlohmann::json &j, SamplePack &s)
{
    s.name = j.at("name").get<std::string>();
    s.picture = j.at("picture").get<std::string>();
    s.sampleRate = j.at("sampleRate").get<unsigned>();
    s.numBits = j.at("numBits").get<unsigned>();
    s.numChannels = j.at("numChannels").get<unsigned>();
    s.categories = j.at("categories").get<std::vector<std::string>>();
    s.samples = j.at("samples").get<std::vector<SamplePackSample>>();
}