from cython.operator cimport dereference as deref

from libcpp.memory cimport unique_ptr
from libcpp.string cimport string


cdef extern from "aroww.hpp" namespace "aroww":
    int addOne(int)

    cdef cppclass ArowwDB:
        ArowwDB(string host, string port) except+

        string host
        string port
        int sockfd

        void set(string key, string value) except +
        void drop(string key) except +


cdef class PyArowwDB:
    cdef unique_ptr[ArowwDB] thisptr

    def __init__(self, host, port):
        self.thisptr.reset(new ArowwDB(host, port))

    def set(self, key, value):
        return deref(self.thisptr).set(key, value)
