<?php

function ces_komunitin_client_get_admin_token($exchange) {
  module_load_include('module', 'oauth2_server');
  $server = oauth2_server_load('komunitin');
  $server = oauth2_server_start($server);
  $server->getAuthorizeController(); //initializes response types in server.
  $jwt = $server->getResponseType('token');
  $oauth = $jwt->createAccessToken('komunitin-app', $exchange['admin'], 'komunitin_accounting', false);
  $token = $oauth['access_token'];

  return $token;
}

// Fix urls so it works when using docker in local development.
function ces_komunitin_client_fix_url($url) {
  $url = str_replace('localhost', 'host.docker.internal', $url);
  return $url;
}

/**
 * Fetch data from Komunitin accounting API.
 * $options = [
 *  'method' => 'GET',
 *  'url' => 'https://host.org/path/to/resource',
 *  'body' => [...],
 *  'token' => "..."
 * ]
 */
function ces_komunitin_client_fetch($options) {
  $url = ces_komunitin_client_fix_url($options['url']);
  $body = json_encode($options['body']);
  $response = drupal_http_request($url, [
    'method' => $options['method'],
    'headers' => [
      'Content-Type' => 'application/vnd.api+json',
      'Authorization' => 'Bearer ' . $options['token']
    ],
    'data' => $body
  ]);

  if ($response->code >= 400) {
    watchdog('ces_komunitin', 'Error fetching data from Komunitin API: @error', ['@error' => $response->error], WATCHDOG_ERROR);
  }

  return $response;
}

function ces_komunitin_client_jsonapi_doc($type, $id, $attributes, $relationships, $included = NULL) {
  $data = [
    'data' => [
      'type' => $type,
      'id' => $id,
      'attributes' => $attributes,
      'relationships' => $relationships
    ]
  ];
  if ($included) {
    $data['included'] = $included;
  }
  return $data;
}

/**
 * Create account in Komunitin accounting API.
 */
function ces_komunitin_client_create_account($record) {
  require_once __DIR__ . '/ces_komunitin.api.inc';

  $exchange = ces_bank_get_exchange($record['exchange']);
  $account = new Account($record, new Currency($exchange));
  $attributes = ['code' => $account->code];
  if ($account->creditLimit != -1) {
    $attributes['creditLimit'] = $account->creditLimit;
  }
  if ($account->maximumBalance != -1) {
    $attributes['maximumBalance'] = $account->maximumBalance;
  }
  $uid = _ces_komunitin_api_social_account_user_id($record['id']);
  $uuid = ces_komunitin_api_social_get_uuid(ResourceTypes::USER, $uid);
  $users = [
    ['type' => 'users', 'id' => $uuid]
  ];
  $relationships = [
    'users' => [
      'data' => $users
    ]
  ];

  $data = ces_komunitin_client_jsonapi_doc('accounts', $account->id, $attributes, $relationships, $users);
  $code = $exchange['code'];
  $token = ces_komunitin_client_get_admin_token($exchange);

  $url = $exchange['data']['komunitin_accounting_api_url'] . "/$code/accounts";

  $response = ces_komunitin_client_fetch([
    'method' => 'POST',
    'url' => $url,
    'body' => $data,
    'token' => $token
  ]);

  if ($response->code == 201) {
    $result = json_decode($response->data);
    watchdog('ces_komunitin', 'Account created in Komunitin: @account', ['@account' => $result->data->id], WATCHDOG_NOTICE);
  }
}
