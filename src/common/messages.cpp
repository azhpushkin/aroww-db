#include <iostream>
#include <cstring>
#include <sstream>
#include <memory>

#include "messages.hpp"
#include "serialization.hpp"


UnknownMessageException::UnknownMessageException(std::string message_): message(message_) {}
const char * UnknownMessageException::what () const throw () {
    return "Unknown message type";
}

void Message::pack_fields(std::stringstream&) {}
void Message::unpack_fields(std::stringstream&) {}

std::string Message::pack_message() {
    std::stringstream ss;

    pack_char(ss, get_flag());
    pack_fields(ss);

    return ss.str();
}

std::unique_ptr<Message> Message::unpack_message(std::string s) noexcept(false) {
    std::stringstream ss(s, std::ios::in);

    char msg_type;
    unpack_char(ss, msg_type);
    
    std::unique_ptr<Message> msg;

    if      (msg_type == 'G') msg = std::make_unique<MessageGetRequest>();
    else if (msg_type == 'S') msg = std::make_unique<MessageSetRequest>();
    else if (msg_type == 'g') msg = std::make_unique<MessageGetResponse>();
    else if (msg_type == 's') msg = std::make_unique<MessageSetResponse>();
    else if (msg_type == 'e') msg = std::make_unique<MessageErrorResponse>();
    else    throw UnknownMessageException(s);

    msg->unpack_fields(ss); 
    return msg;
}

void MessageGetRequest::pack_fields(std::stringstream& ss) {
    pack_string(ss, key);
}
void MessageSetRequest::pack_fields(std::stringstream& ss) {
    pack_string(ss, key);
    pack_string_or_tomb(ss, value);
}

void MessageGetResponse::pack_fields(std::stringstream& ss) {
    pack_string_or_tomb(ss, value);
}

void MessageErrorResponse::pack_fields(std::stringstream& ss) {
    pack_string(ss, error_msg);
}


void MessageGetRequest::unpack_fields(std::stringstream& ss) {
    unpack_string(ss, key);
}
void MessageSetRequest::unpack_fields(std::stringstream& ss) {
    unpack_string(ss, key);
    unpack_string_or_tomb(ss, value);
}
void MessageGetResponse::unpack_fields(std::stringstream& ss) {
    unpack_string_or_tomb(ss, value);
}
void MessageErrorResponse::unpack_fields(std::stringstream& ss) {
    unpack_string(ss, error_msg);
}



