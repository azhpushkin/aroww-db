from ctypes import *
import os
from pathlib import Path
from dataclasses import dataclass

ASCII = 'ascii'  # Avoid typos


class ArowwDBStruct(Structure): 
    _fields_ = [
        ('host', c_char_p),
        ('port', c_char_p),
        ('socket_fd', c_int),
    ]
    

class ArowwResultStruct(Structure): 
    _fields_ = [
        ('is_ok', c_bool),
        ('value', c_char_p),
        ('error_msg', c_char_p),
    ]
 
    
def init_library(libname: str):
    lib = cdll.LoadLibrary(libname)
    
    # ArowwDB* aroww_init(char* host, char* port);
    lib.aroww_init.argtypes = [c_char_p, c_char_p]
    lib.aroww_init.restype = POINTER(ArowwDBStruct)

    # void aroww_close(ArowwDB* db);
    lib.aroww_close.argtypes = [POINTER(ArowwDBStruct)]
    lib.aroww_close.restype = None

    # ArowwResult* aroww_get(ArowwDB* db, char* key);
    lib.aroww_get.argtypes = [POINTER(ArowwDBStruct), c_char_p, c_int]
    lib.aroww_get.restype = POINTER(ArowwResultStruct)

    # ArowwResult* aroww_set(ArowwDB* db, char* key, char* value);
    lib.aroww_set.argtypes = [POINTER(ArowwDBStruct), c_char_p, c_int, c_char_p, c_int]
    lib.aroww_set.restype = POINTER(ArowwResultStruct)

    # ArowwResult* aroww_drop(ArowwDB* db, char* key);
    lib.aroww_drop.argtypes = [POINTER(ArowwDBStruct), c_char_p, c_int]
    lib.aroww_drop.restype = POINTER(ArowwResultStruct)

    # void aroww_free_result(ArowwResult* res);
    lib.aroww_free_result.argtypes = [POINTER(ArowwResultStruct)]
    lib.aroww_free_result.restype = None

    return lib


REPO_PATH = Path(__file__).parent.parent
HARDCODED_PATH = REPO_PATH / 'build' / 'src' / 'libaroww-db.so'


@dataclass(frozen=True)
class ArowwResult:
    is_ok: bool
    value: str = None
    error_msg: str = None
    

class ArowwDB:
    def __init__(self, host="localhost", port=7333):
        self.host = host
        self.port = port
        
        self._lib = init_library(HARDCODED_PATH.absolute())
        self._db = self._lib.aroww_init(
            bytes(host, ASCII),
            bytes(str(port), ASCII)
        )
        
    def to_python(self, struct) -> ArowwResult:
        aroww_res = struct.contents
        repr = ArowwResult(
            is_ok=aroww_res.is_ok,
            value=aroww_res.value if aroww_res.value else None,
            error_msg=aroww_res.error_msg if aroww_res.error_msg else None,
        )
        # self._lib.aroww_free_result(struct)
        return repr
        
    
    def get(self, key: str):
        struct = self._lib.aroww_get(self._db, key.encode(ASCII), len(key.encode(ASCII)))
        return self.to_python(struct)
    
    def drop(self, key: str):
        struct = self._lib.aroww_drop(self._db, key.encode(ASCII), len(key.encode(ASCII)))
        return self.to_python(struct)
    
    def set(self, key: str, value: str):
        struct = self._lib.aroww_set(self._db, key.encode(ASCII), len(key.encode(ASCII)), value.encode(ASCII), len(value.encode(ASCII)))
        return self.to_python(struct)
    
    def __del__(self):
        self.close()  # just in case, lol
    
    def close(self):
        self._lib.aroww_close(self._db)
