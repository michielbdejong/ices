<?php
/**
 * @file
 * Implement Ces Rest functionality.
 */

/**
 * @defgroup ces_rest_funcionality Ces Rest Funcionality
 * @ingroup ces_rest
 * @{
 */

/**
 * Index ces rest.
 */
function _ces_rest_index($code_exchange, $uid, $parameters) {
  
  global $user;

  $output = FALSE;

  // @todo Validate code_exchange.

  // @todo Validate uid.

  // If uid print info user.
  if ( $uid ) {
    // Comprobar que el usuario pertenece a la red.
    $user_load = user_load($uid);
    $bank = new CesBank();
    $output[] = array(
      'user'     => $user_load,
      'accounts' => $bank->getUserAccounts($uid)
    );
    return $output;
  }


  // If not uid print list users.
  $bank = new CesBank();
  $exchange = ces_bank_get_current_exchange();
  $accounts = $bank->getAllAccounts(array('state' => 0, 'exchange' => $exchange['id']));

  foreach ($accounts as $account) {
    foreach ( $account['users'] as $user ) {
      $output[$user['user']] = array(
        'user'     => user_load($user['user']),
        'accounts' => $bank->getUserAccounts($user['user'])
      );
    }
  }

  return $output;
}

/**
 * Add user.
 *
 * @todo Validar datos.
 */

function _ces_rest_add_user($parameters) {

  // Validate data.

  // The user fields.
  $fields = array(
    'name' => $parameters['name'],
    'mail' => $parameters['mail'],
    'pass' => $parameters['pass'],
    'status' => $parameters['status'],
    'language' => $parameters['language'],
    // User custom fields.
    'ces_firstname' => array(LANGUAGE_NONE => array(array('value' => $parameters['firstname']))),
    'ces_surname' => array(LANGUAGE_NONE => array(array('value' => $parameters['surname']))),
    'ces_address' => array(LANGUAGE_NONE => array(array('value' => $parameters['address']))) ,
    'ces_town' => array(LANGUAGE_NONE => array(array('value' => $parameters['town']))),
    'ces_postcode' => array(LANGUAGE_NONE => array(array('value' => $parameters['postcode']))),
    'ces_phonemobile' => array(LANGUAGE_NONE => array(array('value' => $parameters['phonemobile']))),
    'ces_phonework' => array(LANGUAGE_NONE => array(array('value' => $parameters['phonework']))),
    'ces_phonehome' => array(LANGUAGE_NONE => array(array('value' => $parameters['phonehome']))),
    'ces_website' => array(LANGUAGE_NONE => array(array('value' => $parameters['website']))),
    'roles' => array(
      DRUPAL_AUTHENTICATED_RID => 'authenticated user',
    ),
  );

  $user_drupal = FALSE;
  $user_drupal = user_save($user_drupal, $fields);
  if ($user_drupal === FALSE) {
    throw new Exception(t('Error creating Drupal user.'));
  }

  // If you want to send the welcome email, use the following code
  // Manually set the password so it appears in the e-mail.
  $user_drupal->password = $parameters['pass'];

  return $user_drupal;
}

/**
 * Add account.
 *
 * kind: Tipos de cuenta.
 *
 * INDIVIDUAL = 0 (Default)
 * SHARED = 1
 * ORGANIZATION = 2
 * COMPANY = 3
 * PUBLIC = 4
 * VIRTUAL = 5
 *
 * state: Estados de cuenta.
 * HIDDEN = 0
 * ACTIVE = 1 (Default)
 * LOCKED = 2
 * CLOSED = 3
 *
 */
function _ces_rest_add_account($parameters) {

  $bank = new CesBank();
  $exchange = ces_bank_get_current_exchange();
  $limit = $bank->getDefaultLimitChain($exchange['id']);
  $user_uid = $parameters['user'];
  $user = user_load($user_uid);

  // @todo Validate parameters.

  $name = ( isset($parameters['name']) ? $parameters['name'] : FALSE ); 

  if ( ! $name ) {
    $default_account = $bank->getDefaultAccount($exchange['id']);
    $name = $default_account['name'];
    // Es posible que no este libre.
    $code_exchange = substr($name,0,4);
    $number_account = substr($name,4,8);
    do {
      $number_account++;
      $name_free = $code_exchange . sprintf('%1$04d', $number_account );
      $account_free = $bank->getAccountByName($name_free);
    } while ( $bank->getAccountByName($name_free) !== FALSE );

    $name = $name_free;
  }

  try {

    $account = array(
      'exchange' => $exchange['id'],
      'name' => $name,
      'limitchain' => $limit['id'],
      'kind' => $parameters['kind'],
      //'state' => CesBankLocalAccount::STATE_HIDDEN,
      'state' => $parameters['state'],
      'users' => array(
        array(
          'user' => $user_uid,
          'role' => CesBankAccountUser::ROLE_ACCOUNT_ADMINISTRATOR,
        ),
      ),
    );

    $bank->createAccount($account, FALSE); 
    $bank->activateAccount($account);
    $output['account'] = $account; 
  }
  catch (Exception $e) {
    $output['error'] = $e->getMessage();
  }

  return $output;

}

/**
 * @} End of "defgroup ces_message_actions".
 */
