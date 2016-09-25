<?php
/**
 * @file
 * Functions from parse trades
 */

/**
 * @defgroup ces_import4ces_trades Parse trades from CES
 * @ingroup ces_import4ces
 * @{
 * Functions from parse trades
 */

/**
 * Parse trades.
 */
function ces_import4ces_parse_trades($import_id, $data, $row, &$context, $width_ajax = TRUE) {
  global $user;
  if (isset($context['results']['error'])) {
    return;
  }
  $extra_info = $data;
  $tx = db_transaction();
  try {
    ob_start();
    $context['results']['import_id'] = $import_id;
    $bank = new CesBank();

    $import = ces_import4ces_import_load($import_id);
    $exchange = $bank->getExchange($import->exchange_id);
    $code_exchange = $exchange['code'];
    $code_exchange_buyer = substr($data['buyer'], 0, 4);
    $code_exchange_seller = substr($data['seller'], 0, 4);

    $account_seller = _ces_import4ces_trades_get_account($import_id, $data['seller'], $data);
    if ($account_seller === FALSE) {
      throw new Exception(t('Acount @account not found.', array('@account' => $data['seller'])));
    }
    $account_buyer = _ces_import4ces_trades_get_account($import_id, $data['buyer'], $data);
    if ($account_buyer === FALSE) {
      throw new Exception(t('Acount @account not found.', array('@account' => $data['buyer'])));
    }
    // Find uid from user.
    $query = db_query('SELECT uid FROM {users} where name=:name', array(':name' => $data['entered_by']));
    $trade_user_id = $query->fetchColumn(0);
    if (!$trade_user_id) {
      $trade_user_id = $user->uid;
    }

    if ( $data['type'] == 'sess' ) {
      $amount = ( $data['buyer_amount'] + $data['buyer_levy'] );
    } 
    else {
      $amount = ( $data['buyer_amount'] - $data['buyer_levy'] );
    }

    $trans = array(
      'fromaccountname' => $account_buyer['name'],
      'toaccountname' => $account_seller['name'],
      'amount' => $amount, 
      'concept' => $data['description'],
      'user' => $trade_user_id,
      'created' => strtotime($data['date_entered']),
      'modified' => strtotime($data['date_entered']),
    );
    variable_set('ces_import4ces_mail', FALSE);
    $bank->createTransaction($trans);
    $bank->applyTransaction($trans['id']);
    variable_set('ces_import4ces_mail', CES_IMPORT4CES_SEND_MAILS);

    db_insert('ces_import4ces_objects')
      ->fields(array(
        'import_id' => $import_id,
        'object' => 'trades',
        'object_id' => $trans['id'],
        'row' => $row,
        'data' => serialize($extra_info),
      ))->execute();
    ces_import4ces_update_row($import_id, $row);
    ob_end_clean();
  }
  catch (Exception $e) {
    ob_end_clean();
    $tx->rollback();
    $context['results']['error'] = check_plain($e->getMessage());
    $_SESSION['ces_import4ces_row_error']['row']  = $row;
    $_SESSION['ces_import4ces_row_error']['m']    = $e->getMessage();
    $_SESSION['ces_import4ces_row_error']['data'] = $data;
    if ($width_ajax) {
      $result = array('status' => FALSE, 'data' => check_plain($e->getMessage()));
      die(json_encode($result));
    }
    else {
      ces_import4ces_batch_fail_row($import_id, array_keys($data), array_values($data), $row, $context);
    }
  }
}

/**
 * Get accounts.
 * 
 * Get an account record for this trade. If the trade is 
 * inter-exchange, it returns the proper interexchange 
 * virtual account, creating it if necessary.
 * 
 * @param int $import_id
 *   The id of this import.
 * @param string $name
 *   The name of the account to be retrieved.
 * @param array $data
 *   The full import line.
 * 
 * @return array
 *   The accout record or FALSE if it donesn't exist.
 */
function _ces_import4ces_trades_get_account($import_id, $name, $data) {
  $bank = new CesBank();

  $import = ces_import4ces_import_load($import_id);
  $exchange = $bank->getExchange($import->exchange_id);

  $code_exchange = $exchange['code'];
  $code_exchange_name = substr($name, 0, 4);
  $code_exchange_buyer = substr($data['buyer'], 0, 4);
  $code_exchange_seller = substr($data['seller'], 0, 4);
  $code_exchange_entered = substr($data['entered_by'], 0, 4);

  $external_account = FALSE;

  // Añadir transacciones del historico de la red.
  // Solamente lo hacemos sobre transacciones de la misma ecoxarxa ya que
  // una transacción sobre una cuenta de otra ecoxarxa provocará que se
  // modifique el saldo actual de la otra cuenta.
  if ( $code_exchange_name !== $code_exchange ) {
    $external_account = TRUE;
    $name = $code_exchange . $code_exchange_name;
  }

  if ( $external_account ) {
    // Jump record automatically.
    // ces_save_discarded_record($import_id, $data, 'External Account');

    $account = $bank->getAccountByName($name);
    if ($account === FALSE) {
      // The virtual account does not exist yet, so let's create it.
      $account = array(
        'id' => NULL,
        'exchange' => $exchange['id'],
        'name' => $name,
        'balance' => 0.0,
        'state' => CesBankLocalAccount::STATE_HIDDEN,
        'kind' => CesBankLocalAccount::TYPE_VIRTUAL,
        'limitchain' => $exchange['limitchain'],
        'users' => array(
          array(
            'account' => NULL,
            'user' => 1, // @todo Admin of exchange.
            'role' => CesBankAccountUser::ROLE_ACCOUNT_ADMINISTRATOR,
          ),
        ),
      );
      $bank->createAccount($account);
      $bank->activateAccount($account);
    }
    return $account;

  }

  $account = $bank->getAccountByName($name);
  if ($account === FALSE) {

    if ( $data['type'] !== 'sess' ) {
      if ( $data['type'] == 'dess' ) {
        // This is an inter-exchange transaction. Use the corresponding virtual
        // account. Example: NET2NET1
        if ( $code_exchange_name !== $code_exchange_buyer ) {
          $name = $code_exchange_name . $code_exchange_buyer;
        }
        else {
          $name = $code_exchange_name . $code_exchange_seller;
        }
      }
      else {
        // @todo Revision.
        // This is a remote transaction. The cen ID appears to be in the RecordID.
        // Example: HORAcen0746
        if ( $code_exchange_name !== $code_exchange_buyer ) {
          $name = $code_exchange_name . 'cen' . substr($data['buyer_nid'], 3, 4);
        }
        else {
          $name = $code_exchange_name . 'cen' . substr($data['seller_nid'], 3, 4);
        }
      }

    }


    // The virtual account does not exist yet, so let's create it.
    $import = ces_import4ces_import_load($import_id);
    $exchange = $bank->getExchange($import->exchange_id);
    $account = array(
      'id' => NULL,
      'exchange' => $exchange['id'],
      'name' => $name,
      'balance' => 0.0,
      'state' => CesBankLocalAccount::STATE_HIDDEN,
      'kind' => CesBankLocalAccount::TYPE_VIRTUAL,
      'limitchain' => $exchange['limitchain'],
      'users' => array(
        array(
          'account' => NULL,
          'user' => 1,
          'role' => CesBankAccountUser::ROLE_ACCOUNT_ADMINISTRATOR,
        ),
      ),
    );
    $bank->createAccount($account);
    $bank->activateAccount($account);
  }
  else {
    $account = $bank->getAccountByName($name);
  }
  return $account;
}
/** @} */
