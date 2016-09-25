<?php
/**
 * @file
 * Functions from parse balances.
 */

/**
 * @defgroup ces_import4ces_balances Parse balances from CES
 * @ingroup ces_import4ces
 * @{
 * Functions from parse balances
 */

/**
 * Checking balances.
 */
function ces_import4ces_parse_balances($import_id, $data, $row, &$context, $width_ajax = TRUE) {
  global $user;
  if (isset($context['results']['error'])) {
    return;
  }
  $tx = db_transaction();
  try {
    ob_start();
    $context['results']['import_id'] = $import_id;

    // $account_update = db_update('ces_account')
    //   ->condition('name', $data['uid'])
    //   ->fields(array('balance' => $data['balance'], 'state' => 1))
    //   ->execute();

    $balance = db_query('SELECT balance FROM {ces_account}
      WHERE name = :name ',
      array(
        ':name' => $data['uid'],
      )
    )->fetchCol(0);
    $balance = (real) $balance[0];
    $balance_row = (real) $data['balance'];

    if ( $balance !== $balance_row ) {
      ces_save_discarded_record($import_id, $data, 
        'Actual balance: ' . $balance . ' Dif: ' . round( ( $balance - $balance_row ), 2 )
      );
    }

    db_insert('ces_import4ces_objects')
      ->fields(array(
        'import_id' => $import_id,
        'object' => 'balances',
        'object_id' => 0,
        'row' => $row,
        'data' => serialize($data),
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

/** @} */
