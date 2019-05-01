import requests
import json
import datetime
import sys

def url_get( u ):
        return requests.get( u, timeout=5 )

def url_get_w_param( u, d ):
        return requests.get( u, params=d, timeout=5 )

def url_post( u, d=None, h=None ):
        return requests.post( u, headers=h, data=d, timeout=5 )


if len( sys.argv ) < 2:
	print('argv[1]: filename')
	sys.exit(1)

# Document
hdr = { 'Content-Type':'application/json', 'Accept':'application/json' }

with open(sys.argv[1], 'r' ) as f:
	ll = f.readlines()
	
for i in range(len(ll)/2):
	r = url_post( ll[2*i].strip(), ll[2*i+1].strip(), hdr)

# Get IndoorGML
r = url_get( ll[0].strip() )
print (r.text)
