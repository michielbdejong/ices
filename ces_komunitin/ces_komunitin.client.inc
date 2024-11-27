<?php
require_once __DIR__ . '/ces_komunitin.api.inc';

function ces_komunitin_client_get_admin_token($exchange) {
  return ces_komunitin_client_get_user_token($exchange['admin']);
}

/**
 * Get user token for Komunitin accounting API.
 * @param $uid Drupal's user id.
 */
function ces_komunitin_client_get_user_token($uid) {
  module_load_include('module', 'oauth2_server');
  $server = oauth2_server_load('komunitin');
  $server = oauth2_server_start($server);
  $server->getAuthorizeController(); //initializes response types in server.
  $jwt = $server->getResponseType('token');
  $oauth = $jwt->createAccessToken('komunitin-app', $uid, 'komunitin_accounting', false);
  $token = $oauth['access_token'];

  return $token;
}

function ces_komunitin_get_accounting_api_url() {
  $base_url = variable_get('ces_komunitin_accounting_url_internal', 'https://accounting.komunitin.org');
  return $base_url;
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
  $url = $options['url'];
  $body = json_encode($options['body']);
  $response = drupal_http_request($url, [
    'method' => $options['method'],
    'headers' => [
      'Content-Type' => 'application/vnd.api+json',
      'Authorization' => 'Bearer ' . $options['token']
    ],
    'data' => $body,
    'timeout' => 60
  ]);

  if ($response->code >= 400 || $response->code < 200) {
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

  $base_url = ces_komunitin_get_accounting_api_url();

  $url = $base_url . "/$code/accounts";

  $response = ces_komunitin_client_fetch([
    'method' => 'POST',
    'url' => $url,
    'body' => $data,
    'token' => $token
  ]);

  if ($response->code == 201) {
    $result = json_decode($response->data);
    watchdog('ces_komunitin', 'Account created in Komunitin: @account', ['@account' => $result->data->id], WATCHDOG_NOTICE);
  } else {
    watchdog('ces_komunitin', 'Error creating account in Komunitin: @response', ['@response' => isset($response->data) ? $response->data : $response->code], WATCHDOG_ERROR);
    throw new Exception('Error creating account in Komunitin');
  }
}

function ces_komunitin_client_delete_account($record) {
  $exchange = ces_bank_get_exchange($record['exchange']);

  $code = $exchange['code'];
  $token = ces_komunitin_client_get_admin_token($exchange);

  $base_url = ces_komunitin_get_accounting_api_url();

  $url = $base_url . "/$code/accounts/" . $record['id'];

  $response = ces_komunitin_client_fetch([
    'method' => 'DELETE',
    'url' => $url,
    'token' => $token
  ]);

  if ($response->code == 204) {
    watchdog('ces_komunitin', 'Account deleted in Komunitin: @account', ['@account' => $record['id']], WATCHDOG_NOTICE);
  } else {
    watchdog('ces_komunitin', 'Error deleting account in Komunitin: @response', ['@response' => $response->data], WATCHDOG_ERROR);
  }

}

/**
 * Create currency in Komunitin accounting API.
 * @param $record exchange record
 * @return string currency id
 */
function ces_komunitin_client_create_currency($record) {
  $currency = new Currency($record);
  $attributes = [
    'code' => $currency->code,
    'name' => $currency->name,
    'namePlural' => $currency->namePlural,
    'symbol' => $currency->symbol,
    'decimals' => $currency->decimals,
    'scale' => $currency->scale,
    'rate' => [
      'n' => $currency->rate_n,
      'd' => $currency->rate_d
    ]
  ];

  $adminId = ces_komunitin_api_social_get_uuid(ResourceTypes::USER, $record['admin']);

  $settings = [
    'id' => "1", // Ephemeral id.
    'type' => 'currency-settings',
    'attributes' => [
      'defaultInitialCreditLimit' => 0
    ]
  ];

  $relationships = [
    'admins' => [
      'data' => [
        ['type' => 'users', 'id' => $adminId]
      ]
    ],
    'settings' => [
      'data' => ['type' => $settings['type'], 'id' => $settings['id']]
    ]
  ];

  $included = [$settings];

  $data = ces_komunitin_client_jsonapi_doc('currencies', $currency->id, $attributes, $relationships, $included);

  $token = ces_komunitin_client_get_admin_token($record);
  $base_url = ces_komunitin_get_accounting_api_url();

  $url = $base_url . "/currencies";

  $response = ces_komunitin_client_fetch([
    'method' => 'POST',
    'url' => $url,
    'body' => $data,
    'token' => $token
  ]);

  if ($response->code == 201) {
    watchdog('ces_komunitin', 'Currency created in Komunitin: @currency', ['@currency' => $currency->id], WATCHDOG_NOTICE);
    $data = json_decode($response->data);
    return $data->data->id;
  } else {
    watchdog('ces_komunitin', 'Error creating currency in Komunitin: @response', ['@response' => isset($response->data) ? $response->data : $response->code], WATCHDOG_ERROR);
    throw new Exception('Error creating currency in Komunitin');
  }
}
