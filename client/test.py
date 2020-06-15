from aroww_db import ArowwDB, ArowwException
import aroww_db


db = ArowwDB(b"localhost", b"7333")
db.set(b"123", b"asd")
print(db.get(b"123"))
db.drop(b"123")
print(db.get(b"333"))
