import string
import random


def ranstr(length):
    letters = string.printable[:-10]  # cut \t \n etc
    return ''.join(random.choice(letters) for _ in range(length))


AMOUNT = 10_000

GET_KEYS = open('get_keys.txt', 'w')
SET_KEYS = open('set_keys.txt', 'w')
SET_VALUES = open('set_values.txt', 'w')
DROP_KEYS = open('drop_keys.txt', 'w')

# generating GET_DATA
for _ in range(AMOUNT):
    GET_KEYS.write(ranstr(int(random.uniform(10, 100))))
    GET_KEYS.write('\n')
    
    SET_KEYS.write(ranstr(int(random.uniform(10, 100))))
    SET_KEYS.write('\n')
    
    SET_VALUES.write(ranstr(int(random.uniform(10, 100))))
    SET_VALUES.write('\n')
    
    DROP_KEYS.write(ranstr(int(random.uniform(10, 100))))
    DROP_KEYS.write('\n')

GET_KEYS.close()
SET_KEYS.close()
SET_VALUES.close()
DROP_KEYS.close()
