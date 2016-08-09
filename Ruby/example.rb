def hmac_headers(path, request_type = nil)
  request_type ||= "GET"

  if request_type == "GET"
    content_type = ''
  else 
     content_type = 'application/x-www-form-urlencoded'
  end

  digest = OpenSSL::Digest.new('sha1')
  now = Time.now.httpdate
  canonical_string = request_type + "\n" +
                "" + "\n" +
                content_type  + "\n" +
                now  + "\n" +
                path

  signature = OpenSSL::HMAC.digest(digest, PRIVATE_KEY, canonical_string) 
  encoded_signature = Base64.strict_encode64(signature)
  headers = { }
  headers.merge 'Date' => now, 'Authorization' => "APIAuth #{PUBLIC_KEY}:#{encoded_signature}"
end