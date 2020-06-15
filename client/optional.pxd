# Copied from currently opened Cython issue:
# https://github.com/cython/cython/issues/3293)
# Only handful of methods left as I do not need them all
from libcpp cimport bool


cdef extern from "<optional>" namespace "std" nogil:
    cdef cppclass nullopt_t:
        nullopt_t()

    cdef nullopt_t nullopt

    cdef cppclass optional[T]:
        ctypedef T value_type
        bool has_value()
        T& value()
        
