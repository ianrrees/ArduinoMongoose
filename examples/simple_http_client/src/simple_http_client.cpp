//
// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s
//

#include <Arduino.h>
#include <MongooseCore.h>
#include <MongooseHttpClient.h>

#ifdef ESP32
#include <WiFi.h>
#define START_ESP_WIFI
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#define START_ESP_WIFI
#else
#error Platform not supported
#endif

#if ARDUINO_MONGOOSE_SSL_SUPPORTED()
  #define PROTO "https"
#else
  #define PROTO "http"
#endif

MongooseHttpClient client;

const char *ssid = "wifi";
const char *password = "password";

#if ARDUINO_MONGOOSE_SSL_SUPPORTED()
// Root CA bundle
//
// To create a bundle like this, one option is to use openssl (on a regular
// computer) to dump a chain of certificates to the server in question.  For
// example:
//   openssl s_client --connect jsonplaceholder.typicode.com:443 -showcerts
// The output will include a fair bit more information than we need,
// just extract the bits and format like below.
//
// To inspect a certificate, save it as a plain text file (remove the double
// quotes and \r\n escapes) and use openssl:
//   openssl x509 -text -in my_cert_file.crt -noout
//
// Mongoose requires that the first character in root_ca is '-'.  This comes
// about because we don't have a filesystem and use a shortcut in mg_loadfile().
const char *root_ca =
// Note this cert expires Jun  5 23:59:59 2023 GMT
"-----BEGIN CERTIFICATE-----\r\n"
"MIIFNjCCBNygAwIBAgIQCH4DWYOebCLz/pcJdRheAzAKBggqhkjOPQQDAjBKMQsw\r\n"
"CQYDVQQGEwJVUzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwgSW5jLjEgMB4GA1UEAxMX\r\n"
"Q2xvdWRmbGFyZSBJbmMgRUNDIENBLTMwHhcNMjIwNjA2MDAwMDAwWhcNMjMwNjA1\r\n"
"MjM1OTU5WjB1MQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZvcm5pYTEWMBQG\r\n"
"A1UEBxMNU2FuIEZyYW5jaXNjbzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwgSW5jLjEe\r\n"
"MBwGA1UEAxMVc25pLmNsb3VkZmxhcmVzc2wuY29tMFkwEwYHKoZIzj0CAQYIKoZI\r\n"
"zj0DAQcDQgAEM7bSvJL0gicfcXUnGu2/kqcEhJuwANui9W3rvklDFp7GsNdYPFKM\r\n"
"+Zg+aDwaa7tj3eycVWpSfnNcgITEkMHhTqOCA3cwggNzMB8GA1UdIwQYMBaAFKXO\r\n"
"N+rrsHUOlGeItEX62SQQh5YfMB0GA1UdDgQWBBSSC2QXAXHV15bHHIrgt8vhLgZ6\r\n"
"nTA+BgNVHREENzA1gg4qLnR5cGljb2RlLmNvbYIVc25pLmNsb3VkZmxhcmVzc2wu\r\n"
"Y29tggx0eXBpY29kZS5jb20wDgYDVR0PAQH/BAQDAgeAMB0GA1UdJQQWMBQGCCsG\r\n"
"AQUFBwMBBggrBgEFBQcDAjB7BgNVHR8EdDByMDegNaAzhjFodHRwOi8vY3JsMy5k\r\n"
"aWdpY2VydC5jb20vQ2xvdWRmbGFyZUluY0VDQ0NBLTMuY3JsMDegNaAzhjFodHRw\r\n"
"Oi8vY3JsNC5kaWdpY2VydC5jb20vQ2xvdWRmbGFyZUluY0VDQ0NBLTMuY3JsMD4G\r\n"
"A1UdIAQ3MDUwMwYGZ4EMAQICMCkwJwYIKwYBBQUHAgEWG2h0dHA6Ly93d3cuZGln\r\n"
"aWNlcnQuY29tL0NQUzB2BggrBgEFBQcBAQRqMGgwJAYIKwYBBQUHMAGGGGh0dHA6\r\n"
"Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBABggrBgEFBQcwAoY0aHR0cDovL2NhY2VydHMu\r\n"
"ZGlnaWNlcnQuY29tL0Nsb3VkZmxhcmVJbmNFQ0NDQS0zLmNydDAMBgNVHRMBAf8E\r\n"
"AjAAMIIBfQYKKwYBBAHWeQIEAgSCAW0EggFpAWcAdgDoPtDaPvUGNTLnVyi8iWvJ\r\n"
"A9PL0RFr7Otp4Xd9bQa9bgAAAYE7QeI+AAAEAwBHMEUCIQDQ8ae0L/CsuzGyJEx5\r\n"
"8cd8BsENqmUf+E9hdzaQgsqL+AIgJWmcKKSb4gaDagEqPjLkRQdgAfLqlFqxF8H3\r\n"
"M9Ps83AAdgA1zxkbv7FsV78PrUxtQsu7ticgJlHqP+Eq76gDwzvWTAAAAYE7QeJf\r\n"
"AAAEAwBHMEUCIQDEep0uVRRywlJ9wLVVTgTKdokn2m/D0896bNSh9GPJIwIgbgWR\r\n"
"A87FTZmIbglMls/i3VJntRKmjhI2HTUKSOUaxHIAdQCzc3cH4YRQ+GOG1gWp3BEJ\r\n"
"SnktsWcMC4fc8AMOeTalmgAAAYE7QeKjAAAEAwBGMEQCIDoO8v9DAdGXph7DT13R\r\n"
"nVQFZg0o1866m5ldDSDqqH0LAiA78hyg5U+KGXScZfua5W4K8DPCAHCYaTVNehkA\r\n"
"BRIPEjAKBggqhkjOPQQDAgNIADBFAiAX7XHrh7pjcsRgNzXCSN7MOpfGbf/lbYCP\r\n"
"lztHu9UDCAIhAJJnO2X2c+mt/npSg9jIIJ1PJcUcNu+jftBIgpOXYNLE\r\n"
"-----END CERTIFICATE-----\r\n"
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDzTCCArWgAwIBAgIQCjeHZF5ftIwiTv0b7RQMPDANBgkqhkiG9w0BAQsFADBa\r\n"
"MQswCQYDVQQGEwJJRTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJl\r\n"
"clRydXN0MSIwIAYDVQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTIw\r\n"
"MDEyNzEyNDgwOFoXDTI0MTIzMTIzNTk1OVowSjELMAkGA1UEBhMCVVMxGTAXBgNV\r\n"
"BAoTEENsb3VkZmxhcmUsIEluYy4xIDAeBgNVBAMTF0Nsb3VkZmxhcmUgSW5jIEVD\r\n"
"QyBDQS0zMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEua1NZpkUC0bsH4HRKlAe\r\n"
"nQMVLzQSfS2WuIg4m4Vfj7+7Te9hRsTJc9QkT+DuHM5ss1FxL2ruTAUJd9NyYqSb\r\n"
"16OCAWgwggFkMB0GA1UdDgQWBBSlzjfq67B1DpRniLRF+tkkEIeWHzAfBgNVHSME\r\n"
"GDAWgBTlnVkwgkdYzKz6CFQ2hns6tQRN8DAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0l\r\n"
"BBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1UdEwEB/wQIMAYBAf8CAQAwNAYI\r\n"
"KwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5j\r\n"
"b20wOgYDVR0fBDMwMTAvoC2gK4YpaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL09t\r\n"
"bmlyb290MjAyNS5jcmwwbQYDVR0gBGYwZDA3BglghkgBhv1sAQEwKjAoBggrBgEF\r\n"
"BQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzALBglghkgBhv1sAQIw\r\n"
"CAYGZ4EMAQIBMAgGBmeBDAECAjAIBgZngQwBAgMwDQYJKoZIhvcNAQELBQADggEB\r\n"
"AAUkHd0bsCrrmNaF4zlNXmtXnYJX/OvoMaJXkGUFvhZEOFp3ArnPEELG4ZKk40Un\r\n"
"+ABHLGioVplTVI+tnkDB0A+21w0LOEhsUCxJkAZbZB2LzEgwLt4I4ptJIsCSDBFe\r\n"
"lpKU1fwg3FZs5ZKTv3ocwDfjhUkV+ivhdDkYD7fa86JXWGBPzI6UAPxGezQxPk1H\r\n"
"goE6y/SJXQ7vTQ1unBuCJN0yJV0ReFEQPaA1IwQvZW+cwdFD19Ae8zFnWSfda9J1\r\n"
"CZMRJCQUzym+5iPDuI9yP+kHyCREU3qzuWFloUwOxkgAyXVjBYdwRVKD05WdRerw\r\n"
"6DEdfgkfCv4+3ao8XnTSrLE=\r\n"
"-----END CERTIFICATE-----\r\n";
#endif

