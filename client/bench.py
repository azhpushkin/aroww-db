from connection import ArowwDB, ArowwResult
import tqdm
import uuid
import string
import random



KEYS_AMOUNT = 1_000

SET_AMOUNT = 10_000
GET_AMOUNT = 10_000

def get_key():
    val = random.gauss(KEYS_AMOUNT/2, KEYS_AMOUNT/2.3)
    if val >= KEYS_AMOUNT:
        return KEYS_AMOUNT - 1
    elif val < 0:
        return 0
    else:
        return int(val)



possible_keys = [
    ''.join(random.choices(string.hexdigits, k=random.randint(10, 10)))
    for _ in range(KEYS_AMOUNT)
]


set_keys = [get_key() for _ in range(SET_AMOUNT)]
set_values = [
    'x' * random.randint(10, 100)
    for _ in range(SET_AMOUNT)
]
get_keys = [get_key() for _ in range(GET_AMOUNT)]



db = ArowwDB()

for i in tqdm.tqdm(range(SET_AMOUNT)):
    j, v = set_keys[i], set_values[i]
    db.set(possible_keys[j], v)
    
for i in tqdm.tqdm(get_keys):
    db.get(possible_keys[i])
