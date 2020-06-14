#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <sstream>
#include <memory>

#include "serialization.hpp"

enum MsgType {
    GET_REQ = 'G',
    GET_RESP = 'g',

    SET_REQ = 'S',
    SET_RESP = 's',

    ERROR_RESP = 'e',
};


class UnknownMessageException : public std::exception {
public:
    std::string message;
    UnknownMessageException(std::string message_);
    const char * what () const throw ();
};

class Message {
public:    
    std::string pack_message();
    static std::unique_ptr<Message> unpack_message(std::string) noexcept(false);
    virtual char get_flag() = 0;
private:
    virtual void pack_fields(std::stringstream& ss);
    virtual void unpack_fields(std::stringstream& ss);
};

class MessageGetRequest: public Message {
public:
    std::string key;
    char get_flag() { return GET_REQ; };
private:
    void pack_fields(std::stringstream& ss);
    void unpack_fields(std::stringstream& ss);
};

class MessageSetRequest: public Message {
public:
    std::string key;
    string_or_tomb value;
    char get_flag() { return SET_REQ; };
private:
    void pack_fields(std::stringstream& ss);
    void unpack_fields(std::stringstream& ss);
};


class MessageGetResponse: public Message {
public:
    string_or_tomb value;
    char get_flag() { return GET_RESP; };
private:
    void pack_fields(std::stringstream& ss);
    void unpack_fields(std::stringstream& ss);
};

class MessageSetResponse: public Message {
public:
    char get_flag() { return SET_RESP; };
};

class MessageErrorResponse: public Message {
public:
    std::string error_msg;
    char get_flag() { return ERROR_RESP; };
private:
    void pack_fields(std::stringstream& ss);
    void unpack_fields(std::stringstream& ss); 
};
