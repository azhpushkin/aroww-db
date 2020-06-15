import sys
from distutils.core import setup, Extension
from Cython.Build import cythonize

compile_args = ['-g', '-std=c++17']

# TEMP directives, for testing
import pathlib
path = pathlib.Path(__file__).absolute()
repo_root = path.parent.parent

debug_options = dict(
    include_dirs=[str(repo_root/'src'/'lib')],
    library_dirs=[str(repo_root/'build'/'src')],
    runtime_library_dirs=[str(repo_root/'build'/'src')],
)


basics_module = Extension(
    'aroww_db',
    sources=['aroww_db.pyx'],
    extra_compile_args=compile_args,
    libraries=["aroww-db"],
    language='c++',

    **debug_options
)




setup(
    name='aroww_db',
    packages=['aroww_db'],
    ext_modules=cythonize(basics_module, compiler_directives={'language_level' : "3"})
)