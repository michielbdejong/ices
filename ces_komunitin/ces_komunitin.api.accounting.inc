<?php

/**
 * Check access and load a Transaction object. Send error on failure.
 *
 * @return Transaction
 */
function ces_komunitin_api_accounting_transactions_load($id) {
  $exchange = ces_bank_get_current_exchange();
  try {
    // This function may either return FALSE or throw exception.
    $transaction = ces_transaction_load($id);
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
