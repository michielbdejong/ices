<?php
require_once __DIR__ . '/ces_komunitin.api.social.inc';
/**
 * Load a transaction ces_bank record. Send error on failure.
 *
 * @return array transaction record.
 */
function ces_komunitin_api_accounting_transfers_load($exchange, $id) {
  try {
    // This function may either return FALSE or throw exception.
    $bank = new CesBank();
    $transaction = $bank->getTransactionByUuid($id);
  } catch (Exception $e) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
  }
  if (!$transaction) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
  }
  return $transaction;
}

/**
 * CHeck access and read a transaction.
 *
 * @return Transfer
 */
function ces_komunitin_api_accounting_transfers_read($exchange, array $transaction) {
  if (ces_komunitin_api_client_access('komunitin_accounting_read_all')
    || ces_transaction_access('view', $transaction)) {
    return new Transfer($transaction, $exchange, new Currency($exchange));
  }
  // Return a 404 Not found error when the user does not have access to READ
  // the resource. This way we don't disclose whether the record actually
  // exists or not.
  ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
}

/**
 * Create a new transfer. Depending on the initial state, tries to commit it.
 *
 * @param $body Object
 *
 * @return Transfer
 */
function ces_komunitin_api_accounting_transfers_create($exchange, $body) {
  // 1. Validation.
  // 1.1 Root object.
  if (!isset($body->data)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $data = $body->data;

  // 1.2 type & id.
  if ($data->type !== 'transfers') {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Bad type');
  }
  $id = strtolower($data->id);
  if (!ces_komunitin_api_check_valid_uuid($id)) {
    ces_komunitin_api_send_error(KomunitinApiError::INVALID_ID);
  }
  $bank = new CesBank();
  $duplicated = $bank->getTransactionByUuid($id);
  if (!empty($duplicated)) {
    ces_komunitin_api_send_error(KomunitinApiError::DUPLICATED_ID);
  }

  // 1.3 Attributes
  if (!isset($data->attributes)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Missing attributes');
  }
  $attributes = $data->attributes;

  // 1.3.1 Meta. Only support string by now.
  if (!is_string($attributes->meta)) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED, 'Invalid meta');
  }
  $meta = $attributes->meta;

  // 1.3.2 Amount.
  if (!is_int($attributes->amount) || $attributes->amount <= 0) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_AMOUNT);
  }
  $amount = $attributes->amount / pow(10, $exchange['currencyscale']);

  // 1.3.3 State
  $state = $attributes->state ?? Transfer::STATE_NEW;
  // Allow only new and committed states on creation.
  if ($state !== Transfer::STATE_NEW && $state !== Transfer::STATE_COMMITTED) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_TRANSFER_STATE);
  }

  // 1.4 Relationships.
  if (!isset($data->relationships) || !isset($data->relationships->payer) || !isset($data->relationships->payee)
  || !isset($data->relationships->payer->data) || !isset($data->relationships->payee->data)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Missing payer or payee in transfer');
  }

  // 1.4.1 Payer.
  $payer = $bank->getAccountByUUID($data->relationships->payer->data->id);
  if (!$payer) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_PAYER);
  }

  // 1.4.2 Payee.
  $payee = $bank->getAccountByUUID($data->relationships->payee->data->id);
  if (!$payee) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_PAYEE);
  }

  // Only support local transfers by now.
  if ($payer['exchange'] != $exchange['id'] || $payee['exchange'] != $exchange['id']) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED, 'External transfers are not supported');
  }

  // 2. Perform transaction.
  $transaction = array(
    'uuid' => $id,
    'fromaccount' => $payer['id'],
    'toaccount' => $payee['id'],
    'amount' => $amount,
    'concept' => $meta,
  );

  // 2.1 Check permission
  if (!ces_transaction_access('use', $transaction)) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }

  // 2.2 Create transaction
  $bank->createTransaction($transaction);

  // 2.3 Apply transaction
  if ($state == Transfer::STATE_COMMITTED) {
    try {
      $bank->applyTransaction($transaction['id']);
    }
    catch (Exception $e) {
      ces_komunitin_api_send_error(KomunitinApiError::TRANSFER_ERROR, $e->getMessage());
    }
  }

  // 3. Return transaction.
  $transaction = $bank->getTransaction($transaction['id']);
  $currency = new Currency($exchange);
  return new Transfer($transaction, $exchange, $currency);
}

/**
 * Update transaction. Applies the transaction if the state is updated.
 */
