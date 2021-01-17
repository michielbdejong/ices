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
  $bank = new CesBank();
  $account = $bank->getAccountByName($code);
  $uid = _ces_komunitin_api_social_account_user_id($account['id']);
  $account['user'] = user_load($uid);
  return $account;
}
function ces_komunitin_api_social_members_load_collection($exchange, $filters, $sorts) {
  // Get list of uids directly from database call.
  $accounts = db_select('ces_account', 'a')
    ->fields('a')
    ->condition('a.exchange', $exchange['id'])
    // discard inactive accounts (new, blocked or closed).
    ->condition('a.state', 1)
    // discard virtual accounts
    ->condition('a.kind', 5, '<')
    ->execute()
    ->fetchAll(PDO::FETCH_ASSOC);
  $aids = array_map(function ($account) { return $account['id']; }, $accounts);
  $uids = _ces_komunitin_api_social_account_user_id_multiple($aids);
  $users = user_load_multiple($uids);
  array_walk($accounts, function (&$value, $key) use ($users, $uids) {
    $value['user'] = $users[$uids[$value['id']]];
  }, $users);

  return $accounts;
}

function ces_komunitin_api_social_members_read($exchange, $member) {
  if (ces_bank_user_access('view', $member['user'])) {
    return new Member($member, new Group($exchange));
  }
  else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_members_read_collection($exchange, array $members) {
  if (ces_bank_access('view', 'exchange accounts', $exchange['id'])) {
    $group = new Group($exchange);
    $result = [];
    foreach ($members as $member) {
      if (ces_bank_user_access('view', $member['user'])) {
        $result[] = new Member($member, $group);
      }
    }
    return $result;
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

/*
 * IntegralCES and Komunitin have slightly different architecture for users and
 * accounts. IntegralCES has nxn relation between users and accounts, where offers
 * and wants belong to users and transactions belong to members.
 * Komunitin is simpler and defines a member with 1x1 relation to accounts. Thus
 * in order to make the mapping between the two systems we do:
 *
 * 1) We define one Komunitin member for each IntegralCES account.
 * 2) The IntegralCES user assigned to the Komunitin member is, among the users
 * related to the account, the one with the first record in table ces_accountuser.
 * 3) That means that a single IntegralCES user may be related to several members,
 * and also, a user need not to have an assigned member.
 * 4) For all objects (offers, needs) linked to an IntegralCES user, the related
 * Komunitin member is the first account in terms of ces_accountuser such that
 * the account is mapped to the user and the exchange of the account is the same
 * as the exchange of the object. If no such member exists, then the objects are
 * not linked to any Komunitin member.
 */
function _ces_komunitin_api_social_account_user_id($account_id) {
  return db_select('ces_accountuser', 'au')
    ->fields('au', ['id', 'user'])
    ->condition('au.account', $account_id)
    ->condition('au.privilege', 0)
    ->orderBy('au.id')
    ->range(0,1)
    ->execute()
    ->fetchField(1);
}
/**
 * @see _ces_komunitin_api_social_account_user_id
 * @return int|null
 */
function _ces_komunitin_api_social_user_account_id($uid, $exchangeid) {
  $result = db_query('SELECT au.id, au.account, au.user
    FROM {ces_accountuser} au
    JOIN {ces_accountuser} au2 ON au.account = au2.account
    JOIN {ces_account} a ON au2.account = a.id
    WHERE a.exchange = :exchangeid
    AND au2.user = :uid
    AND au.privilege = 0
    AND au2.privilege = 0',
    [':uid' => $uid, ':exchangeid' => $exchangeid]
  )->fetchAll();
  //compute the user associated to each account.
  $accounts = [];
  foreach ($result as $row) {
    if (!isset($accounts[$row->account]) || $accounts[$row->account]->id > $row->id) {
      $accounts[$row->account] = $row;
    }
  }
  // get the account that satisfies the criteria (if any).
  $account = FALSE;
  foreach ($accounts as $row) {
    if ($row->user == $uid && ($account == FALSE || $account->id > $row->id)) {
      $account = $row;
    }
  }
  return $account ? $account->account : NULL;
}
/**
 * Multiple version of _ces_komunitin_api_social_account_user_id. Returns an array
 * indexed by $aids with $uid as value.
 */
function _ces_komunitin_api_social_account_user_id_multiple(array $aids) {
  $result = db_query('SELECT id, user, account from {ces_accountuser}
    WHERE account IN (:aids)
    AND privilege = 0', [':aids' => $aids])
  ->fetchAll();
  $rows = [];
  foreach($result as $row) {
    if (!isset($rows[$row->account]) || $rows[$row->account]->id > $row->id) {
      $rows[$row->account] = $row;
    }
  }
  $uids = [];
  foreach ($aids as $aid) {
    $uids[$aid] = isset($rows[$aid]) ? $rows[$aid]->user : NULL;
  }
  return $uids;
}


function _ces_komunitin_api_social_user_member($uid, $exchangeid) {
  $id = _ces_komunitin_api_social_user_account_id($uid, $exchangeid);
  if ($id == NULL) return NULL;

  $bank = new CesBank();
  $account = $bank->getAccount($id);
  $account['user'] = user_load($uid);
  $account['uid'] = $uid;
  return $account;
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
    $group = new Group($exchange);
    $category = new Category(ces_category_load($need->category), $group);
    $member = new Member(_ces_komunitin_api_social_user_member($need->user, $exchange['id']), $group);
    return new Need($need, $member, $group, $category);
  }
  else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}
function ces_komunitin_api_social_needs_read_collection($exchange, $needs)
{
  // General access check.
  if (ces_bank_access('view', 'exchange details', $exchange['id'])) {
    $group = new Group($exchange);
    $result = [];
    foreach ($needs as $need) {
      // Resource by resource access check.
      if (ces_offerwant_access('view', $need)) {
        $category = new Category(ces_category_load($need->category), $group);
        $member = new Member(_ces_komunitin_api_social_user_member($need->user, $exchange['id']), $group);
        $result[] =  new Need($need, $member, $group, $category);
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
    $group = new Group($exchange);
    $category = new Category(ces_category_load($offer->category), $group);
    $member = new Member(_ces_komunitin_api_social_user_member($offer->user, $exchange['id']), $group);
    return new Offer($offer, $member, $group, $category);
  }
  else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_offers_read_collection($exchange, $offers) {
  // General access check.
  if (ces_bank_access('view', 'exchange details', $exchange['id'])) {
    $group = new Group($exchange);
    $result = [];
    foreach ($offers as $offer) {
      // Resource by resource access check.
      if (ces_offerwant_access('view', $offer)) {
        $category = new Category(ces_category_load($offer->category), $group);
        $member = new Member(_ces_komunitin_api_social_user_member($offer->user, $exchange['id']), $group);
        $result[] = new Offer($offer, $member, $group, $category);
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
        // Check whether this account is the default account for any user.
        $uid = _ces_komunitin_api_social_account_user_id($obj->id);
        $aid = _ces_komunitin_api_social_user_account_id($uid, $exchange['id']);
        if ($aid == $obj->id) {
          $query->propertyCondition('user', $uid);
        } else {
          // This member is related to an account that is not the default account
          // for any user, so it doesn't have related resources.
          return [];
        }
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
function ces_komunitin_api_social_account_needs_count($account_id, $exchange_id) {
  return _ces_komunitin_api_social_account_offersneeds_count($account_id, $exchange_id, 'need');
}
function ces_komunitin_api_social_account_offers_count($account_id, $exchange_id) {
  return _ces_komunitin_api_social_account_offersneeds_count($account_id, $exchange_id, 'offer');
}
function _ces_komunitin_api_social_category_offersneeds_count($category, $type) {
  $type = $type == 'offer' ? 'offer' : 'want';
  $count = db_select('ces_offerwant', 'o')
    ->fields('o', ['id'])
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
function _ces_komunitin_api_social_account_offersneeds_count($account_id, $exchange_id, $type) {
  $type = $type == 'offer' ? 'offer' : 'want';
  $uid = _ces_komunitin_api_social_account_user_id($account_id);
  $aid = _ces_komunitin_api_social_user_account_id($uid, $exchange_id);
  if ($aid == $account_id) {
    $query = db_select('ces_offerwant', 'o')
      ->fields('o', ['id']);
    $query->join('ces_category', 'c', 'o.category = c.id');
    $count = $query
    ->condition('c.exchange', $exchange_id)
    ->condition('o.user', $uid)
    ->condition('o.type', $type)
    ->condition('o.state', 1)
    ->condition('o.expire', REQUEST_TIME, '>')
    ->countQuery()
    ->execute()
    ->fetchField();
    return $count;
  } else {
    return 0;
  }
}

