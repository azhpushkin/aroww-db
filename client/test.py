import aroww_db


db = aroww_db.ArowwDB("localhost", "7333")
db.set("123", "asd\x00asd")
assert db.get("123") == "asd\x00asd"  # zero is passed fine
assert len(db.get("123")) == 7
db.drop("123")
assert db.get("333") is None
assert db.get("123") is None
