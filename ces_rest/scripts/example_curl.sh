## @file 
## Examples curl services drupal.
##
## Dependencias:
## pjson: sudo pip install pjson

username=integraledu
password=eduintegral
endpoint=https://localhost/integralces/cesrest/
cookies_file=/tmp/cookies

test_user="user`date +%s`"
test_pass="test"
test_mail="`date +%s`@test.com"
test_town="les olivex"
test_postcode="12345"
test_firstname="firstname `date +%s`"

example_curl_json() {
  echo -e "$1" | pjson | grep $2 | cut -d: -f2 | cut -d\" -f2
}

echo
echo Login
echo
session="$(curl -k -X POST -H "Content-type: application/json" -c $cookies_file \
${endpoint}user/login \
-d"
{
  \"username\":\"$username\",
  \"password\":\"$password\"
}")"

echo $session | pjson

# Procesamos json
sessid=$(example_curl_json "$session" "sessid")
token=$(example_curl_json "$session" "token")

echo
echo sessid: $sessid
echo token: $token
echo

echo
echo
echo ces_rest
echo
curl -k -H "Content-type: application/json" -b $cookies_file -X GET \
${endpoint}ces_rest | pjson

echo
echo
echo Create a user.
echo

curl -k -H "Content-type: application/json" \
  -H "X-CSRF-Token: $token" \
  -H "Connection: Keep-alive" \
  -b $cookies_file -X POST \
  ${endpoint}user/register \
-d"
{
  \"name\":\"$test_user\",
  \"pass\":\"$test_pass\",
  \"mail\":\"$test_mail\",
  \"status\":\"1\",
  \"language\":\"ca\",
  \"notify\":\"0\",
  \"ces_town\": {
    \"und\": {
      \"0\": {
        \"value\": \"$test_town\"
      }
      }
  },
  \"ces_postcode\": {
    \"und\": {
      \"0\": {
      \"value\": \"$test_postcode\"
      }
    }
  },
  \"ces_firstname\": {
    \"und\": {
      \"0\": {
        \"value\": \"$test_firstname\"
      }
    }
  }
}
"


echo
echo
echo Logout.
echo
curl -k -i -H "Content-Type: application/json" -b cookies_file -X POST ${endpoint}user/logout

[[ -e "$cookies_file" ]] && rm "$cookies_file"

exit