function ces_komunitin_api_accounting_transfers_update($exchange, array $transaction, $body) {
  // Check access.
  if (!(ces_transaction_access('edit', $transaction))) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
  if (!isset($body->data)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $data = $body->data;

  // 1.2 type & id.
  if ($data->type !== 'transfers') {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Bad type');
  }
  // Use komunitin data model.
  $oldtransfer = ces_komunitin_api_accounting_transfers_read($exchange, $transaction);

  // Check id.
  $id = strtolower($data->id);
  if ($oldtransfer->id != $id) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Incorrect id');
  }

  if (isset($data->attributes)) {
    $atts = $data->attributes;
    // Check we're not trying to update other fields than state (though we could
    // allow changing fields before committing).
    if ((isset($atts->meta) && $atts->meta != $oldtransfer->meta)
      || (isset($atts->amount) && $atts->amount != $oldtransfer->amount)) {
      ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN, 'It is not allowed to modify a transaction after it is created');
    }
    if ($atts->state != $oldtransfer->state) {
      if ($oldtransfer->state == Transfer::STATE_NEW || $oldtransfer->state == Transfer::STATE_PENDING) {
        global $user;
        $bank = new CesBank();
        $accounts = $bank->getUserAccounts($user->uid);
        $account = null;
        foreach ($accounts as $one) {
          if ($one['id'] == $transaction['fromaccount'] || $one['id'] == $transaction['toaccount']) {
            $account = $one;
            break;
          }
        }
        if ($account == null) {
          // . print_r($accounts, true) . print_r($transaction, true)
          ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'You don\'t have any account in this transaction.');
        }
        if ($atts->state == Transfer::STATE_ACCEPTED || $atts->state == Transfer::STATE_COMMITTED) {
          // Accept pending transaction.
          try {
            $bank->acceptTransaction($transaction['id'], $account['id']);
          } catch (Exception $e) {
            ces_komunitin_api_send_error(KomunitinApiError::INTERNAL_SERVER_ERROR, $e->getMessage());
          }
        } else if ($atts->state == Transfer::STATE_REJECTED || $atts->state == Transfer::STATE_DELETED) {
          // Reject pending transaction.
          try {
            $bank->rejectTransaction($transaction['id'], $account['id']);
          } catch (Exception $e) {
            ces_komunitin_api_send_error(KomunitinApiError::INTERNAL_SERVER_ERROR, $e->getMessage());
          }
        } else {
          // unexpected state change.
          ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Invalid state change');
        }
        $transaction = $bank->getTransaction($transaction['id']);
        return ces_komunitin_api_accounting_transfers_read($exchange, $transaction);
      } else if ($oldtransfer->state == Transfer::STATE_COMMITTED || $oldtransfer->state == Transfer::STATE_DELETED) {
        ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN, 'Can\'t update a committed or deleted transfer');
      } else {
        // updating a accepted or rejected transfer, but still not committed nor deleted.
        // We don't still have a use case for that.
        ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED, 'Can\'t update a transfer with accepted or rejected state');
      }
    } else {
      // This is an empty update.
      return $oldtransfer;
    }
  }

}

/**
 * Check access and delete an existing transaction.
 */
function ces_komunitin_api_accounting_transfers_delete($exchange, array $transaction) {
  // Check access.
  if (!(ces_transaction_access('edit', $transaction))) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
  // Delete transaction.
  $bank = new CesBank();
  try {
    $bank->deleteTransaction($transaction['id']);
    return TRUE;
  }
  catch (Exception $e) {
    ces_komunitin_api_send_error(KomunitinApiError::TRANSFER_ERROR, $e->getMessage());
  }
}

/**
 * Load ces_bank account record.
 */
function ces_komunitin_api_accounting_accounts_load($exchange, $code) {
  try {
    // This function may either return FALSE or throw exception.
    $bank = new CesBank();
    if (ces_komunitin_api_social_is_uuid($code)) {
      $account = $bank->getAccountByUuid($code);
    }
    else {
      $account = $bank->getAccountByName($code);
    }
  } catch (Exception $e) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
  }
  if (!$account) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
  }
  return $account;
}

/**
 * Read an Account.
 *
 * @return Account
 */
function ces_komunitin_api_accounting_accounts_read($exchange, array $account)
{
  // Allow all authorized users to see basic account information: id, code,
  // balance, credit/debit limits for all accounts in all exchange groups.
  return new Account($account, new Currency($exchange));
}

