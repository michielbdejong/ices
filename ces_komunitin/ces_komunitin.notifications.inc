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
  if ($action == 'account_credited' || $action == 'account_credit_pending') {
    // Create Event object.
    $exchange = ces_bank_get_exchange($exchange_id);
    $code = $exchange['code'];

    $bank = new CesBank();
    $transaction = $bank->getTransaction($params['transaction']['id']);
    $transfer_id = $transaction['uuid'];

    $urlPrefix = ces_komunitin_api_get_base_url() . '/accounting';
    $transferHref = $urlPrefix . '/' . $code . '/transfers/' . $transfer_id;
    $externalTransfer = new ExternalTransfer($transfer_id, 'transfers', $transferHref);

    global $user;
    $userId = ces_komunitin_api_social_get_uuid(ResourceTypes::USER, $user->uid);
    $urlPrefix = ces_komunitin_api_get_base_url() . '/social';
    $userHref = $urlPrefix . '/users/' . $userId;
    $externalUser = new ExternalUser($userId, 'users', $userHref);

    $events = [
      'account_credit_pending' => Event::TRANSFER_PENDING,
      'account_credited' => Event::TRANSFER_COMMITTED
    ];
    $data = [
      'name' => $events[$action],
      'code' => $code,
    ];

    $event = new Event($data, $externalUser, $externalTransfer);
    ces_komunitin_send_event($event, $urlPrefix);
  }
}

function ces_komunitin_send_event($event, $urlPrefix) {
  // Serialize payload.
  $content = CustomFactory::newEncoder(array(
    Event::class => EventSchema::class,
    ExternalTransfer::class => ExternalTransferSchema::class,
    ExternalUser::class => ExternalUserSchema::class
  ))->withUrlPrefix($urlPrefix)
    ->encodeData($event);

  // Send payload to notifications service.
  $notifications_url = getenv('NOTIFICATIONS_API_URL');
  if (empty($notifications_url)) {
    watchdog('ces_komunitin', 'Missing NOTIFICATIONS_API_URL environment variable.', [], WATCHDOG_WARNING);
    $notifications_url = 'https://notifications.komunitin.org';
  }
  $url = $notifications_url . '/events';

  $username = variable_get('ces_komunitin_events_username', getenv('NOTIFICATIONS_EVENTS_USERNAME'));
  $password = variable_get('ces_komunitin_events_password', getenv('NOTIFICATIONS_EVENTS_PASSWORD'));

  if (strpos($notifications_url, 'https://') !== 0) {
    watchdog('ces_komunitin', 'Notifications URL %url is not secure. Your credentials may be stolen.', ['%url' => $notifications_url], WATCHDOG_WARNING);
  }
  $result = drupal_http_request($url, [
    'method' => 'POST',
    'headers' => [
      'Content-Type' => 'application/vnd.api+json',
      'Authorization' => 'Basic ' . base64_encode( $username . ':' . $password)
    ],
    'data' => $content
  ]);
  if (isset($result->error)) {
    watchdog('ces_komunitin', 'Error sending event: ' . $result->code . ' ' . $result->error . '. ' . (isset($result->data) ? $result->data : ''), [], WATCHDOG_ERROR);
    watchdog('ces_komunitin', "Event sent: \n" . $content, [], WATCHDOG_ERROR);
  }
}
