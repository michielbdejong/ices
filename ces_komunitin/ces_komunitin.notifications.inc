<?php

require_once __DIR__ . "/ces_komunitin.api.inc";

/**
 * Public function to be called to copy a mail notifcation to the notifications
 * service. If you want to directly notify of an event, use the ces_komunitin_event
 * function.
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
  if ($action == 'account_credited' || $action == 'account_debit_pending' || $action == 'account_credit_rejected') {
    // Create Event object.
    $exchange = ces_bank_get_exchange($exchange_id);
    $code = $exchange['code'];

    $bank = new CesBank();
    $transaction = $bank->getTransaction($params['transaction']['id']);
    $transfer_id = $transaction['uuid'];
    $payer = $bank->getTransactionFromAccount($transaction);
    $payee = $bank->getTransactionToAccount($transaction);

    $events = [
      'account_debit_pending' => Event::TRANSFER_PENDING,
      'account_credited' => Event::TRANSFER_COMMITTED,
      'account_credit_rejected' => Event::TRANSFER_REJECTED,
    ];

    ces_komunitin_event($events[$action], $code, array(
      'transfer' => $transfer_id,
      'payer' => ces_komunitin_api_social_get_uuid(ResourceTypes::MEMBER, $payer['id']),
      'payee' => ces_komunitin_api_social_get_uuid(ResourceTypes::MEMBER, $payee['id']),
    ));

  } else if ($action == 'account_activated') {
    $exchange = ces_bank_get_exchange($exchange_id);
    $code = $exchange['code'];
    $member = $params['account'];
    ces_komunitin_event(Event::MEMBER_JOINED, $code, array(
      'member' => ces_komunitin_api_social_get_uuid(ResourceTypes::MEMBER, $member['id'])
    ));
  }
}

/**
 * Public function to be called to notify an event to the notifications service.
 * @param $name The event name, from Event class constants.
 * @param $code The exchange code.
 * @param $data The event data as an string-indexed associative array.
 */
function ces_komunitin_event($name, $code, $data) {
  $event = Event::create($name, $code, $data);
  ces_komunitin_send_event($event);
}

/**
 * Helper function, use ces_komunitin_event instead.
 *
 * Sends an event object to the notifications service
 */
function ces_komunitin_send_event($event) {
  // Serialize payload.
  $urlPrefix = ces_komunitin_api_get_base_url() . '/social';
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

/**
 * Called from module file on entity_insert and entity_update hooks.
 */
function ces_komunitin_notifications_entity_change($entity, $type) {
  if ($type == 'ces_offerwant') {
    module_load_include('inc', 'ces_komunitin', 'ces_komunitin.api.social');

    $published = $entity->state == 1 && $entity->expire > REQUEST_TIME;
    $oldpublished = isset($entity->original) && $entity->original->state == 1 && $entity->original->expire > REQUEST_TIME;

    if ($published && !$oldpublished) {
      // The offer or need has just been published.
      $category = ces_category_load($entity->category);
      $exchange = ces_bank_get_exchange($category->exchange);

      // Check if account is active before sending notifications.
      $account_id = _ces_komunitin_api_social_user_account_id($entity->user, $exchange['id']);
      $bank = new CesBank();
      $account = $bank->getAccount($account_id);
      if ($account['state'] == CesBankLocalAccount::STATE_ACTIVE) {
        if ($entity->type == 'offer') {
          ces_komunitin_event(Event::OFFER_PUBLISHED, $exchange['code'], array(
            'offer' => ces_komunitin_api_social_get_uuid(ResourceTypes::OFFER, $entity->id),
          ));
        } else {
          ces_komunitin_event(Event::NEED_PUBLISHED, $exchange['code'], array(
            'need' => ces_komunitin_api_social_get_uuid(ResourceTypes::NEED, $entity->id),
          ));
        }
      }
    }
  }
}