/**
 * Load a collection of transfers.
 *
 * @param $exchange The exchange record.
 * @param $fields The field JSONApi parameters
 * @param $filters The filter JSONApi parameter
 * @param $sorts The sort JSONApi parameter
 *
 * @return array of transaction records.
 *
 */
function ces_komunitin_api_accounting_transfers_load_collection($exchange, $filters, $sorts, $pageAfter, $pageSize) {
  try {
    $bank = new CesBank();

    // Compute conditions field.
    $conditions = [];
    foreach ($filters as $field => $condition) {
      if ($field === 'account') {
        if(count($condition) !== 1) {
          ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED, "Only one account is supported in filtering.");
        }
        $uuid = $condition[0];
        $account = $bank->getAccountByUuid($uuid);
        if ($account === FALSE) {
          ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND, "Account not found.");
        }
        $conditions['account'] = $account['id'];
      }
      else {
        ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED, "Filtering other than 'filter[account]' is not supported.");
      }
    }

    // Compute order fields.
    $order_field = 'created';
    $order_type = 'ASC';
    foreach ($sorts as $field => $isAsc) {
      if ($field == 'updated') {
        $order_field = 'modified';
        $order_type = $isAsc ? 'ASC' : 'DESC';
      }
      else if ($field == 'created') {
        $order_field = 'created';
        $order_type = $isAsc ? 'ASC' : 'DESC';

      }
      else {
        ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED, "Ordering field other than 'updated' or 'created' is not supported.");
      }
    }

    // Pagination
    $conditions['limit'] = $pageSize;
    $conditions['offset'] = $pageAfter;

    // Run getTransactions() funtion.
    $records = $bank->getTransactions($conditions, $order_field, $order_type);
    return $records;

  } catch (Exception $e) {
    // No acceptable reasons for an exception.
    ces_komunitin_api_send_error(KomunitinApiError::INTERNAL_SERVER_ERROR, $e->getMessage());
  }
}

function ces_komunitin_api_accounting_transfers_read_collection($exchange, array $transactions) {
  $currency = new Currency($exchange);
  $result = [];
  foreach($transactions as $transaction) {
    // Discard transactions without view access
    if (ces_transaction_access('view', $transaction)) {
      $result[] = new Transfer($transaction, $exchange, $currency);
    }
  }
  return $result;
}

function ces_komunitin_api_accounting_currency_read($exchange) {
  // Allow all authorized users to see currency information.
  return new Currency($exchange);
}


function ces_komunitin_api_accounting_accounts_load_collection($exchange, $filters, $sorts, $pageAfter, $pageSize) {
  $bank = new CesBank();
  $conditions = [];

  // Set implicit exchange filter field.
  $conditions['exchange'] = $exchange['id'];

  // Only active accounts by default.
  $conditions['state'] = 1;

  // Only regular accounts (skip virtual ones!)
  $conditions['kind'] = [0,1,2,3,4];

  // Apply filters.
  foreach ($filters as $field => $value) {
    // Fix names.
    if ($field == 'id') {
      $field = 'uuid';
    }
    // $value is an array if the query parameter had several comma-separated values,
    // which is correctly interpreted as en IN clause.
    $conditions[$field] = $value;
  }
  // Order

  // Pagination.
  $conditions['limit'] = $pageSize;
  $conditions['offset'] = $pageAfter;

  return $bank->getAllAccounts($conditions, 'id');
}

function ces_komunitin_api_accounting_accounts_read_collection($exchange, array $accounts) {
  $result = [];
  $currency = new Currency($exchange);
  foreach($accounts as $account) {
    $result[] = new Account($account, $currency);
  }
  return $result;
}

function ces_komunitin_api_accounting_accounts_settings_read($exchange, array $account) {
  // Require edit access to account just to see the settings.
  if (!ces_bank_access('edit', 'account', $account['id'])) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
  return new AccountSettings($account, new Currency($exchange));
}

function ces_komunitin_api_accounting_accounts_settings_update($exchange, array $account, $body) {
  if (!ces_bank_access('edit', 'account', $account['id'])) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
  if (!isset($body->data) || !isset($body->data->attributes)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $attributes = $body->data->attributes;
  if (isset($attributes->acceptPaymentsAutomatically)) {
    $bank = new CesBank();
    $account['data']['accept']['manual'] = !$attributes->acceptPaymentsAutomatically;
    $bank->updateAccount($account);
  }
  return new AccountSettings($account, new Currency($exchange));
}
