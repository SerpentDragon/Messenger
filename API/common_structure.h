#pragma once

#include <string>
#include <sstream>
#include <vector>

struct Contact
{
    int id;
    std::string name;
    std::string public_key;
    std::string picture;
    int chat;
    std::vector<int> participants;
    bool saved_in_db;

    inline std::string serialize(char delimiter = ';') const
    {
        std::ostringstream oss;
        oss << id << delimiter 
            << name << delimiter
            << public_key << delimiter
            << picture << delimiter
            << chat;

        return oss.str();
    }

    inline static Contact deserialize(const std::string& str, char delimiter = ';')
    {
        std::istringstream iss(str);
        std::string token;

        Contact contact;

        std::getline(iss, token, delimiter);
        contact.id = std::stoi(token); token.clear();
        std::getline(iss, contact.name, delimiter);
        std::getline(iss, contact.public_key, delimiter);
        std::getline(iss, contact.picture, delimiter);
        std::getline(iss, token, delimiter);
        contact.chat = std::stoi(token);

        return contact;
    }
};
