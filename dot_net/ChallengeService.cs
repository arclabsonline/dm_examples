using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Reflection;
using System.Security.Cryptography;
using System.Security.Policy;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Web;

namespace DynamatricsExample.Models
{
    public class ChallengeService
    {
        const string PRIVATE_KEY = YOUR_KEY;
        const string PUBLIC_KEY = YOUR_KEY;

        public async Task<Challenge> PostRetrieveChallengeAsync(String email)
        {
            string create_uri = "https://www.dynamatrics.com/api/v1/challenge/create";

            var postData = new List<KeyValuePair<string, string>>();
            postData.Add(new KeyValuePair<string, string>("email", email));
            HttpContent content = new FormUrlEncodedContent(postData);

            HttpClient httpClient = new HttpClient();
            HttpResponseMessage response = await httpClient.PostAsync(create_uri, content);
            response.Content.Headers.ContentType = new MediaTypeHeaderValue("application/json");
            if (response.IsSuccessStatusCode)
            {
                string responseBody = await response.Content.ReadAsStringAsync();
                return JsonConvert.DeserializeObject<Challenge>(responseBody);

            }
            else
            {
                return null;
            }
        }

        public async Task<ChallengeAnswer> PostAnswerChallengeAsync(string email, string answer)
        {
            string dynamatrics_uri = "https://www.dynamatrics.com";
            string answer_uri = "/api/v1/challenge/answer";

            //setup security headers, sign request with private key
            DateTime current_date = DateTime.UtcNow;

            // example of format "Mon, 28 Sep 2015 20:41:26 GMT"
            string date_string = current_date.ToString("R");

            string md5 = "";
            string contentType = "";
            string canonString = $"{contentType},{md5},{answer_uri},{date_string}";
            string answerHash = SHA1(Encoding.ASCII.GetBytes((answer)));
            string b64 = HMAC(canonString, PRIVATE_KEY);
            string authString = $"APIAuth {PUBLIC_KEY}:{b64}";
            var postData = new List<KeyValuePair<string, string>>();
            postData.Add(new KeyValuePair<string, string>("email", email));
            postData.Add(new KeyValuePair<string, string>("answer_hash", answerHash));

            HttpContent content = new FormUrlEncodedContent(postData);
            HttpClient httpClient = new HttpClient();
            HttpRequestMessage request = new HttpRequestMessage();
            request.RequestUri = new Uri(dynamatrics_uri + answer_uri);
            request.Content = content;
            request.Content.Headers.ContentType = null;
            request.Headers.Add("Authorization", authString);
            request.Headers.Date = current_date;
            request.Method = new HttpMethod("Post");

            //send request
            HttpResponseMessage response = await httpClient.SendAsync(request);
            response.Content.Headers.ContentType = new MediaTypeHeaderValue("application/json");


            if (response.IsSuccessStatusCode)
            {
                string responseBody = await response.Content.ReadAsStringAsync();
                return JsonConvert.DeserializeObject<ChallengeAnswer>(responseBody);
            }
            else
            {
                return null;
            }
        }

        private static string HMAC(string input, string key)
        {

            var hmac = new HMACSHA1(Encoding.ASCII.GetBytes(key));
            string retval = Convert.ToBase64String(hmac.ComputeHash(Encoding.ASCII.GetBytes(input)));
            hmac.Dispose();

            return retval;
        }

        private static string ByteToString(IEnumerable<byte> data)
        {
            return string.Concat(data.Select(b => b.ToString("x2")));
        }

        private static byte[] StringToBytes(string str)
        {
            byte[] bytes = new byte[str.Length * sizeof(char)];
            System.Buffer.BlockCopy(str.ToCharArray(), 0, bytes, 0, bytes.Length);
            return bytes;
        }

        private string SHA1(byte[] seed)
        {
            using (SHA1Managed sha1 = new SHA1Managed())
            {
                var hash = sha1.ComputeHash(seed);
                return ByteToString(hash);
            }
        }

        public class Challenge
        {
            public string challenge { get; set; }
            public string challenge_hash { get; set; }
            public bool two_factor { get; set; }
            public string seed_time { get; set; }
            public string duration { get; set; }
            public string expiry { get; set; }
            public string error { get; set; }
        }

        public class ChallengeAnswer
        {
            public bool answer_success { get; set; }
        }
    }
}