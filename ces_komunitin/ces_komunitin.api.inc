<?php
require_once dirname(__FILE__) . '/ces_komunitin.api.accounting.inc';
require_once dirname(__FILE__) . '/ces_komunitin.api.social.inc';

require_once dirname(__FILE__) . '/includes/JsonApiRequest.php';
require_once dirname(__FILE__) . '/includes/KomunitinApiError.php';
require_once dirname(__FILE__) . '/includes/CustomFactory.php';
// Accounting API objects
require_once dirname(__FILE__) . '/includes/Currency.php';
require_once dirname(__FILE__) . '/includes/Account.php';
require_once dirname(__FILE__) . '/includes/AccountSettings.php';
require_once dirname(__FILE__) . '/includes/Transfer.php';

// Social API objects
require_once dirname(__FILE__) . '/includes/Group.php';
require_once dirname(__FILE__) . '/includes/SignupSettings.php';
require_once dirname(__FILE__) . '/includes/Contact.php';
require_once dirname(__FILE__) . '/includes/Member.php';
require_once dirname(__FILE__) . '/includes/Offer.php';
require_once dirname(__FILE__) . '/includes/Need.php';
require_once dirname(__FILE__) . '/includes/Category.php';
require_once dirname(__FILE__) . '/includes/User.php';
require_once dirname(__FILE__) . '/includes/UserSettings.php';
require_once dirname(__FILE__) . '/includes/ExternalResources.php';

//Notification API objects
require_once dirname(__FILE__) . '/includes/Event.php';

use Neomerx\JsonApi\Contracts\Schema\LinkInterface;
use Neomerx\JsonApi\Encoder\Encoder;
use Neomerx\JsonApi\Schema\Link;

function _ces_komunitin_api_get_base_path() {
  return 'ces/api';
}

function ces_komunitin_api_get_social_api_url() {
  global $base_url;
  return $base_url . '/' . _ces_komunitin_api_get_base_path() . '/social';
}

function ces_komunitin_api_get_accounting_api_url($exchange = FALSE) {
  global $base_url;
  if ($exchange && !empty($exchange['data']['komunitin_accounting_api_url'])) {
    return $exchange['data']['komunitin_accounting_api_url'];
  } else {
    return $base_url . '/' . _ces_komunitin_api_get_base_path() . '/accounting';
  }
}

/**
 * Handle Komunitin API call.
 */
