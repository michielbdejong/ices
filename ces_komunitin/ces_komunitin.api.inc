<?php
require_once dirname(__FILE__) . '/includes/JsonApiRequest.php';
require_once dirname(__FILE__) . '/includes/KomunitinApiError.php';
require_once dirname(__FILE__) . '/includes/Transaction.php';

use Neomerx\JsonApi\Encoder\Encoder;

function _ces_komunitin_api_get_base_path() {
  return 'ces/api';
}
function _ces_komunitin_api_get_base_url() {
  global $base_url;
  return $base_url . '/' . _ces_komunitin_api_get_base_path();
}
/**
 * Handle Komunitin API call.
 */
function ces_komunitin_api() {
  global $user;
  try {
    $request = new JsonApiRequest(_ces_komunitin_api_get_base_path());

    // Api namespace (social, accounting).
    $path = explode('/', $request->path);
    $api = $path[0];
    _ces_komunitin_api_verify_api($api);

    // Authorization.
    $token = oauth2_server_check_access('komunitin', 'komunitin_' . $api);
    if ($token instanceof OAuth2\Response) {
      $error_code = $token->getStatusCode() == 403 ?
        KomunitinApiError::FORBIDDEN : KomunitinApiError::UNAUTHORIZED;
      ces_komunitin_api_send_error($error_code);
    }
    else {
      // Login user identified by Authorization header token.
      $user = user_load($token['user_id']);
    }

    // Exchange
    $exchange_code = $path[1];
    $exchange = ces_bank_get_exchange_by_name($exchange_code);
    if (!$exchange) {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
    }

    // Endpoint
    $type = $path[2] ?? 'currencies';
    $endpoints = ces_komunitin_api_get_endpoints();

    if (!isset($endpoints[$api][$type])) {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
    }
    $endpoint = $endpoints[$api][$type];

    // Method
    $methods = array(
      'GET' => 'read',
      'POST' => 'create',
      'PATCH' => 'update',
      'DELETE' => 'delete'
    );

    if (isset($path[3])) {
      $endpoint = $endpoint['%'];
      $resource_id = $path[3];
    }
    else {
      $resource_id = null;
    }
    if (!isset($methods[$request->method]) || array_search($methods[$request->method], $endpoint['operations']) === FALSE) {
      $operations = $endpoint['operations'];
      array_keys(array_filter($methods, function($operation) use ($operations) {
        return array_search($operation, $operations);
      }));
      drupal_add_http_header('Allow', join(', ', $operations));
      ces_komunitin_api_send_error(KomunitinApiError::METHOD_NOT_ALLOWED);
    }
    $operation = $methods[$request->method];

    // Prepare parameters.
    $parameters = [$exchange];

    // Load resource if it exists.
    if ($resource_id) {
      $record = ces_komunitin_api_call_function('load', $api, $type, [$exchange, $resource_id]);
      $parameters[] = $record;
    }

    // Read request body if needed.
    if ($operation == 'create' || $operation == 'update') {
      if (!$request->body) {
        ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
      }
      $body = json_decode($request->body);
      if (json_last_error() !== JSON_ERROR_NONE) {
        ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
      }
      $parameters[] = $body;
    }

    // Call operation.
    $result = ces_komunitin_api_call_function($operation, $api, $type, $parameters);


    if ($result === TRUE) {
      // Success but no content.
      $status = 204;
      $content = '';
    }
    else {
      $content = Encoder::instance(array(
        Transaction::class => TransactionSchema::class
      ))
        ->withUrlPrefix(_ces_komunitin_api_get_base_url() . '/' . $api . '/' . $exchange_code)
        ->withEncodeOptions(JSON_PRETTY_PRINT)
        ->encodeData($result);
      $status = $operation == 'create' ? 201 : 200;
    }
    // Send response
    ces_komunitin_api_send_response($status, $content);
  } catch (Exception $e) {
    ces_komunitin_api_send_error(KomunitinApiError::INTERNAL_SERVER_ERROR);
  }
}

function ces_komunitin_api_call_function($operation, $api, $type, $params = []) {
  require_once dirname(__FILE__) . '/ces_komunitin.api.' . $api . '.inc';
  $name = 'ces_komunitin_api_' . $api . '_' . $type  . '_' . $operation;
  return call_user_func_array($name, $params);
}
/**
 * Return implemented endpoints.
 */
function ces_komunitin_api_get_endpoints() {
  return array(
    'accounting' => array(
      'transactions' => array(
        'operations' => array('create'),
        '%' => array(
          'operations' => array('read', 'delete')
        )
      )
    )
  );
}

function _ces_komunitin_api_verify_api($api) {
  if ($api != 'accounting') {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED);
  }
}

/**
 * Send a JSON API error object.
 *
 * @param $code An error code from KomunitinApiError.
 */
function ces_komunitin_api_send_error($code, $details = null) {
  $kerror = new KomunitinApiError($code, $details);
  $error = $kerror->getJsonApiError();
  $response = Encoder::instance()->encodeError($error);
  ces_komunitin_api_send_response($kerror->getStatusCode(), $response);
}

/**
 * @param $status the http status code number.
 * @param $content the JSON API content body as a string.
 */
function ces_komunitin_api_send_response($status, $content) {
  $status_text = _ces_komunitin_api_http_status_text($status);
  drupal_add_http_header('Status', $status_text);
  if ($content) {
    drupal_add_http_header('Content-Type', 'application/vnd.api+json');
    drupal_add_http_header('Content-Length', strlen($content));
    echo $content;
  }
  drupal_exit();
}

/**
 * 404 => '404 Not Found'
 */
function _ces_komunitin_api_http_status_text($code) {
  // Take the array of HTTP status codes from OAuth2 module
  $text = OAuth2\Response::$statusTexts[$code];
  return $code . ' ' . $text;
}


/**
 * Check if a given string is a valid UUID
 *
 * From https://gist.github.com/joel-james/3a6201861f12a7acf4f2
 *
 * @param   string  $uuid The string to check
 * @return  boolean
 */
function ces_komunitin_api_check_valid_uuid($uuid) {
  return (is_string($uuid) &&
    (preg_match('/^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/', $uuid)));
}
