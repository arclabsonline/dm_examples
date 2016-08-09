#include "helpers.h"
#include <intsafe.h>

#include <clocale>
#include <locale>
#include <vector>

#include <iostream>
#include <iosfwd>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <windows.h>
#include <ppltasks.h>

#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <cpprest/astreambuf.h>
#include <cpprest/astreambuf.h>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>   

#include <cassert>
#include <cctype>
#include <sys/stat.h>
using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams


#define BASE_URI L"http://www.dynamatrics.com"
#define PRIVATE_KEY "YourKey"
#define PUBLIC_KEY  "YourKey"
#define MAX_URI 1024


char* SHA1(char* data);
char* MD5(char* data);
unsigned char* HMAC_SHA1(char* data, char* key);
char *BASE64Encode(unsigned char *input, int length);
::std::string base64_decode(const ::std::string &ascdata);
char* GetDateTime();
void FetchChallenge(wchar_t *webUri, wchar_t *w_username, __out HBITMAP *bmp)
{
  *bmp = NULL;

  char *uri = (char*)malloc(MAX_URI);
  wcstombs(uri, webUri, MAX_URI);

  char username[1024];
  wcstombs(username, w_username, 1024);

  char content[1024];
  sprintf(content, "email=%s", username);

  char *date = GetDateTime();
  char *md5 = ""; //normally would be content's MD5
  char *contentType = "";

  char canonString[1024];
  sprintf(canonString, "%s,%s,%s,%s", contentType, md5, uri, date);

  unsigned char *HMAC = HMAC_SHA1(canonString, PRIVATE_KEY);
  char *b64 = BASE64Encode(HMAC, 20);

  char authString[1024];
  sprintf(authString, "APIAuth %s:%s", PUBLIC_KEY, b64);

  http_client_config config;
  http_client client(BASE_URI, config);

  http_request request(methods::POST);
  request.set_request_uri(webUri);
  request.set_body(content, contentType);

  request.headers().add(L"Authorization", authString);
  request.headers().add(L"Date", date);

  json::value json;

  client.request(request).then([](http_response response) -> pplx::task<json::value>
  {
    return response.extract_json();

  }).then([&json](pplx::task<json::value> previousTask)
  {
    try
    {
      json = previousTask.get();
    }
    catch (const http_exception& e)
    {
      json = NULL;
    }

  }).wait();

  if (json != NULL){

    if (json.has_field(U("challenge_image"))){

      json::value base64 = json.at(U("challenge_image"));

      string_t str = base64.as_string();

      std::setlocale(LC_ALL, "");
      const std::wstring ws = str;
      const std::locale locale("");
      typedef std::codecvt<wchar_t, char, std::mbstate_t> converter_type;
      const converter_type& converter = std::use_facet<converter_type>(locale);
      std::vector<char> to(ws.length() * converter.max_length());
      std::mbstate_t state;
      const wchar_t* from_next;
      char* to_next;
      const converter_type::result result = converter.out(state, ws.data(), ws.data() + ws.length(), from_next, &to[0], &to[0] + to.size(), to_next);

      if (result == converter_type::ok || result == converter_type::noconv) {

        const std::string s(&to[0], to_next);
        ::std::string image = base64_decode(s);

        char const *c = image.c_str();
        int len = image.length();

        FILE* file = fopen("dynaimg.bmp", "wb");

        if (file != NULL && c != NULL && len > 0){

          int l = fwrite(c, 1, len, file);
          fclose(file);

          if (l > 0){
            HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, L"dynaimg.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
            *bmp = hBitmap;
          }
        }
        else{
          fclose(file);
        }
      }
    }
  }
}

int AnswerChallenge(wchar_t* webUri, wchar_t* w_username, wchar_t* w_answer, __out wchar_t *login){

  if (webUri != NULL && wcslen(webUri) > 0 && w_username != NULL && wcslen(w_username) > 0 && w_answer != NULL && wcslen(w_answer) > 0)
  {
    http_client_config config;
    http_client client(BASE_URI, config);

    char *answerHash;
    char *uri = (char*)malloc(MAX_URI);
    char *date = GetDateTime();
    char content[1024];
    char md5[] = "";
    char *contentType;
    char canonString[1024];
    unsigned char *HMAC;
    char *b64;
    char authString[1024];

    char answer[1024];
    wcstombs(answer, w_answer, 1024);

    char username[1024];
    wcstombs(username, w_username, 1024);

    answerHash = SHA1(answer);

    contentType = "";

    wcstombs(uri, webUri, MAX_URI);
    sprintf(content, "email=%s&answer_hash=%s", username, answerHash);

    sprintf(canonString, "%s,%s,%s,%s", contentType, md5, uri, date);

    HMAC = HMAC_SHA1(canonString, PRIVATE_KEY);
    b64 = BASE64Encode(HMAC, 20);

    sprintf(authString, "APIAuth %s:%s", PUBLIC_KEY, b64);

    http_request request(methods::POST);

    request.set_request_uri(webUri);
    request.set_body(content, contentType);

    request.headers().add(L"Authorization", authString);
    request.headers().add(L"Date", date);

    json::value json;

    client.request(request).then([](http_response response) -> pplx::task<json::value>
    {
      if (response.status_code() == status_codes::OK)
      {
        return response.extract_json();
      }

      return pplx::task_from_result(json::value());

    }).then([&json](pplx::task<json::value> previousTask)
    {
      try
      {
        json = previousTask.get();

      }
      catch (const http_exception& e)
      {
        json = NULL;
      }

    }).wait();

    if (json != NULL){

      if (json.has_field(U("answer_success"))){

        json::value successObj = json.at(U("answer_success"));
        std::wstring sValue = successObj.as_string();

        if (json.has_field(U("answer_success"))){

          try {

            json::value loginObj = json.at(U("windows_login"));
            std::wstring loginValue = loginObj.as_string();

            wcscpy(login, loginValue.c_str());
          }
          catch (web::json::json_exception e) {}
        }

        return sValue.compare(L"true");
      }
    }
    else{

      return -1;
    }
  }

  return -1;
}