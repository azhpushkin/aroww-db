# cython: c_string_type=unicode, c_string_encoding=utf8
# cython: binding=True

from cython.operator cimport dereference as deref
from libcpp.memory cimport unique_ptr
from libcpp.string cimport string

from optional cimport optional

cdef extern from "aroww.hpp" namespace "aroww":
    
    cdef cppclass CppArowwDB "aroww::ArowwDB":
        CppArowwDB(string host, string port) except+RuntimeError

        string host
        string port
        int sockfd

        optional[string] get(string key) except +RuntimeError
        void set(string key, string value) except +RuntimeError
        void drop(string key) except +RuntimeError


class ArowwException(RuntimeError):
    pass


cdef class ArowwDB:
    cdef unique_ptr[CppArowwDB] thisptr
    cdef public string host
    cdef public string port

    def __init__(self, host, port):
        self.thisptr.reset(new CppArowwDB(host, port))
        self.host = host
        self.port = port

    def get(self, key):
        cdef optional[string] result
        try:
            result = deref(self.thisptr).get(key)
            return result.value() if result.has_value() else None
        except RuntimeError as e:
            raise ArowwException(*e.args)
    
    def set(self, key, value):
        try:
            return deref(self.thisptr).set(key, value)
        except RuntimeError as e:
            raise ArowwException(*e.args)
        
    def drop(self, key):
        try:
            return deref(self.thisptr).drop(key)
        except RuntimeError as e:
            raise ArowwException(*e.args)
