from cython.operator cimport dereference as deref
from libcpp.memory cimport unique_ptr
from libcpp.string cimport string

from optional cimport optional


cdef extern from "aroww.hpp" namespace "aroww":
    cdef cppclass CppArowwDB "aroww::ArowwDB":
        CppArowwDB(string host, string port) except+

        string host
        string port
        int sockfd

        optional[string] get(string key) except +
        void set(string key, string value) except +
        void drop(string key) except +


cdef class ArowwDB:
    cdef unique_ptr[CppArowwDB] thisptr

    def __init__(self, host, port):
        self.thisptr.reset(new CppArowwDB(host, port))

    def get(self, key):
        cdef optional[string] result
        result = deref(self.thisptr).get(key)
        return result.value() if result.has_value() else None
    
    def set(self, key, value):
        return deref(self.thisptr).set(key, value)

    def drop(self, key):
        return deref(self.thisptr).drop(key)
