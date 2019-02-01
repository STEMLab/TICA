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
	print('argv[2] (optional): id')
	print('argv[3] (optional): server_info ( e.g. http://127.0.0.1:9797 )')
	sys.exit(1)

docId = 'model'
if len( sys.argv ) >= 3:
	docId = sys.argv[2]
docId += '_' + datetime.datetime.today().strftime('%Y%m%d%H%M%S')
ifId = 'if1'
psfId = 'psf1'

base_url = 'http://127.0.0.1:9797/'
if len(sys.argv) >= 4:
	base_url = sys.argv[3] + '/'

doc_url = base_url + 'documents/' + docId

# Document
hdr = { 'Content-Type':'application/json', 'Accept':'application/json' }
dt = { 'docId': docId }
r = url_post( doc_url, json.dumps(dt), hdr )

# IndoorFeatures
dt = { 'docId': docId, 'id': ifId }
r = url_post( doc_url + '/indoorfeatures/' + ifId, json.dumps(dt), hdr )

# PrimalSpaceFeatures
dt = { 'docId': docId, 'parentId': ifId, 'id': psfId }
r = url_post( doc_url + '/primalspacefeatures/' + psfId, json.dumps(dt), hdr )

# Cellspace
def make_cellspace( cid, wkt ):
	return { 'docId': docId, 'parentId': psfId, 'id': cid,
		'geometry': { 'type': 'Solid',
			'coordinates': wkt,
			'properties': { 'id': cid + 'g',
				'type': 'wkt' } } }

with open( sys.argv[1], 'r' ) as f:
	ll = f.readlines();
	f.close()

cell_i = 0
boundary_offset = 0
while ll:
	cell_i = cell_i + 1
	cellId = 'cellspace' + str( cell_i )

	npoly = int(ll[0])
	i = npoly
	l = 1;
	polygons = []
	while i > 0:
		nv = int(ll[l])
		vinfo = ll[l+1:l+1+nv]
		vinfo.append( vinfo[0] )
		vinfo = [ t.strip() for t in vinfo ]
		polygons.append( '((' + ','.join( vinfo ) + '))' )
		l += nv+1
		i -= 1
	wktext = 'SOLID (( ' + ','.join( polygons ) + ' ))'

	nanno = int(ll[l])
	i = nanno
	l += 1
	annotations = []
	while i > 0:
		nv = int(ll[l])
		vinfo = ll[l+1:l+1+nv]
		vinfo.append( vinfo[0] )
		vinfo = [ t.strip() for t in vinfo ]
		annotations.append( '((' + ','.join( vinfo ) + '))' )
		l += nv+1
		i -= 1

	boundaryIds = [ 'boundary' + str(ii+boundary_offset) for ii in range( len(annotations ) ) ]

	dt = make_cellspace( cellId, wktext )
	dt[ 'properties' ] = { 'partialboundedBy': boundaryIds }
	r = url_post( doc_url + '/cellspace/' + cellId, json.dumps(dt), hdr )

	# Cellspace boundary
	def make_cellspaceboundary( cid, wkt ):
		return { 'docId': docId, 'parentId': psfId, 'id': boundaryId,
			'geometry': { 'type': 'Polygon',
				'coordinates': wkt,
				'properties': { 'id': cid + 'g',
					'type': 'wkt' } } }

	for i in range( len(annotations ) ):
		boundaryId = 'boundary' + str(i+boundary_offset)
		wktext = 'POLYGON ' + annotations[i]
		dt = make_cellspaceboundary( boundaryId, wktext )
		r = url_post( doc_url + '/cellspaceboundary/' + boundaryId, json.dumps(dt), hdr )

	boundary_offset += len(annotations)
	ll = ll[l:]

# Get IndoorGML
r = url_get( doc_url )
print (r.text)
