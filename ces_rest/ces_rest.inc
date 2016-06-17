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
 * Index ces rest exchange.
 */
function _ces_rest_exchange_index($code_exchange) {
  
  global $user;

  $output = FALSE;

  $bank = new CesBank();
  $exchange = $bank->getExchangeByName($code_exchange);
  return $exchange;

  }
/**
 * Index ces rest transactions.
 *
 * @todo Acabar función.
 * @todo Búsqueda por fecha, cuenta, ....
 */
function _ces_rest_transactions_index($code_exchange, $account, $parameters) {
  
  global $user;

  $output = FALSE;

  $exchange = ces_exchange_load($code_exchange);
  $bank = new CesBank();
  $exchange = $bank->getExchange($exchange['id']);

  if ($exchange === FALSE) {
    $output['error'] = t('Invalid exchange');
  }

  // If uid print info user.
  if ( $account ) {
    // Id or Name.
    if ( intval($account) ) {
      $account = $bank->getAccount($account);
    }
    else {
      $account = $bank->getAccountByName($account);
    }
  }

  // ces_bank.pages.inc line: 765
  if ($account) {
    $conditions['account'] = $account['id'];
  }
  else {
    $conditions['exchange'] = $exchange['id'];
  }
  return $transactions = $bank->getTransactions($conditions);
}

/**
 * Index ces rest.
 *
 * @todo Búsqueda de usuario por Cuenta, nombre, apellido, email.
 */
function _ces_rest_index($code_exchange, $uid, $parameters) {
  
  global $user;

  $output = FALSE;

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
  $exchange = ces_exchange_load($code_exchange);
  $accounts = $bank->getAllAccounts(array('exchange' => $exchange['id']));

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
 */

function _ces_rest_add_user($code_exchange, $parameters) {

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
 * Update user.
 */

function _ces_rest_update_user($code_exchange, $uid, $parameters) {

  // The user fields.
  foreach( $parameters AS $key => $value ) {
    switch ($key) {
      case 'name':
        $fields['name'] = $value;
        break;
      
      case 'mail':
        $fields['mail'] = $value;
        break;
      
      case 'pass':
        $fields['pass'] = $value;
        break;
      
      case 'status':
        $fields['status'] = $value;
        break;
      
      case 'language':
        $fields['language'] = $value;
        break;
      
      case 'ces_firstname':
        $fields['ces_firstname'] = array(
          LANGUAGE_NONE => array(
            array(
              'value' => $value,
            )
          )
        );
        break;
      
      case 'ces_surname':
        $fields['ces_surname'] = array(
          LANGUAGE_NONE => array(
            array(
              'value' => $value,
            )
          )
        );
        break;
      
      case 'ces_address':
        $fields['ces_address'] = array(
          LANGUAGE_NONE => array(
            array(
              'value' => $value,
            )
          )
        );
        break;
      
      case 'ces_town':
        $fields['ces_town'] = array(
          LANGUAGE_NONE => array(
            array(
              'value' => $value,
            )
          )
        );
        break;
      
      case 'ces_postcode':
        $fields['ces_postcode'] = array(
          LANGUAGE_NONE => array(
            array(
              'value' => $value,
            )
          )
        );
        break;
      
      case 'ces_phonemobile':
        $fields['ces_phonemobile'] = array(
          LANGUAGE_NONE => array(
            array(
              'value' => $value,
            )
          )
        );
        break;
      
      case 'ces_phonework':
        $fields['ces_phonework'] = array(
          LANGUAGE_NONE => array(
            array(
              'value' => $value,
            )
          )
        );
        break;
      
      case 'ces_phonehome':
        $fields['ces_phonehome'] = array(
          LANGUAGE_NONE => array(
            array(
              'value' => $value,
            )
          )
        );
        break;
      
      case 'ces_website':
        $fields['ces_website'] = array(
          LANGUAGE_NONE => array(
            array(
              'value' => $value,
            )
          )
        );
        break;
      
      default:
        // @todo Error campo desconocido.
        $output['error'][] = 'Campo ['.$key.'] desconocido ['.$value.']'; 
        break;
    }
  }

  $user_drupal = user_load($uid);
  if ($user_drupal === FALSE) {
    throw new Exception(t('Error: nonexistent user (' . $uid . ').'));
  }

  $user_drupal = user_save($user_drupal, $fields);
  if ($user_drupal === FALSE) {
    throw new Exception(t('Error updating user.'));
  }

  $output['user'] = $user_drupal;
  return $output;
}

/**
 * Index ces rest.
 */
function _ces_rest_accounts_index($code_exchange, $account, $parameters) {
  
  global $user;

  $output = FALSE;

  $bank = new CesBank();
  $exchange = ces_exchange_load($code_exchange);

  // If uid print info user.
  if ( $account ) {
    // Id or Name.
    if ( intval($account) ) {
      $output = $bank->getAccount($account);
    }
    else {
      $output = $bank->getAccountByName($account);
    }
    return $output;
  }


  // If not uid print list users.
  $accounts = $bank->getAllAccounts(array('exchange' => $exchange['id']));

  foreach ($accounts as $account) {
    $output[] = $account;
  }

  return $output;
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
function _ces_rest_add_account($code_exchange, $parameters) {

  $bank = new CesBank();
  $exchange = ces_exchange_load($code_exchange);
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
 * Update account.
 */
function _ces_rest_update_account($code_exchange, $account_id, $parameters) {

  $bank = new CesBank();
  $exchange = ces_exchange_load($code_exchange);

  // If uid print info user.
  if ( $account_id ) {
    // Id or Name.
    if ( is_numeric($account_id) ) {
      $account = $bank->getAccount($account_id);
    }
    else {
      $account = $bank->getAccountByName($account_id);
    }
  }

  try {

    foreach ($parameters AS $key => $value) {
      $account[$key] = $value;
    }

    $bank->updateAccount($account, FALSE); 
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
