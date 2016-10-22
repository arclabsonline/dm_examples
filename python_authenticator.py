from base64 import b64encode
from datetime import datetime
from datetime import tzinfo
from Crypto.Hash import SHA, HMAC
import hashlib
from urlparse import urlparse
import requests
import urllib
import json

private_key = "DsC2GORnPUNriZXMvceHwgtt"

public_key = "TQ28Pz95YgHbTRNQXrfghwtt"

dyna_host = "https://www.dynamatrics.com"

def get_challenge (email):
    dyna_uri = "/api/v1/challenge/create"
    dyna_url = dyna_host + dyna_uri
    content_type = "application/x-www-form-urlencoded"
    content = {'email': email}

    return make_dyna_request (content, dyna_uri)

def answer_challenge (email, answer):
    dyna_uri = "/api/v1/challenge/answer"
    answer_hash = hashlib.sha1(answer).hexdigest()

    content = {'email': email, "answer_hash": answer_hash}

    return make_dyna_request(content, dyna_uri)

def make_dyna_request(content, uri):
    dyna_uri = uri
    dyna_url = dyna_host + dyna_uri
    content_type = "application/x-www-form-urlencoded"

    content_string = urllib.urlencode(content)
    hashed_content_string = b64encode(hashlib.md5(content_string).digest())
    now = datetime.utcnow().strftime("%a, %d %b %Y %H:%M:%S GMT")
    #Just doing this as an example of how to do it
    url_parts = urlparse(dyna_url)

    canon_string = "{0},{1},{2},{3}".format(content_type, hashed_content_string, url_parts.path, now)
    
    #Couple of steps to turn it into a correctly b64 encoded string
    hmac = HMAC.new( private_key, canon_string, SHA )
    hmac_dig_str = hmac.digest()
    hmac_signed_str = b64encode( hmac_dig_str )

    #Create contents of the Authorization header
    signature = "APIAuth " + public_key + ":" + hmac_signed_str

    #make request with python requests lib
    r = requests.post(dyna_url, data=content, headers={'Authorization': signature, 'Date': now, 'Content-MD5': hashed_content_string, 'Content-Type': content_type})

    #return response object
    return r

#Command-line python auth system!
email = raw_input("Enter your Dyna email: ")
print "you entered", email

#get a challenge for this user
response_object = json.loads(get_challenge(email).text)
#print out the whole response for fun
print response_object
#Print out the challenge to console
print response_object['challenge']

answer = raw_input("Answer the challenge: ")
print "you entered", answer

answer_response_object = answer_challenge(email, answer)
print answer_response_object
