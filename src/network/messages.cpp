#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <memory>

#include "messages.hpp"


void pack_field(std::stringstream& ss, std::string field) {
    int64_t size = (int64_t) field.size();
    ss.write((char*) &(size), sizeof(int64_t));
    ss.write(field.c_str(), field.size());
}


void unpack_field(std::stringstream& ss, std::string& field) {
    std::uint64_t str_size;
    ss.read((char*)(&str_size), sizeof(str_size));

    field.resize(str_size);
    ss.read(&field[0], str_size);
}

std::string Message::pack_message() {
    // int64_t msg_size = 0;
    std::stringstream ss;

    char type = get_flag();
    // ss.write((char*) &msg_size, sizeof(msg_size));
    ss.write(&(type), 1);
    pack_fields(ss);
    // msg_size = ss.tellp();  // Last position as length

    // ss.seekp(ss.beg);

    // ss.write((char*) &msg_size, sizeof(msg_size));

    return ss.str();
}

void Message::pack_fields(std::stringstream&) {}

void MsgGetReq::pack_fields(std::stringstream& ss) {
    pack_field(ss, key);
}
void MsgSetReq::pack_fields(std::stringstream& ss) {
    pack_field(ss, key);
    pack_field(ss, value);
}
void MsgDropReq::pack_fields(std::stringstream& ss) {
    pack_field(ss, key);
}
void MsgGetOkResp::pack_fields(std::stringstream& ss) {
    pack_field(ss, val);
}
void MsgErrorResp::pack_fields(std::stringstream& ss) {
    pack_field(ss, error_msg);
}


std::unique_ptr<Message> Message::unpack_message(std::string s) {
    std::stringstream ss(s, std::ios::in);

    char msg_type;
    ss.read(&msg_type, 1);
    
    std::unique_ptr<Message> msg;

    if      (msg_type == 'G') msg = std::make_unique<MsgGetReq>();
    else if (msg_type == 'S') msg = std::make_unique<MsgSetReq>();
    else if (msg_type == 'D') msg = std::make_unique<MsgDropReq>();
    else if (msg_type == 'g') msg = std::make_unique<MsgGetOkResp>();
    else if (msg_type == 'm') msg = std::make_unique<MsgGetMissingResp>();
    else if (msg_type == 's') msg = std::make_unique<MsgUpdateOkResp>();
    else if (msg_type == 'e') msg = std::make_unique<MsgErrorResp>();
    msg->unpack_fields(ss); 
    return msg;
}

void Message::unpack_fields(std::stringstream&) {}

void MsgGetReq::unpack_fields(std::stringstream& ss) {
    unpack_field(ss, key);
}
void MsgSetReq::unpack_fields(std::stringstream& ss) {
    unpack_field(ss, key);
    unpack_field(ss, value);
}
void MsgDropReq::unpack_fields(std::stringstream& ss) {
    unpack_field(ss, key);
}
void MsgGetOkResp::unpack_fields(std::stringstream& ss) {
    unpack_field(ss, val);
}
void MsgErrorResp::unpack_fields(std::stringstream& ss) {
    unpack_field(ss, error_msg);
}



