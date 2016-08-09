# dm_examples
DynaMatrics Code Examples in various languages and libraries

The Authentication Header 
The Amazon S3 REST API uses the standard HTTP Authorization header to pass authentication information. (The name of the standard header is unfortunate because it carries authentication information, not authorization.) Under the Amazon S3 authentication scheme, the Authorization header has the following form:

Authorization: AWS AWSAccessKeyId:Signature
Developers are issued an AWS access key ID and AWS secret access key when they register. For request authentication, the AWSAccessKeyId element identifies the access key ID that was used to compute the signature and, indirectly, the developer making the request.

The Signature element is the RFC 2104 HMAC-SHA1 of selected elements from the request, and so the Signature part of the Authorization header will vary from request to request. If the request signature calculated by the system matches the Signature included with the request, the requester will have demonstrated possession of the AWS secret access key. The request will then be processed under the identity, and with the authority, of the developer to whom the key was issued.

Following is pseudogrammar that illustrates the construction of the Authorization request header. (In the example, \n means the Unicode code point U+000A, commonly called newline).


Authorization = "AWS" + " " + AWSAccessKeyId + ":" + Signature;

Signature = Base64( HMAC-SHA1( YourSecretAccessKeyID, UTF-8-Encoding-Of( StringToSign ) ) );

StringToSign = HTTP-Verb + "\n" +
  Content-MD5 + "\n" +
  Content-Type + "\n" +
  Date + "\n" +
  CanonicalizedAmzHeaders +
  CanonicalizedResource;

CanonicalizedResource = [ "/" + Bucket ] +
  <HTTP-Request-URI, from the protocol name up to the query string> +
  [ subresource, if present. For example "?acl", "?location", "?logging", or "?torrent"];

CanonicalizedAmzHeaders = <described below>

HMAC-SHA1 is an algorithm defined by RFC 2104 - Keyed-Hashing for Message Authentication . The algorithm takes as input two byte-strings, a key and a message. For Amazon S3 request authentication, use your AWS secret access key (YourSecretAccessKeyID) as the key, and the UTF-8 encoding of the StringToSign as the message. The output of HMAC-SHA1 is also a byte string, called the digest. The Signature request parameter is constructed by Base64 encoding this digest.