void printResponse(MongooseHttpClientResponse *response)
{
  Serial.printf("%d %.*s\n", response->respCode(), response->respStatusMsg().length(), (const char *)response->respStatusMsg());
  int headers = response->headers();
  int i;
  for(i=0; i<headers; i++) {
    Serial.printf("_HEADER[%.*s]: %.*s\n", 
      response->headerNames(i).length(), (const char *)response->headerNames(i), 
      response->headerValues(i).length(), (const char *)response->headerValues(i));
  }

  Serial.printf("\n%.*s\n", response->body().length(), (const char *)response->body());
}

void setup()
{
  Serial.begin(115200);

#ifdef START_ESP_WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname: ");
#ifdef ESP32
  Serial.println(WiFi.getHostname());
#elif defined(ESP8266)
  Serial.println(WiFi.hostname());
#endif
#endif

  Mongoose.begin();

#if ARDUINO_MONGOOSE_SSL_SUPPORTED()
  Mongoose.setRootCa(root_ca);
#endif

  // Based on https://github.com/typicode/jsonplaceholder#how-to
  client.get(PROTO"://jsonplaceholder.typicode.com/posts/1", [](MongooseHttpClientResponse *response) {
    printResponse(response);
  });

  client.post(PROTO"://jsonplaceholder.typicode.com/posts", "application/json; charset=UTF-8",
    "{\"title\":\"foo\",\"body\":\"bar\",\"userId\":1}",
    [](MongooseHttpClientResponse *response)
  {
    printResponse(response);
  });

//  client.put(PROTO"://jsonplaceholder.typicode.com/posts/1", "application/json; charset=UTF-8",
//    "{\"id\":1,\"title\":\"foo\",\"body\":\"bar\",\"userId\":1}",
//    [](MongooseHttpClientResponse *response)
//  {
//    printResponse(response);
//  });

//  client.patch(PROTO"://jsonplaceholder.typicode.com/posts/1", "application/json; charset=UTF-8",
//    "{\"title\":\"foo\"}",
//    [](MongooseHttpClientResponse *response)
//  {
//    printResponse(response);
//  });

//  client.delete(PROTO"://jsonplaceholder.typicode.com/posts/1", [](MongooseHttpClientResponse *response) {
//    printResponse(response);
//  });

//  MongooseHttpClientRequest *request = client.beginRequest(PROTO"://jsonplaceholder.typicode.com/posts");
//  request->setMethod(HTTP_GET);
//  request->addHeader("X-hello", "world");
//  request->onBody([](MongooseHttpClientResponse *response) {
//    Serial.println("onBody() callback:");
//    printResponse(response);
//  });
//  request->onResponse([](MongooseHttpClientResponse *response) {
//    Serial.println("onResponse() callback:");
//    printResponse(response);
//  });
//  client.send(request);
}

void loop()
{
  Mongoose.poll(1000);
}
