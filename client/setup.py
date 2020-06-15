import sys
from distutils.core import setup, Extension
from Cython.Build import cythonize

compile_args = ['-g', '-std=c++17']


basics_module = Extension('pyarowwdb',
                sources=['aroww.pyx'],
                extra_compile_args=compile_args,
                libraries=["aroww-db"],
                library_dirs=["/home/maqquettex/projects/aroww-db/build/src"],
                language='c++')

setup(
    name='pyarowwdb',
    packages=['pyarowwdb'],
    ext_modules=cythonize(basics_module)
)