#pragma once

#include <string>
#include <sstream>
#include <vector>

// defines

constexpr inline int max_msg_length = 5000;
constexpr inline char msg_end[] = "<END>";

// structures

struct Contact
{
    int id;
    std::string name;
    std::string picture;
    int chat;
    std::vector<int> participants;
    bool saved_in_db;

    inline std::string serialize(char delimiter = ';') const
    {
        std::ostringstream oss;
        oss << id << delimiter 
            << name << delimiter
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
        std::getline(iss, contact.picture, delimiter);
        std::getline(iss, token, delimiter);
        contact.chat = std::stoi(token);

        return contact;
    }
};
