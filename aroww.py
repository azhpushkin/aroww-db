from ctypes import *
import os
from pathlib import Path

ASCII = 'ascii'
class Connection(Structure): 
    _fields_ = [('cpp_conn', c_void_p)]
    
class ConRes(Structure): 
    _fields_ = [
        ('is_ok', c_int),
        ('value', c_char_p),
        ('error_msg', c_char_p),
    ]
    
    
    
proj_dir = Path(__file__).parent
lib_dir = proj_dir / 'build' / 'src' / 'libaroww-db.so'

lib = cdll.LoadLibrary(lib_dir.absolute()) 

lib.open_connection.restype = POINTER(Connection)
lib.connection_get.restype = POINTER(ConRes)
lib.connection_drop.restype = POINTER(ConRes)
lib.connection_set.restype = POINTER(ConRes)


class ArowwDB:
    def __init__(self, host="localhost", port=7333):
        self.host = host
        self.port = port
        self._conn = lib.open_connection(
            bytes(host, ASCII),
            bytes(str(port), ASCII)
        )
        
    def _process(self, res):
        if not res.contents.is_ok:
            raise Exception(res.contents.error_msg.decode('ascii'))
        return res.contents.value.decode(ASCII)
    
    def get(self, key: str):
        res = lib.connection_get(self._conn, key.encode(ASCII))
        return self._process(res)
    
    def drop(self, key: str):
        res = lib.connection_drop(self._conn, key.encode(ASCII))
        return self._process(res)
    
    def set(self, key: str, value: str):
        res = lib.connection_set(self._conn, key.encode(ASCII), value.encode(ASCII))
        return self._process(res)
    
    def __del__(self):
        self.close()  # just in case, lol
    
    def close(self):
        lib.close_connection(self._conn)