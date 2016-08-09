Response Syntax
---------------

These are the available DynaMatric rule formats.  Each DynaMatric contains four rules, which are separated by a ‘|’ character.

    {1-36},{1-36},{operator}

**or**    
    
    {1-36},c{constant[0-9]},+

1-36 is the cell in the matrix Left to right, top to bottom. No rule can use the same cell as another rule in the set.

Available operators are:
* Addition (+)

* Difference (-)

* Lesser (<)

* Greater (>)

In a DynaMatric you have the ability to set a constant instead of a corresponding cell.  Constants are prefixed by the letter c in order to not be confused with a cell. The constant must be between 0-9. Constants can only be added to a chosen cell. No other operators are valid.

This would be an example  of a valid DynaMatric string: 

    "1,36,+|6,c9,+|24,c0,+|3,19,-"
API Security
-----------

DynaMatrics utilizes a Hash-based message authentication code (HMAC) strategy for signing all requests to our API.   This is the same strategy Amazon uses to authenticate all Amazon Web Service requests (Documentation).  Here’s how it works:

A canonical string is first created using your HTTP headers containing the content-type, content-MD5, request URI and the timestamp. If content-type or content-MD5 are not present, then a blank string is used in their place. If the timestamp isn't present, a valid HTTP date is automatically added to the request. 
The canonical string string is computed as follows:
canonical_string = 'content-type,content-MD5,request URI,timestamp'
This string is then used to create the signature which is a Base64 encoded SHA1 HMAC, using the client's private secret key.
This signature is then added as the Authorization HTTP header in the form:
Authorization = APIAuth 'client access id':'signature from step 2'
On the server side, the SHA1 HMAC is computed in the same way using the request headers and the client's secret key, which is known to only the client and the server but can be looked up on the server using the client's access id that was attached in the header.  The signed request expires after 15 minutes in order to avoid replay attacks.
