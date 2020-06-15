from aroww_db import ArowwDB


db = ArowwDB(b"localhost", b"7333")
db.set(b"123", b"asd")
print(db.get(b"123"))