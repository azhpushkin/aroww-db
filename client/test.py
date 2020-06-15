import pyarowwdb as aroww


db = aroww.PyArowwDB(b"localhost", b"7333")
db.set(b"123", b"asd")