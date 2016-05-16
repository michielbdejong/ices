#!/bin/bash

## @file test_rest.sh
## @brief Test rest service.
##
## Documentation:
##
## - http://curl.haxx.se/docs/httpscripting.html
## - https://www.drupal.org/node/1795770
## - http://docs.oracle.com/cd/E40329_01/dev.1112/e27134/restapioauthl.htm#AIDEV6760
##
## Example:
##
## @code
## Pass info of user with uid 3:
## oauth2_test.sh -d -at 8c5105afc7928ebb34eb48505c78a8f4f8a60957 -a users/NET2/3
## @endcode
## Dependencies.
##
## pjson: sudo pip install pjson

## Configurations.

# Base URL of your web site.
site_url="https://localhost/integralces"

# Path to temporary file which will store your cookie data.
cookie_path=/tmp/cookie

# Drupal authentication credentials.
username="Fermat"
password="fermat"

# Oauth2.
client_id="test_client"
client_secret="test_secret"
scope="cesrest"
scope_text="Access your account data"
endpoint_auth="${site_url}/oauth2/authorize"
endpoint_token="${site_url}/oauth2/token"
redirect_uri=http://localhost/headers.php
response_type=code
state=xyz
clientUri=
action=
# Endpoint. URL of your custom service.
service_url=$site_url/cesrest

# Con estos datos y la extensión RESTClient de firefox  podemos optener un 
# token que podemos pasar al script para testear.
authorization_access_token=
authorization_refresh_token=


# Actions.
declare -i actions=(ces_rest ces_users user)

# Others.
debug=FALSE
login=FALSE  ## TRUE / FALSE / LOGOUT
oauth2=TRUE

## Help.
function test_rest_help(){
echo "
Uso: test_rest.sh 

Options:

-d                     Debuger.
-a [action] [params]   Name scope and params.
-l [option]            TRUE/FALSE/LOGOUT Default FALSE.
-o                     Using oauth2.
-at                    Authentication oauth2 token.
-ar                    Authentication oauth2 refresh.
-h                     This help.

"
}

## Execute login with oauth2.
function test_rest_login_oauth2() {
  curl -s --cookie-jar "$cookie_path" \
  -d "name=$username&pass=$password&form_id=user_login" \
  $site_url/user/login
}

## Execute action with oauth2.
function test_rest_curl_oauth2() {
  local action="$1"
  local data="${2:-FALSE}"

  echo
  echo $*
  echo

  [[ "$debug" == "FLASE" ]] && echo $*

  [[ "$data" != "FALSE" ]]   && cmd="$cmd -d '$data'"

  curl -k -v \
    -H "Content-type: application/json" \
    -H "Authorization: Bearer $authorization_access_token" \
    --cookie $cookie_path \
    --request GET $action | pjson

}

# Parameters.
while [ -n "$1" ] ; do
   case "$1" in
      -d)  debug=TRUE ; shift 1 ;;
      -a)  action=$2 ; shift 2 ;;
      -l)  login=$2; shift 2 ;;
      -o)  oauth2=TRUE; shift 1 ;;
      -at) authorization_access_token=$2; shift 2 ;;
      -ar) authorization_refresh_token=$2; shift 2 ;;
      -h)  test_rest_help; exit ;;
      *) break ;;
   esac
done

test_rest_curl_oauth2 $service_url/${action}/$1 
exit

[[ "$debug" == "TRUE" ]] && echo -e "\ncookie file: $cookie_path\n"

# Si no tenemos action las disparamos todas.
if [ -z $action ] ; then
  for a in ${actions[*]} ; do
    [[ "$debug" == "TRUE" ]] && echo -e "\n$a\n"
    test_rest_curl_oauth2 $service_url/$a
  done
else
  test_rest_curl_oauth2 $service_url/${action}/$1 
fi

if [ "$debug" == "TRUE" ] ; then
  test_rest_curl_oauth2 $site_url/ces/bank/account/message | lynx -dump -nolist -stdin
fi
  
exit

test_user="user`date +%s`"
test_pass="test"
test_mail="`date +%s`@test.com"
test_town="les olivex"
test_postcode="12345"
test_firstname="firstname `date +%s`"


echo
echo
echo Create a user.
echo

curl -k -v \
  -H "Content-type: application/json" \
  -H "Authorization: Bearer $authorization_access_token" \
  --cookie $cookie_path \
  -H "Connection: Keep-alive" \
  -X POST \
  ${service_url}/user/register \
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
exit
