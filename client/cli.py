import re
import argparse
from connection import ArowwDB, ArowwResult






if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Connect to ArowwDB server')
    parser.add_argument('-H,--host', type=str, dest='host', default="localhost",
                        help='Target host')
    parser.add_argument('-p,--port', type=int, dest='port', default="7333",
                        help='Target port')
    
    
    args = parser.parse_args()
    
    
    db = ArowwDB(host=args.host, port=args.port)
    
    
    get_re = re.compile(r'^get (?P<key>\S+)$') 
    set_re = re.compile(r'^set (?P<key>\S+) (?P<value>\S+)$') 
    drop_re = re.compile(r'^drop (?P<key>\S+)$') 
    
    
    while True:
        s = input("cli> ")
        s = s.strip()
        if s == '':
            continue
        elif s.lower() in ('q', 'quit', 'exit'):
            break
        elif s.lower() == 'help':
            print("Sorry, no help yet :C")
            continue
        elif gr := re.match(get_re, s):
            res = db.get(gr.group('key'))
            print(res)
        elif gr := re.match(set_re, s):
            res = db.set(gr.group('key'), gr.group('value'))
            print(res)
        elif gr := re.match(drop_re, s):
            res = db.drop(gr.group('key'))
            print(res)
        else:
            print('Bad command!')
            
        
        
