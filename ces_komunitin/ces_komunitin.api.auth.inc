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
    $token = ces_komunitin_auth_one_time_token($client_id, $user, $base);
    if ($token) {
      $url = $base . '/set-password?token=' . $token;
      $lang = user_preferred_language($user);
      $parameters = array(
        'url' => $url,
        'to_name' => ces_user_get_name($user),
      );
      drupal_mail('ces_komunitin', 'reset_password', $user->mail, $lang, array('url' => $url, ));
    }
  }
  // Anyway, return 204 no content to avoid leaking information.
  drupal_add_http_header('Status', '204 No Content');
  drupal_exit();
}

function ces_komunitin_auth_one_time_token($client_id, $user, $redirect_uri) {
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
    $code = $responseType->createAuthorizationCode($client_id, $user->uid, '');
    return $code;
  }
  return false;
}
