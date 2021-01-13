<?php

class ResourceTypes {
  const USER = "user";
  const GROUP = "group";
  const EMAIL = "email";
  const PHONE = "phone";
  const MEMBER = "member";
  const NEED = "need";
  const OFFER = "offer";
  const CATEGORY = "category";

  const ALL_TYPES = [self::GROUP, self::EMAIL, self::PHONE, self::MEMBER, self::NEED, self::OFFER];
}

function ces_komunitin_api_social_get_uuid($type, $id) {
  // $id is a 32-bit identifier from the database.
  // The pseudo UUIDv4 is formed by
  // Type (T): 12 hex = 48 bits = 6 bytes = 6 characters
  // Identifier (I): 12hex = 48bits = 0x00 + 32bits integer.
  // Random (R): 6hex = 24bits = 3 bytes
  // 4, A: Fixed positions.

  // TTTTTTTT-TTTT-4RRR-ARRR-IIIIIIIIIIII

  //1. $type to 6 chars length:
  $type = substr($type, 0, 6);
  while (strlen($type) < 6) {
    $type .= ' ';
  }
  $type = bin2hex($type);

  //2. random bits. Not really random but tied to the current installation,
  //since otherwise it'd get different uuids for the same id.
  $random = bin2hex(substr(drupal_get_hash_salt(),0,3));

  //3. id
  $id = substr("000000000000" . dechex((int) $id), -12);

  // Give the uuid format.
  $uuid = substr($type, 0, 8) . '-' . substr($type, 8) . '-4' . substr($random, 0, 3) . '-a' . substr($random, 3) . '-' . $id;

  return $uuid;
}

function ces_komunitin_api_social_get_id($uuid) {
  // Recover type.
  $type = substr($uuid, 0, 8) . substr($uuid, 9, 4);
  $type = trim(hex2bin($type));
  foreach (ResourceTypes::ALL_TYPES as $item) {
    if (substr($item, strlen($type)) == $type) { //item.startswith(type).
      $type = $item;
    }
  }
  // recover id
  $id = substr($uuid, -12);
  $id = hexdec($id);

  $result = new StdClass();
  $result->type = $type;
  $result->id = $id;

  return $result;
}

function ces_komunitin_api_social_groups_read($exchange) {
  return new Group($exchange);
}

function ces_komunitin_api_social_groups_load_collection($exchange, $filters, $sorts) {
  $bank = new CesBank();

  // Only active excahnges.
  $conditions = array('state' => 1);
  $exchanges = $bank->getAllExchanges($conditions);
  return $exchanges;
}

function ces_komunitin_api_social_groups_read_collection($exchanges) {
  // Allow everyone to see groups list.
  $groups = [];
  foreach ($exchanges as $exchange) {
    $groups[] = ces_komunitin_api_social_groups_read($exchange);
  }
  return $groups;
}

function ces_komunitin_api_social_users_read() {
  global $user;
  $exchange = ces_bank_get_current_exchange();
  return new User($user, $exchange);
}

function ces_komunitin_api_social_members_load($exchange, $code) {
  // Code is the drupal user name.
  $user = user_load_by_name($code);
  return $user;
}
function ces_komunitin_api_social_members_load_collection($exchange, $filters, $sorts) {
  // Get list of uids directly from database call.
  $query = db_select('users', 'u')
    ->fields('u', array('uid'));
  $query->join('ces_accountuser', 'au', 'au.user = u.uid');
  $query->join('ces_account', 'a', 'a.id = au.account');
  $ids = $query->condition('a.exchange', $exchange['id'])
    // discard blocked users.
    ->condition('u.status', 1)
    // discard inactive accounts (new, deleted).
    ->condition('a.state', 1)
    // discard virtual accounts
    ->condition('a.kind', 5, '<')
    ->distinct()
    ->execute()
    ->fetchCol();
  return user_load_multiple($ids);
}

function ces_komunitin_api_social_members_read($exchange, $user) {
  if (ces_bank_user_access('view', $user)) {
    return new Member($user, $exchange);
  }
  else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_members_read_collection($exchange, array $users) {
  if (ces_bank_access('view', 'exchange accounts', $exchange['id'])) {
    $members = [];
    foreach ($users as $user) {
      $members[] = ces_komunitin_api_social_members_read($exchange, $user);
    }
    return $members;
  }
  else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_needs_load($exchange, $code) {
  $need = _ces_komunitin_api_social_offerneeds_load($code, 'need');
  return $need;
}
function ces_komunitin_api_social_needs_load_collection($exchange, $filters, $sorts) {
  return _ces_komunitin_api_social_offerneeds_load_collection($exchange, $filters, $sorts, 'need');
}

function ces_komunitin_api_social_needs_read($exchange, $need) {
  if (ces_offerwant_access('view', $need)) {
    return new Need($need, $exchange);
  }
  else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}
function ces_komunitin_api_social_needs_read_collection($exchange, $needs)
{
  // General access check.
  if (ces_bank_access('view', 'exchange details', $exchange['id'])) {
    $result = [];
    foreach ($needs as $need) {
      // Resource by resource access check.
      if (ces_offerwant_access('view', $need)) {
        $result[] = new Need($need, $exchange);
      }
    }
    return $result;
  }
  else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_offers_read($exchange, $offer){
  if (ces_offerwant_access('view', $offer)) {
    return new Offer($offer, $exchange);
  }
  else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_offers_read_collection($exchange, $offers) {
  // General access check.
  if (ces_bank_access('view', 'exchange details', $exchange['id'])) {
    $result = [];
    foreach ($offers as $offer) {
      // Resource by resource access check.
      if (ces_offerwant_access('view', $offer)) {
        $result[] = new Offer($offer, $exchange);
      }
    }
    return $result;
  } else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_offers_load($exchange, $code) {
  $offer = _ces_komunitin_api_social_offerneeds_load($code, 'offer');
  return $offer;
}

function ces_komunitin_api_social_offers_load_collection($exchange, $filters, $sorts) {
  return _ces_komunitin_api_social_offerneeds_load_collection($exchange, $filters, $sorts, 'offer');
}

function _ces_komunitin_api_social_offerneeds_load_collection($exchange, $filters, $sorts, $type) {
  $type = ($type == 'offer') ? 'offer' : 'want';

  // Get list of category ids in exchange.
  $categories = ces_offerswants_get_categories_ids($type, $exchange['id']);

  // Get the list of object ids.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'ces_offerwant');
  $query->entityCondition('bundle', $type);
  $query->propertyCondition('category', $categories, 'IN');
  // Active objects and not expired.
  $query->propertyCondition('state', 1);
  $query->propertyCondition('expire', REQUEST_TIME, '>');
  $result = $query->execute();
  // Load full entities.
  if (isset($result['ces_offerwant'])) {
    $ids = array_keys($result['ces_offerwant']);
    $offers = ces_offerwant_load_multiple($ids);
    return $offers;
  } else {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
  }
}

function _ces_komunitin_api_social_offerneeds_load($code, $type = 'offer') {
  $type = ($type == 'offer') ? 'offer' : 'want';
  $offers = ces_offerwant_load_multiple([], ['code' => $code, 'type' => $type]);
  if (!$offers) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
  }
  return reset($offers);
}

