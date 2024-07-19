<?php

function ces_komunitin_api_auth_reset_password() {
  global $user;

  $email = $_POST['email'];
  $client_id = $_POST['client_id'];

  $user = user_load_by_mail($email);

  if ($user && $user->status == 1) {
    watchdog('ces_komunitin', 'Reset password requested for email %email.', array('%user' => $user->mail), WATCHDOG_NOTICE);
    // Send one-time login link to the user.
    $base = variable_get('ces_komunitin_app_url', 'https://komunitin.org');
    $token = ces_komunitin_auth_one_time_token($client_id, $user);
    if ($token) {
      $url = $base . '/set-password?token=' . $token;
      $lang = user_preferred_language($user);
      $parameters = array(
        'url' => $url,
        'to_name' => ces_user_get_name($user),
      );
      drupal_mail('ces_komunitin', 'reset_password', $user->mail, $lang, $parameters);
    }
  }
  // Anyway, return 204 no content to avoid leaking information.
  drupal_add_http_header('Status', '204 No Content');
  drupal_exit();
}

function ces_komunitin_api_auth_resend_validation_email() {
  global $user;

  $email = $_POST['email'];
  $client_id = $_POST['client_id'];
  $group = $_POST['group'];

  $user = user_load_by_mail($email);
  if ($user && $user->status == 0) {
    // Check user has account in correct exchange.
    $bank = new CesBank();
    $exchange = ces_bank_get_exchange_by_name($group);
    if (!empty($exchange)) {
      $acc = $bank->getUserAccountsInExchange($user->uid, $exchange['id']);
      if (!empty($acc)) {
        ces_komunitin_send_validation_email($user->uid, $group);
      }
    }
  }
  // Anyway, return 204 no content to avoid leaking information.
  drupal_add_http_header('Status', '204 No Content');
  drupal_exit();
}

function ces_komunitin_send_validation_email($uid, $group) {
  $user = user_load($uid); // Reload user to get a fully loaded object.
  $base = variable_get('ces_komunitin_app_url', 'https://komunitin.org');
  $token = ces_komunitin_auth_one_time_token('komunitin-app', $user);
  $lang = user_preferred_language($user);
  drupal_mail('ces_komunitin', 'signup_validate_email', $user->mail, $lang, [
    'url' => $base . '/groups/' . $group . '/signup-member?token=' . $token,
    'to_name' => ces_user_get_name($user),
  ]);
}

function ces_komunitin_auth_one_time_token($client_id, $user, $scope = null) {
  // Use oauth2_server module to create an authorization code.
  $client = oauth2_server_client_load($client_id);
  if ($client) {
    // Initialize the server.
    $server = oauth2_server_load($client->server);
    $server = oauth2_server_start($server);
    // Expire the code in 24 hours.
    $server->setConfig('auth_code_lifetime', 24*3600);

    $controller = $server->getAuthorizeController();
    // Build code.
    $responseType = $server->getResponseType('code');
    $code = $responseType->createAuthorizationCode($client_id, $user->uid, '', $scope);
    return $code;
  }
  return false;
}

function bas64url_encode($data) {
  return rtrim(strtr(base64_encode($data), '+/', '-_'), '=');
}

function ces_komunitin_api_auth_jwks() {
  drupal_page_is_cacheable(FALSE);
  // Get the public key used by oauth2 module.
  $keys = oauth2_server_get_keys();

  // Obtain the public key modulus and exponent.
  $public_key = openssl_pkey_get_public($keys['public_key']);
  $details = openssl_pkey_get_details($public_key);
  $modulus = bas64url_encode($details['rsa']['n']);
  $exponent = bas64url_encode($details['rsa']['e']);

  // Create an arbitrary key id.
  $kid = substr(hash('sha256', $modulus), 0, 32);

  // Build the JWKS response.
  $jwks = array(
    'keys' => array(
      array(
        'kty' => 'RSA',
        'alg' => 'RS256',
        'use' => 'sig',
        'kid' => $kid,
        'n' => $modulus,
        'e' => $exponent,
      ),
    ),
  );
  // Output.
  drupal_json_output($jwks);
}
