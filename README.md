### Aroww DB

Inspired by "Designing Data-Intensive Applications" book and
"What I cannot create I do not understand" quote by R. Feynman.

At a time of writing, simple key-value database, 
created for learning purposes only.

This repository consists
* database code that implement key-value storage
* simple protocol to encode and decode commands
* client library to access database commands
* Cython wrapper to use in fancy Python programs


### How to try this out
C++ code is launched via CMake targets.

For a quick setup build and launch `serv` target, which will 
instantiate files storage inside of the build directory and start the server
on `localhost:7333`

As a client, you can use simple Python wrapper which is inside of the `client` folder.
Running `make` inside this folder would compile Python C Extension module which you can 
import and try by yourself.

`make connect` shortcut opens Python REPL with library loaded and `db` variable initialized.
You can also take a look at the `test.py` file to see how wrapper is used:

```python
import aroww_db

db = aroww_db.ArowwDB("localhost", "7333")

db.set("my greetings", "Hello world!")
assert db.get("my greetings") == "Hello world!"

assert db.get("nothing") is None

db.drop('my greetings')
assert db.get('my greetings') is None
```

Restarting server and client keeps the keys persistent, so
the state of the db would be restored.


As for other C++ CMake targets, there are.
* `arrow-db` which builds shared library with client-code only
* `tests` (also aliased as `test`), which runs test cases for the library
* `aroww-db-for-tests`, which build shared library specificaly for tests
