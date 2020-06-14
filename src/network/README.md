## Protocol structure

Communication between client and server is performed via single set of messages.
Every message is composed from blocks of following objects:
* **char**  
Serialized as plain 1 byte
* **8-byte signed integer** (int64_t from cstdint)  
Serialized as plain 8 bytes in little endian.  
(Little endian is used as unified serialization
across all the code, not only network message)
* **string**  
Serialized as 8-bytes integer (length of string),
followed by corresponding amount of chars
* **nullable string** (referred to as **string_or_tomb** in the code)  
Serialized in the same way as usual string,
but can contain -1 as value of length (represents absence of value).


There are 