function ces_komunitin_api() {
  global $user;
  try {
    $request = new JsonApiRequest(_ces_komunitin_api_get_base_path());
    $path = explode('/', $request->path);
    $api = reset($path);

    // Log in with the token.
    $token = ces_komunitin_api_check_auth_token();
    // Broad authorization check. Individual operations may perform additional
    // checks. It is required to provide either:
    //  - user credentials with komunitin_social or komunitin_accounting scope.
    //  - client credentials with komunitin_social_read_all or ç
    //    komunitin_accounting_read_all scope for read operations.
    //  - nothing, for read operations on public endpoints.

    // Unauthenticated request
    if ($token instanceof OAuth2\Response) {
      // Check if it is to a public endpoint and quick return otherwise.
      if (!ces_komunitin_api_is_public_endpoint($path, $request->method)) {
        $error_code = $token->getStatusCode() == 403 ?
          KomunitinApiError::FORBIDDEN : KomunitinApiError::UNAUTHORIZED;
        ces_komunitin_api_send_error($error_code);
      }
    // Authenticated request carrying user credentials.
    } else if (!empty($token['user_id']) && ces_komunitin_api_client_access('komunitin_' . $api)) {
        // Login user identified by Authorization header token.
        $user = user_load($token['user_id']);
        // Flag that this user is using the Komunitin api (then it will receive
        // komunitin emails, etc from now on).
        ces_komunitin_api_user_logged_in($user);
    // Authenticated request carrying client credentials for GET request.
    } else if (
      empty($token['user_id']) &&
      ces_komunitin_api_client_access('komunitin_' . $api . '_read_all') &&
      $request->method == 'GET'
    ) {
      $user = user_load(0);
    }
    // Insuficcient scope.
    else {
      ces_komunitin_api_send_error(KomunitinApiError::UNAUTHORIZED);
    }

    // Parse request.
    $endpoints = ces_komunitin_api_get_endpoints();
    // Api namespace (social, accounting).
    if (!isset($endpoints[$api])) {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
    }
    else {
      $endpoints = $endpoints[$api];
    }
    $code = next($path);
    // TODO: better $endpoints structure would simplify that.
    if ($api == 'social' && ($code == 'groups' || $code == 'users')) {
      // This is an exception: usually this path element should be the group or
      // currency code.
      $endpoint = $endpoints[$code];
      $type = $code;
      $code = FALSE;
      $exchange = FALSE;
    }
    else {
      // load exchange.
      $exchange = ces_bank_get_exchange_by_name($code);
      if (!$exchange) {
        ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
      }
      // Check if endpoint is defined.
      $type = next($path);
      // Single group endpoint doesn't have type
      if ($type === FALSE) {
        $type = 'groups';
      }
      if (!isset($endpoints[$type])) {
        ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
      }
      $endpoint = $endpoints[$type];
    }

    // Method
    $methods = array(
      'GET' => 'read',
      'POST' => 'create',
      'PATCH' => 'update',
      'DELETE' => 'delete'
    );

    $resource_id = next($path);
    $attribute = FALSE;
    if ($resource_id !== FALSE) {
      // This case is just for /users/me
      if (isset($endpoint[$resource_id])) {
        $endpoint = $endpoint[$resource_id];
      }
      // This is the general case.
      else if (isset($endpoint['%'])) {
        $endpoint = $endpoint['%'];
      }
      else {
        ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
      }
      // Check if attribute is defined.
      $attribute = next($path);
      if ($attribute !== FALSE) {
        if (isset($endpoint[$attribute])) {
          $endpoint = $endpoint[$attribute];
        }
        else {
          ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
        }
      }
    }
    // Check if method (GET, POST, PATCH, DELETE) is allowed on this endpoint.
    if (!isset($methods[$request->method]) || array_search($methods[$request->method], $endpoint['operations']) === FALSE) {
      $operations = $endpoint['operations'];
      array_keys(array_filter($methods, function($operation) use ($operations) {
        return array_search($operation, $operations);
      }));
      drupal_add_http_header('Allow', join(', ', $operations));
      ces_komunitin_api_send_error(KomunitinApiError::METHOD_NOT_ALLOWED);
    }
    $operation = $methods[$request->method];

    // Differentiate getting single resource from getting multiple resources.
    // Note that groups have resource_id = FALSE but are not a collection when
    // the exchange is set.
    $collection = $operation == 'read'
      && ($type != 'groups' && $resource_id === FALSE
      || $type == 'groups' && $exchange === FALSE)
      && isset($endpoint['%']);

    if ($collection) {
      $operation = 'read_collection';
    }

    // Prepare parameters.
    $parameters = [];
    if ($exchange) {
      $parameters[] = $exchange;
    }

    // Load resource if it exists.
    if ($resource_id) {
      $record = ces_komunitin_api_call_function('load', $api, $type, [$exchange, $resource_id]);
      $parameters[] = $record;
    }

    // Load collection of resources.
    if ($collection) {
      // Pagination. We use the https://jsonapi.org/profiles/ethanresnick/cursor-pagination/ spec for pagination.
      // However, we actually use the offset as a cursor. For perfornamce reasons we may change
      // the cursor definition to something that allows us random access since the client uses it
      // as an opaque string.

      $pageSize = $request->pageSize;
      $pageAfter = $request->pageAfter ?? 0;

      $records = ces_komunitin_api_call_function('load_collection', $api, $type, [$exchange, $request->filters, $request->sorts, $pageAfter, $pageSize]);
      $parameters[] = $records;
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
    $result = ces_komunitin_api_call_function($operation, $api, $type, $parameters, $attribute);


    if ($result === TRUE) {
      // Success but no content.
      $status = 204;
      $content = '';
    }
    else {
      $includes = [];
      foreach ($request->includes as $path => $include) {
        $includes[] = $path;
      }

      $urlprefix = $api == 'accounting'
        ? ces_komunitin_api_get_accounting_api_url($exchange)
        : ces_komunitin_api_get_social_api_url();

      $encoder = CustomFactory::newEncoder(array(
        Currency::class => CurrencySchema::class,
        Account::class => AccountSchema::class,
        AccountSettings::class => AccountSettingsSchema::class,
        Transfer::class => TransferSchema::class,
        Group::class => GroupSchema::class,
        SignupSettings::class => SignupSettingsSchema::class,
        Contact::class => ContactSchema::class,
        Member::class => MemberSchema::class,
        MinimalMember::class => MinimalMemberSchema::class,
        Offer::class => OfferSchema::class,
        Need::class => NeedSchema::class,
        Category::class => CategorySchema::class,
        User::class => UserSchema::class,
        UserSettings::class => UserSettingsSchema::class,
        ExternalAccount::class => ExternalAccountSchema::class,
        ExternalCurrency::class => ExternalCurrencySchema::class,
      ))
        ->withUrlPrefix($urlprefix)
        ->withIncludedPaths($includes)
        ->withEncodeOptions(JSON_PRETTY_PRINT);

      if ($collection) {
        $next = (count($result) == $request->pageSize) ? new Link(false, $request->getNextUrl(), false) : null;
        $encoder->withLinks([
          LinkInterface::NEXT => $next
        ]);
      }

      $content = $encoder->encodeData($result);
      $status = $operation == 'create' ? 201 : 200;
    }
    // Send response
    ces_komunitin_api_send_response($status, $content);
  } catch (Exception $e) {
    ces_komunitin_api_send_error(KomunitinApiError::INTERNAL_SERVER_ERROR, $e->getMessage());
  }
}

/**
 * Call a function in the API, for example:
 *   - ces_komunitin_api_accounting_currency_read
 *   - ces_komunitin_api_accounting_accounts_load
 *   - ces_komunitin_api_accounting_accounts_settings_load
 *
 * The load and load_collection operations are only implemented in main types
 * (without attribute param).
 */
function ces_komunitin_api_call_function($operation, $api, $type, $params = [], $attribute = null) {
  $name = 'ces_komunitin_api_' . $api . '_' . $type . ($attribute ? '_' . $attribute :  '')  . '_' . $operation;
  return call_user_func_array($name, $params);
}
/**
 * Return all implemented endpoints.
 */
function ces_komunitin_api_get_endpoints() {
  $endpoints = array(
    'accounting' => array(
      'currency' => array(
        'operations' => array('read')
      ),
      'accounts' => array(
        'operations' => array('read'),
        '%' => array(
          'settings' => array(
            'operations' => array('read', 'update')
          ),
          'operations' => array('read')
        )
      ),
      'transfers' => array(
        'operations' => array('read', 'create'),
        '%' => array(
          'operations' => array('read', 'update', 'delete')
        )
      )
    ),
    'social' => array(
      'groups' => array(
        'operations' => array('read'),
        '%' => array(
          'operations' => array('read')
        )
      ),
      'users' => array(
        'operations' => array('read', 'create'),
        'me' => array(
          'operations' => array('read', 'update'),
          'settings' => array(
            'operations' => array('read', 'update')
          )
        ),
        '%' => array(
          'operations' => array('read'),
          'settings' => array(
            'operations' => array('read')
          )
        )
      ),
      'members' => array(
        'operations' => array('read'),
        '%' => array(
          'operations' => array('read', 'update')
        )
      ),
      'needs' => array(
        'operations' => array('read', 'create'),
        '%' => array(
          'operations' => array('read', 'update', 'delete')
        )
      ),
      'offers' => array(
        'operations' => array('read', 'create'),
        '%' => array(
          'operations' => array('read', 'update', 'delete')
        )
      )
    )
  );
  return $endpoints;
}
function ces_komunitin_get_public_paths() {
  return [
    'accounting/%/currency' => ['GET'],
    'social/groups' => ['GET'],
    'social/%' => ['GET'],
    'social/users' => ['POST'],
    'social/members' => ['GET'], // only public for certain groups
  ];
}
/**
 * Check if path is one of the available endpoints for unauthorized users.
 * Public endpoints are:
 *
 */
function ces_komunitin_api_is_public_endpoint($path, $method) {
  $public = ces_komunitin_get_public_paths();
  foreach ($public as $candidate => $methods) {
    if (in_array($method, $methods) && ces_komunitin_api_path_matches($path, $candidate)) {
      return TRUE;
    }
  }
  return FALSE;
}

function ces_komunitin_api_path_matches($path, $candidate) {
  $match = FALSE;
  $items = explode('/', $candidate);
  if (count($path) == count($items)) {
    $match = TRUE;
    foreach ($items as $i => $item) {
      if ($item != $path[$i] && $item != '%') {
        $match = FALSE;
      }
    }
  }
  return $match;
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
/**
 * Check if the given scope is granted for the current request.
 * @param string $scope space separated list of OAuth2 scopes.
 * @return boolean
 */
function ces_komunitin_api_client_access($scope) {
  $token = ces_komunitin_api_check_auth_token($scope);
  return !($token instanceof OAuth2\Response);
}

function ces_komunitin_api_check_auth_token($scope = NULL) {
  // The funciton oauth2_server_check_access() from oauth2_server module does
  // not work well when ustin JWT tokens and in fact it always rejects
  // authentication. See https://www.drupal.org/project/oauth2_server/issues/2860106
  // $token = oauth2_server_check_access('komunitin', $scope);

  // Thats why we reimplement the check here.

  $server = oauth2_server_load('komunitin');
  $oauth2_server = oauth2_server_start($server);

  $response = new OAuth2\Response();
  $token = $oauth2_server->getAccessTokenData(OAuth2\Request::createFromGlobals(), $response);
  // If there's no token, that means validation failed. Stop here.
  if (!$token) {
    return $response;
  }

  // Check scope, if provided
  // If token doesn't have a scope, it's null/empty, or it's insufficient, throw an error.
  $scope_util = new Drupal\oauth2_server\Scope($server);
  if ($scope && (!isset($token["scope"]) || !$token["scope"] || !$scope_util->checkScope($scope, $token["scope"]))) {
    $response->setError(401, 'insufficient_scope', 'The request requires higher privileges than provided by the access token');
    $response->addHttpHeaders(array('WWW-Authenticate' => sprintf('%s, realm="%s", scope="%s"', 'bearer', 'Service', $scope)));
    return $response;
  }

  return $token;
}

/**
 * Save that the user is using the Komunitin API.
 */
function ces_komunitin_api_user_logged_in(&$user) {
  $user_settings = ces_user_get_settings($user);
  if (empty($user_settings['komunitin'])) {
    $user_settings['komunitin'] = TRUE;
    ces_user_set_settings($user, ['komunitin' => TRUE]);
  }
}
