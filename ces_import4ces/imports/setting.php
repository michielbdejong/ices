<?php
/**
 * @file
 * Functions from parse setting
 */

/**
 * @defgroup ces_import4ces_setting Parse setting from CES
 * @ingroup ces_import4ces
 * @{
 * Functions from parse setting
 */

/**
 * Parse exchange settings.
 */
function ces_import4ces_parse_setting($import_id, $setting, $row, &$context, $width_ajax = TRUE) {
  if (isset($context['results']['error'])) {
    return;
  }

  $tx = db_transaction();
  try {
    ob_start();
    // Crear bank.
    $bank = new CesBank();

    // Create exchange administrator drupal user. It will be completed in users
    // step.
    $record = array(
      'name' => $setting['xid'] . '0000',
      'mail' => (CES_IMPORT4CES_ANONYMOUS) ? 'test-' . $setting['xid'] .
      '0000@test.com' : $setting['exchange_email'],
      'pass' => $setting['nid_psw'],
      'status' => 1,
      'roles' => array(DRUPAL_AUTHENTICATED_RID => 'authenticated user'),
    );
    $admin_user = user_save('', $record);
    // Compute curency value and currency scale.
    $timevalues = array(
      'h' => 1,
      'm' => 0.01666667,
    );
    $currvalues = array(
      'Euro' => '0.1',
    );
    $currencyvalue = 1;
    // if ($setting['TimeBased'] == -1) {
    //   if (isset($timevalues[$setting['TimeUnit']])) {
    //     $currencyvalue = $timevalues[$setting['TimeUnit']];
    //   }
    // }
    // else {
    //   if (isset($currvalues[$setting['ConCurName']])) {
    //     $currencyvalue = $currvalues[$setting['ConCurName']];
    //   }
    // }
    // Create exchange.
    $exchange = array(
      'code' => $setting['xid'],
      'shortname' => $setting['short_name'],
      'name' => $setting['full_title'],
      'website' => $setting['web_address'],
      'country' => $setting['country_code'],
      'region' => $setting['province'],
      'town' => $setting['location'],
      'map' => $setting['map_address'],
      'currencysymbol' => html_entity_decode($setting['concurrency_symbol'], ENT_QUOTES, 'UTF-8') ,
      'currencyname' => $setting['currency_name'],
      'currenciesname' => $setting['currency_name_plural'],
      'currencyvalue' => $currencyvalue,
      'currencyscale' => 2,
      'admin' => $admin_user->uid,
      'data' => array(
        'registration_offers' => 1,
        'registration_wants' => 0,
        'default_lang' => _ces_import4ces_get_language($setting['language_short']),
      ),
    );

    // Save data in DB.
    $extra_data = $setting;

    $bank->createExchange($exchange);
    $bank->activateExchange($exchange);
    // Update default credit/debit limit.
    $default_limit = $bank->getLimitChain($exchange['limitchain']);
    $default_credit = $setting['CredLim'];
    $default_debit = $setting['DebLim'];
    if ($default_credit != 0) {
      $default_limit['limits'][] = array(
        'classname' => 'CesBankAbsoluteCreditLimit',
        'value' => $default_credit,
        'block' => FALSE,
      );
    }
    if ($default_debit != 0) {
      $default_limit['limits'][] = array(
        'classname' => 'CesBankAbsoluteDebitLimit',
        'value' => -$default_debit,
        'block' => FALSE,
      );
    }
    $bank->updateLimitChain($default_limit);
    db_update('ces_import4ces_exchange')
    ->condition('id', $import_id)->fields(array(
      'exchange_id' => $exchange['id'],
      'data' => serialize($extra_data),
    ))->execute();
    ces_import4ces_update_row($import_id, $row);
    $context['message'] = check_plain($exchange['name']);
    $context['results']['import_id'] = $import_id;
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
/** @} */
