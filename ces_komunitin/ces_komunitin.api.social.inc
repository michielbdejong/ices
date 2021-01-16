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
  try {
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
  } catch (Exception $e) {
    return FALSE;
  }
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

function ces_komunitin_api_social_categories_load_collection($exchange, $filters, $sorts) {
  // 3 means categories for both offers and wants.
  return ces_offerswants_get_categories(3, $exchange['id']);
}

function ces_komunitin_api_social_categories_read_collection($exchange, $categories) {
  if (ces_bank_access('view', 'exchange details', $exchange['id'])) {
    $group = new Group($exchange);
    $return = array_map(function ($category) use ($group) {
      return new Category($category, $group);
    }, $categories);
    return $return;
  } else {
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


  // Get the list of object ids.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'ces_offerwant');
  $query->entityCondition('bundle', $type);

  // Filter by exchange.
  $categories = ces_offerswants_get_categories_ids($type, $exchange['id']);
  $query->propertyCondition('category', $categories, 'IN');

  // Active objects and not expired.
  $query->propertyCondition('state', 1);
  $query->propertyCondition('expire', REQUEST_TIME, '>');

  // Apply API filtering.
  foreach($filters as $field => $values) {
    if ($field == 'member') {
      if (count($values) !== 1) {
        ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED, "Only one member filtering is supported.");
        return;
      }
      $value = reset($values);
      $obj = ces_komunitin_api_social_get_id($value);
      if ($obj->type == 'member' && $obj->id) {
        $query->propertyCondition('user', $obj->id);
      } else {
        ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Member not found.");
      }
    } else {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED, "Filtering other than 'filter[member]' is not supported.");
    }
  }

  $result = $query->execute();
  // Load full entities.
  if (isset($result['ces_offerwant'])) {
    $ids = array_keys($result['ces_offerwant']);
    $offers = ces_offerwant_load_multiple($ids);
    return $offers;
  } else {
    return [];
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

function ces_komunitin_api_social_members_count($exchange) {
  return db_select('ces_account')
    ->fields(NULL, array('id'))
    ->condition('exchange', $exchange['id'])
    ->condition('kind', 4, '<')
    ->condition('state', 1)
    ->countQuery()
    ->execute()
    ->fetchField();
}
function ces_komunitin_api_social_needs_count($exchange) {
  return _ces_komunitin_api_social_offersneeds_count($exchange, 'need');
}
function ces_komunitin_api_social_offers_count($exchange) {
  return _ces_komunitin_api_social_offersneeds_count($exchange, 'offer');
}
function ces_komunitin_api_social_category_offers_count($category) {
  return _ces_komunitin_api_social_category_offersneeds_count($category, 'offer');
}
function ces_komunitin_api_social_category_needs_count($category) {
  return _ces_komunitin_api_social_category_offersneeds_count($category, 'need');
}
function ces_komunitin_api_social_user_needs_count($user) {
  return _ces_komunitin_api_social_user_offersneeds_count($user, 'need');
}
function ces_komunitin_api_social_user_offers_count($user) {
  return _ces_komunitin_api_social_user_offersneeds_count($user, 'offer');
}
function _ces_komunitin_api_social_category_offersneeds_count($category, $type) {
  $type = $type == 'offer' ? 'offer' : 'want';
  $count = db_select('ces_offerwant', 'o')
    ->condition('o.category', $category->id)
    ->condition('o.type', $type)
    ->condition('o.state', 1)
    ->condition('o.expire', REQUEST_TIME, '>')
    ->countQuery()
    ->execute()
    ->fetchField();
    return $count;
}
function _ces_komunitin_api_social_offersneeds_count($exchange, $type) {
  $type = $type == 'offer' ? 'offer' : 'want';
  $query = db_select('ces_offerwant', 'o');
  $query->join('ces_category', 'c', 'o.category = c.id');
  $count = $query->fields('o', array('id'))
    ->condition('c.exchange', $exchange['id'])
    ->condition('c.context', $type == 'offer' ? [1,3] : [1,2], 'IN')
    ->condition('o.type', $type)
    ->condition('o.state', 1)
    ->condition('o.expire', REQUEST_TIME, '>')
    ->countQuery()
    ->execute()
    ->fetchField();
  return $count;
}
function _ces_komunitin_api_social_user_offersneeds_count($user, $type) {
  $type = $type == 'offer' ? 'offer' : 'want';
  $type = $type == 'offer' ? 'offer' : 'want';
  $count = db_select('ces_offerwant', 'o')
    ->condition('o.user', $user->uid)
    ->condition('o.type', $type)
    ->condition('o.state', 1)
    ->condition('o.expire', REQUEST_TIME, '>')
    ->countQuery()
    ->execute()
    ->fetchField();
  return $count;
}

