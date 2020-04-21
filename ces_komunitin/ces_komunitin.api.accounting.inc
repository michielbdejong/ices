<?php

/**
 * Check access and load a Transaction object. Send error on failure.
 *
 * @return Transaction
 */
function ces_komunitin_api_accounting_transactions_load($exchange, $id) {
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
  if (!ces_transaction_access('view', $transaction)) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
  return new Transaction(_ces_komunitin_api_get_base_url(), $transaction, $exchange);
}

/**
 * Create a new transaction. Depending on the initial state, tries to commit it.
 *
 * @param $data Object
 *
 * @return Transaction
 */
function ces_komunitin_api_accounting_transactions_create($exchange, $body) {
  // 1. Validation.

  // 1.1 Root object.
  if (!isset($body->data)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $data = $body->data;

  // 1.2 type & id.
  if ($data->type !== 'transactions') {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $id = $data->id;
  if (!ces_komunitin_api_check_valid_uuid($id)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $bank = new CesBank();
  $duplicated = $bank->getTransactionByUuid($id);
  if (!empty($duplicated)) {
    ces_komunitin_api_send_error(KomunitinApiError::DUPLICATED_ID);
  }

  // 1.3 Attributes
  if (!isset($data->attributes)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }

  // 1.4 Transfer
  $attributes = $data->attributes;
  if (empty($attributes->transfers) || ! is_array($attributes->transfers)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  // Only a single transfer per transaction is implemented here.
  if (count($attributes->transfers) !== 1) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED);
  }
  $transfer = $attributes->transfers[0];

  // 1.4.1 Accounts.
  $payer = ces_komunitin_api_accounting_get_account_from_url($transfer->payer);
  $payee = ces_komunitin_api_accounting_get_account_from_url($transfer->payee);
  if (!$payer || !$payee) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  // Only support local transaction by now.
  if ($payer['exchange'] != $exchange['id'] || $payee['exchange'] != $exchange['id']) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED);
  }
  // Only support string meta by now.
  if (!is_string($transfer->meta)) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED);
  }
  $meta = $transfer->meta;

  // 1.4.2 Amount.
  if (!is_int($transfer->amount) || $transfer->amount <= 0) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $amount = $transfer->amount / pow(10, $exchange['currencyscale']);

  // 1.5 State
  $state = $attributes->state ?? Transaction::STATE_NEW;
  // Allow only new and committed states on creation.
  if ($state !== Transaction::STATE_NEW && $state !== Transaction::STATE_COMMITTED) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
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
  if ($state == Transaction::STATE_COMMITTED) {
    $bank->applyTransaction($transaction['id']);
  }

  // 3. Return transaction.
  $transaction = $bank->getTransaction($transaction['id']);
  return new Transaction(_ces_komunitin_api_get_base_url(), $transaction, $exchange);
}

/**
 * @return Account record or FALSE.
 */
function ces_komunitin_api_accounting_get_account_from_url($url) {
  $name = array_pop(explode('/', $url));
  if ($name) {
    $bank = new CesBank();
    return $bank->getAccountByName($name);
  }
  else {
    return FALSE;
  }
}

