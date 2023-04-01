<?php

require_once __DIR__ . "/ces_komunitin.api.inc";

/**
 * Sends event to Notifications service.
 *
 * @param $action The ces_message action name. One of:
 * 'new_exchange', 'exchange_activated', 'new_account', 'account_activated',
 * 'account_debited', 'account_credited', 'send_offerwant_notify', 'account_credit_pending'
 * 'account_debit_pending', 'account_debit_rejected', 'account_credit_rejected',
 * 'alert_offerwant_expired', 'alert_offerwant_newoffer', 'alert_offerwant_newwant',
 * 'alert_offerwant_offersperiodical', 'alert_offerwant_wantsperiodical'.
 * @param $exchange_id
 * @param $user_recipient_id
 * @param $params array of params depending the action. For transaction events it
 * has:
 * 'transaction' => [
 *    'id' =>
 *    ...
 *  ]
 */
function ces_komunitin_notification_event($action, $exchange_id, $user_recipient_id, $params, $sender_id = false) {
  // Both account_credited and account_debited actions are called after a
  // transaction is committed.
  if ($action == 'account_credited') {
    // Create Event object.
    $exchange = ces_bank_get_exchange($exchange_id);
    $code = $exchange['code'];

    $bank = new CesBank();
    $transaction = $bank->getTransaction($params['transaction']['id']);
    $transfer_id = $transaction['uuid'];

    $urlPrefix = ces_komunitin_api_get_base_url() . '/accounting';
    $transferHref = $urlPrefix . '/' . $code . '/transfers/' . $transfer_id;
    $externalTransfer = new ExternalTransfer($transfer_id, 'transfers', $transferHref);

    $data = [
      'name' => Event::TRANSFER_COMMITTED,
      'code' => $code,
    ];

    $event = new Event($data, $externalTransfer);
    // Serialize payload.
    $content = CustomFactory::newEncoder(array(
      Event::class => EventSchema::class,
      ExternalTransfer::class => ExternalTransferSchema::class
    ))->withUrlPrefix($urlPrefix)
      ->encodeData($event);

    // Send payload to notifications service.
    $notifications_url = getenv('NOTIFICATIONS_API_URL');
    if (empty($notifications_url)) {
      watchdog('ces_komunitin', 'Missing NOTIFICATIONS_API_URL environment variable.', [], WATCHDOG_WARNING);
      $notifications_url = 'https://notifications.komunitin.org';
    }
    $url = $notifications_url . '/events';
    $result = drupal_http_request($url, [
      'method' => 'POST',
      'headers' => [
        'Content-Type' => 'application/vnd.api+json'
      ],
      'data' => $content
    ]);
    if (isset($result->error)) {
      watchdog('ces_komunitin', $result->code . ' ' . $result->error . '. ' . $result->data, [], WATCHDOG_ERROR);
      watchdog('ces_komunitin', "Sent: \n" . $content, [], WATCHDOG_DEBUG);
    }
  }
}
