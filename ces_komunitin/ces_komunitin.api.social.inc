<?php

class ResourceTypes {
  const USER = "user";
  const USER_SETTINGS = "user-settings";
  const GROUP = "group";
  const MEMBER = "member";
  const NEED = "need";
  const OFFER = "offer";
  const CATEGORY = "category";
  const FILE = "file";

  // Contact Types
  const EMAIL = "email";
  const PHONE = "phone";
  const WEBSITE = "website";
  const WHATSAPP = "whatsapp";
  const TELEGRAM = "telegram";
  const INSTAGRAM = "instagram";
  const FACEBOOK = "facebook";
  const TWITTER = "twitter";

  const ALL_TYPES = [self::USER, self::USER_SETTINGS, self::GROUP, self::MEMBER,
    self::NEED, self::OFFER, self::CATEGORY, self::FILE,
    self::EMAIL, self::PHONE, self::WEBSITE, self::WHATSAPP, self::TELEGRAM,
    self::INSTAGRAM, self::FACEBOOK, self::TWITTER];
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
  if (!ces_komunitin_api_social_is_uuid($uuid)) {
    return FALSE;
  }
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

function ces_komunitin_api_social_is_uuid($uuid) {
  if (strlen($uuid) != 36) {
    return FALSE;
  }
  $uuid = strtolower($uuid);
  $parts = explode('-', $uuid);
  if (count($parts) != 5) {
    return FALSE;
  }
  if (strlen($parts[0]) != 8 || strlen($parts[1]) != 4 || strlen($parts[2]) != 4 || strlen($parts[3]) != 4 || strlen($parts[4]) != 12) {
    return FALSE;
  }
  if (@hex2bin(implode('', $parts)) === FALSE) {
    return FALSE;
  }
  return TRUE;
}

function ces_komunitin_api_social_groups_read($exchange) {
  return new Group($exchange);
}

function ces_komunitin_api_social_groups_load_collection($exchange, $filters, $sorts, $pageAfter, $pageSize) {

  $query = db_select('ces_exchange', 'e')
    ->fields('e')
    ->condition('e.state', 1);

  foreach ($filters as $field => $values) {
    if ($field == 'search') {
      $keys = explode(' ', reset($values));
      foreach ($keys as $key) {
        $escaped = db_like($key);
        $query->condition(db_or()
          ->condition('e.name', '%' . $escaped . '%', 'LIKE')
          ->condition('e.code', '%' . $escaped . '%', 'LIKE')
          ->condition('e.shortname', '%' . $escaped . '%', 'LIKE')
          ->condition('e.currencyname', '%' . $escaped . '%', 'LIKE')
          ->condition('e.currenciesname', '%' . $escaped . '%', 'LIKE')
          ->condition('e.town', '%' . $escaped . '%', 'LIKE')
       );
      }
    }
  }
  $query->orderBy('e.name', 'ASC')
    ->range($pageAfter, $pageSize);

  $exchanges = $query->execute()->fetchAll(PDO::FETCH_ASSOC);
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
  // No need to check access since the user is the current user.
  global $user;
  $exchange = ces_bank_get_current_exchange();
  return new User($user, $exchange);
}

function ces_komunitin_api_social_users_update($body) {
  // No need to check access since the user is the current user.
  global $user;
  if (!isset($body->data)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $data = $body->data;
  if (isset($data->attributes)) {
    // Providing password is required for updating email or password.
    require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
    $password = $data->attributes->password;
    if (!user_check_password($password, $user)) {
      ces_komunitin_api_send_error(KomunitinApiError::INVALID_PASSWORD);
    }
    $edit = [];
    // Update email
    if (isset($data->attributes->email)) {
      $edit['mail'] = $data->attributes->email;
    }
    // Update password
    if (isset($data->attributes->newPassword)) {
      $edit['pass'] = $data->attributes->newPassword;
    }
    // Update user in database.
    if (!empty($edit)) {
      $user = user_save($user, $edit);
    }
  }
  return ces_komunitin_api_social_users_read();
}

function ces_komunitin_api_social_users_settings_read() {
  // No need to check access since the user is the current user.
  global $user;
  $exchange = ces_bank_get_current_exchange();
  return new UserSettings($user, $exchange);
}

function ces_komunitin_api_social_users_settings_update($body) {
  // No need to check access since the user is the current user.
  global $user;
  $exchange = ces_bank_get_current_exchange();
  module_load_include('module', 'ces_user');
  $obj = $body->data->attributes;
  $settings = [];
  if (isset($obj->language)) {
    $settings['language'] = $obj->language;
  }
  if (isset($obj->notifications)) {
    $settings['notifications'] = (array) $obj->notifications;
  }
  ces_user_set_settings($user, $settings);
  return new UserSettings($user, $exchange);
}

function ces_komunitin_api_social_members_load($exchange, $code) {
  // Load by id.
  $bank = new CesBank();
  if (ces_komunitin_api_social_is_uuid($code)) {
    $id = ces_komunitin_api_social_get_id($code);
    $account = $bank->getAccount($id->id);
  } else {
    // Load by account name.
    $account = $bank->getAccountByName($code);
  }
  $uid = _ces_komunitin_api_social_account_user_id($account['id']);
  $account['user'] = user_load($uid);
  return $account;
}

function ces_komunitin_api_social_members_load_collection($exchange, $filters, $sorts, $pageAfter, $pageSize) {
  $query = db_select('ces_account', 'a')
    ->fields('a')
    ->condition('a.exchange', $exchange['id'])
    // discard inactive accounts (new, blocked or closed).
    ->condition('a.state', 1)
    // discard virtual accounts.
    ->condition('a.kind', 5, '<');
  $query->join('ces_accountuser', 'au', 'a.id = au.account');
  $query->condition('au.privilege', 0);
  // This is tricky indeed. We get the first ces_accountuser with given account id
  // and privilege = 0. In other words, we get the ces_accountuser such that there's
  // no other ces_accountuser with the same account privilege = 0 and lower id.
  $query->leftJoin('ces_accountuser', 'au2', 'au2.account = au.account AND au2.id < au.id AND au2.privilege = 0');
  $query->isNull('au2.account');
  // Join users table
  $query->join('users', 'u', 'u.uid = au.user');
  $query->fields('u', array('uid'));

  foreach($filters as $field => $values) {
    if ($field == 'search') {
      // Add more tables to the select since they have fields to search on.
      // Using left joins since some of the fields may not exist but still find
      // the term in another one.
      $query->leftJoin('field_data_ces_firstname', 'd1', 'd1.entity_id = u.uid');
      $query->leftJoin('field_data_ces_surname','d2', 'd2.entity_id = u.uid');
      $keys = explode(' ', reset($values));
      foreach ($keys as $key) {
        $like = '%' . db_like($key) . '%';
        $query->condition(db_or()
          ->condition('a.name', $like, 'LIKE')
          ->condition('d1.ces_firstname_value', $like, 'LIKE')
          ->condition('d2.ces_surname_value', $like, 'LIKE')
        );
      }
    }
    else if ($field == 'account') {
      if (count($values) == 1) {
        $query->condition('a.uuid', $values[0]);
      }
      else  {
        $query->condition('a.uuid', $values, 'IN');
      }
    }
    else {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED, "Filtering other than 'account' or 'search' is not supported.");
    }
  }

  $query
    // Pagination.
    ->range($pageAfter, $pageSize)
    //default order
    ->orderBy('a.name', 'ASC');

  $accounts = $query->execute()->fetchAll(PDO::FETCH_ASSOC);
  $uids = array_map(function($account) { return $account['uid']; }, $accounts);
  $users = user_load_multiple($uids);
  array_walk($accounts, function(&$value, $key) use ($users) {
    $value['user'] = $users[$value['uid']];
  });

  return $accounts;
}

function ces_komunitin_api_social_members_read($exchange, $member) {
  if (ces_komunitin_api_client_access('komunitin_social_read_all') ||
    ces_bank_user_access('view', $member['user'])) {

    return new Member($member, new Group($exchange));
  }
  else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_members_read_collection($exchange, array $members) {
  $client_access = ces_komunitin_api_client_access('komunitin_social_read_all');
  if ($client_access || ces_bank_access('view', 'exchange accounts', $exchange['id'])) {
    $group = new Group($exchange);
    $result = [];
    foreach ($members as $member) {
      if ($client_access || ces_bank_user_access('view', $member['user'])) {
        $result[] = new Member($member, $group);
      }
    }
    return $result;
  }
  else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_members_update($exchange, $member, $body) {
  if (!ces_bank_access('edit', 'account', $member['id'])) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
  if (!$body->data) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $data = $body->data;
  $user = $member['user'];
  if (!empty($data->attributes)) {
    $attributes = $data->attributes;
    // Update ces_fields
    $fields = [];
    // Name
    if (!empty($attributes->name)) {
      // Integralces has both firstname and surname, but Komunitin has only
      // the name. Since for organizations only the firstname is used, we match
      // the whole komunitin name to the Integralces firstname.
      $fields['ces_firstname'] = $attributes->name;
      $fields['ces_surname'] = '';
    }
    // Bio
    if (!empty($attributes->description)) {
      $fields['ces_description'] = $attributes->description;
    }
    // Address
    if (!empty($attributes->address)) {
      $address = $attributes->address->streetAddress;
      $fields['ces_address'] = $address;
      $town = $attributes->address->addressLocality;
      $fields['ces_town'] = $town;
      $postcode = $attributes->address->postalCode;
      $fields['ces_postcode'] = $postcode;
      $region = $attributes->address->addressRegion;
      $fields['ces_region'] = $region;
      $country = $attributes->address->addressCountry;
      $fields['ces_country'] = $country;
    }
    // use correct format for field api.
    $edit = [];
    foreach($fields as $name => $value) {
      $edit[$name] = [LANGUAGE_NONE => [['value' => $value]]];
    }

    // Location
    if (!empty($attributes->location)) {
      $edit['ces_geolocation'] = [LANGUAGE_NONE => [
        _ces_user_geolocation_field_value([
          'lat' => $attributes->location->coordinates[1],
          'lng' => $attributes->location->coordinates[0],
        ])
      ]];
    }


    // Image
    if (!empty($attributes->image)) {
      // user_save takes care of the file usage.
      $file = _ces_komunitin_api_social_get_file_by_url($attributes->image);
      if ($file == NULL) {
        ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Invalid image');
      } else {
        $edit['picture'] = $file;
      }
    }
    // Update user in database.
    $user = user_save($user, $edit);
  }
  if (!empty($data->relationships) && !empty($data->relationships->contacts)) {
    if (!isset($body->included) || !isset($data->relationships->contacts->data)) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Missing included or related contacts.");
    }
    $included = $body->included;
    $contacts = [];
    foreach ($data->relationships->contacts->data as $related) {
      // Get the id of the related contact.
      $id = $related->id;
      // Find the id in the included array.
      $contact = NULL;
      foreach ($included as $item) {
        if ($item->type == "contacts" &&  $item->id == $id) {
          $contact = $item;
          break;
        }
      }
      // We could support not modified contacts that are present in the relationship
      // but not in the included array, but it is not necessary for the actual
      // app usage of the API.
      if ($contact == NULL) {
        ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Contact not found in included.");
      }
      $contacts[$contact->attributes->type] = $contact->attributes->name;
    }
    // Update contacts.
    ces_user_set_contacts($user, $contacts);
  }
  // Update user object inside $member so we return an updated resource.
  $member['user'] = user_load($user->uid);

  return ces_komunitin_api_social_members_read($exchange, $member);
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
 * and wants belong to users and transactions belong to accounts.
 * Komunitin is simpler and defines a member with 1x1 relation to accounts. Thus
 * in order to make the mapping between the two systems we do:
 *
 * 1) We define one Komunitin member for each IntegralCES account.
 * 2) The IntegralCES user assigned to the Komunitin member is, among the users
 * related to the account, the one with the first record in table ces_accountuser
 * with privilege = 0.
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
 * Returns account from user & exchange
 */
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
function ces_komunitin_api_social_needs_load_collection($exchange, $filters, $sorts, $pageAfter, $pageSize) {
  return _ces_komunitin_api_social_offerneeds_load_collection($exchange, $filters, $sorts, 'need', $pageAfter, $pageSize);
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

/**
 * Return a file object or null.
 */
function _ces_komunitin_api_social_get_file_by_url($url) {
  $filename = basename(rawurldecode($url));
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'file')
    ->propertyCondition('uri', "%/". db_like($filename), 'LIKE');
  $result = $query->execute();
  if (isset($result['file']) && count($result['file']) > 0) {
    $resultFile = reset($result['file']);
    $file = file_load($resultFile->fid);
    return $file;
  } else {
    return NULL;
  }
}
/**
 * Validate fields common to offers and needs in create and update operations.
 */
function _ces_komunitin_api_social_offersneeds_validate($exchange, $body, $new, $type) {
  $validated = [];
  if (!isset($body->data)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $data = $body->data;

  // type.
  $resourceType = $type === 'offer' ? 'offers' : 'needs';
  if ($data->type !== $resourceType) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Bad type');
  }

  if ($new && isset($data->id)) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN, 'Id must not be set');
  }

  // attributes.
  if (!isset($data->attributes)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Attributes must be set');
  }
  $attributes = $data->attributes;

  // content
  if ($new && !isset($attributes->content)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Content must be set');
  }
  if (isset($attributes->content)) {
    if ((!is_string($attributes->content) || strlen(trim($attributes->content)) < 10)) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Invalid content');
    }
    $validated['body'] = trim($attributes->content);
  }

  // expires
  if (isset($attributes->expires)) {
    $date = DateTimeImmutable::createFromFormat(DateTime::RFC3339_EXTENDED, $attributes->expires);
    if (!$date) {
      $date = DateTimeImmutable::createFromFormat(DateTime::RFC3339, $attributes->expires);
    }
    if (!$date) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Invalid expires format');
    }
    $expires = $date->getTimestamp();
    // Only force future expire date for new needs.
    if ($new && ($expires < REQUEST_TIME)) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Invalid expires range');
    }
    $validated['expire'] = $expires;
  }

  //state
  if (isset($attributes->state)) {
    $states = ['hidden', 'published'];
    if (!in_array($attributes->state, $states)) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Invalid state');
    }
    $validated['state'] = array_search($attributes->state, $states);
  }

  // images
  $images = [];
  $uploaded = [];
  if (isset($attributes->images) && is_array($attributes->images)) {
    foreach ($attributes->images as $url) {
      $file = _ces_komunitin_api_social_get_file_by_url($url);
      if ($file == NULL) {
        ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Invalid image ' . $url);
      } else {
        if (!$file->status) {
          $info = image_get_info($file->uri);
          if ($info) {
            $directory = file_default_scheme() . '://' . variable_get('ces_offerswants_picture_path', 'ces_offerswants_pictures');
            // Prepare the pictures directory.
            file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
            $destination = file_stream_wrapper_uri_normalize($directory . '/' . sha1_file($file->uri) . '.' . $info['extension']);
            if (($file = file_move($file, $destination, FILE_EXISTS_RENAME)) !== FALSE) {
              $file->status = FILE_STATUS_PERMANENT;
              $file = file_save($file);
              $uploaded[] = $file;
            }
          }
        }
        $images[] = $file->fid;
      }
    }
  }
  $validated['image'] = implode(',',$images);
  $validated['uploaded'] = $uploaded;

  // category
  if ($new && (!isset($data->relationships) || !isset($data->relationships->category) || !isset($data->relationships->category->data))) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Category must be set');
  }
  if (isset($data->relationships) && isset($data->relationships->category)) {
    $categoryId = ces_komunitin_api_social_get_id($data->relationships->category->data->id);
    $category = ces_category_load($categoryId->id);
    if (!$category || $category->exchange !== $exchange['id']) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Invalid category' );
    }
    $validated['category'] = $category->id;
  }

  // member
  if ($new && (!isset($data->relationships)||!isset($data->relationships->member) || !isset($data->relationships->member->data))) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Member must be set');
  }
  if (isset($data->relationships) && isset($data->relationships->member)) {
    $obj = ces_komunitin_api_social_get_id($data->relationships->member->data->id);
    $uid = _ces_komunitin_api_social_account_user_id($obj->id);
    $validated['user'] = $uid;
  }

  // Extra fields for offers.
  if ($type == 'offer') {
    if ($new && !isset($attributes->name) || isset($attributes->name) && trim($attributes->name) == '') {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Title must be set');
    }
    if (isset($attributes->name)) {
      $validated['title'] = trim($attributes->name);
    }
    // price
    if ($new && !isset($attributes->price) || isset($attributes->price) && trim($attributes->price) == '') {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Price must be set');
    }
    if (isset($attributes->price)) {
      $validated['price'] = trim($attributes->price);
    }
  }

  return $validated;
}

function _ces_komunitin_api_social_offersneeds_create($exchange, $body, $type) {
  // Access.
  if (!ces_offerwant_access('add')) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
  $data = _ces_komunitin_api_social_offersneeds_validate($exchange, $body, TRUE, $type);

  if ($type == 'need') {
    $data['title'] = ces_komunitin_api_social_create_title($data['body'], 50);
  }

  $defaultExpire = REQUEST_TIME + ($type == 'need' ? 7 : 365) * 60*60*24*7; // 7 days for needs, 1 year for offers.

  $o = (object)[
    'type' => $type == 'offer' ? 'offer' : 'want',
    'title' => $data['title'],
    'body' => $data['body'],
    'keywords' => '',
    'state' => $data['state'],
    'image' => $data['image'],
    'created' => REQUEST_TIME,
    'modified' => REQUEST_TIME,
    'expire' => isset($data['expire']) ? $data['expire'] : $defaultExpire,
    'user' => $data['user'],
    'category' => $data['category']
  ];
  // price
  if (isset($data['price'])) {
    $o->ces_offer_rate = array(LANGUAGE_NONE => array(array('value' => $data['price'])));
  }

  // Check that user has enough privileges.
  if (!ces_offerwant_access('edit', $o)) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }

  // Create the need.
  $o = ces_offerwant_save($o);

  // Mark uploaded files.
  foreach ($data['uploaded'] as $file) {
    file_usage_add($file, 'ces_offerswants', 'ces_offerwant', $o->id);
  }

  return $o;
}

function _ces_komunitin_api_social_offersneeds_update($exchange, $o, $body, $type) {
  if (!ces_offerwant_access('edit', $o)) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
  $data = _ces_komunitin_api_social_offersneeds_validate($exchange, $body, FALSE, $type);
  if (isset($data['body'])) {
    $o->body = $data['body'];
  }
  if (isset($data['state'])) {
    $o->state = $data['state'];
  }
  if (isset($data['expire'])) {
    $o->expire = $data['expire'];
  }
  if (isset($data['category'])) {
    $o->category = $data['category'];
  }
  if (isset($data['user']) && ($data['user'] !== $o->user)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'User cannot be changed');
  }
  $o->modified = REQUEST_TIME;

  if (isset($data['image'])) {
    // Delete old files.
    $oldFiles = strlen($o->image) > 0 ? explode(',', $o->image) : [];
    $newFiles = strlen($data['image']) > 0 ? explode(',', $data['image']) : [];
    $deletedFiles = array_diff($oldFiles, $newFiles);
    foreach ($deletedFiles as $fid) {
      if (($file = file_load($fid)) !== FALSE) {
        file_usage_delete($file, 'ces_offerswants', 'ces_offerwant', $o->id);
        file_delete($file);
      }
    }
    // Add new files.
    $o->image = $data['image'];
    foreach ($data['uploaded'] as $file) {
      file_usage_add($file, 'ces_offerswants', 'ces_offerwant', $o->id);
    }
  }

  // type specific data
  if ($type == 'need') {
    if (isset($data['body'])) {
      $o->title = ces_komunitin_api_social_create_title($data['body'], 50);
    }
  } else if ($type == 'offer') {
    if (isset($data['title'])) {
      $o->title = $data['title'];
    }
    if (isset($data['price'])) {
      $o->ces_offer_rate = array(LANGUAGE_NONE => array(array('value' => $data['price'])));
    }
  }

  $o = ces_offerwant_save($o);
  return $o;
}


function _ces_komunitin_api_social_offersneeds_delete($exchange, $o) {
  if (ces_offerwant_access('edit', $o)) {
    ces_offerwant_delete($o->id);
    return TRUE; // Send 204 No Content.
  }
  else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_needs_create($exchange, $body) {
  $need = _ces_komunitin_api_social_offersneeds_create($exchange, $body, 'need');
  // Return the created need.
  return ces_komunitin_api_social_needs_read($exchange, $need);
}

function ces_komunitin_api_social_needs_update($exchange, $need, $body) {
  $need = _ces_komunitin_api_social_offersneeds_update($exchange, $need, $body, 'need');
  return ces_komunitin_api_social_needs_read($exchange, $need);
}

function ces_komunitin_api_social_needs_delete($exchange, $need) {
  return _ces_komunitin_api_social_offersneeds_delete($exchange, $need);
}

function ces_komunitin_api_social_offers_load($exchange, $code) {
  $offer = _ces_komunitin_api_social_offerneeds_load($code, 'offer');
  return $offer;
}

function ces_komunitin_api_social_offers_load_collection($exchange, $filters, $sorts, $pageAfter, $pageSize) {
  return _ces_komunitin_api_social_offerneeds_load_collection($exchange, $filters, $sorts, 'offer', $pageAfter, $pageSize);
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

function ces_komunitin_api_social_offers_create($exchange, $body) {
  $offer = _ces_komunitin_api_social_offersneeds_create($exchange, $body, 'offer');
  // Return the created offer.
  return ces_komunitin_api_social_offers_read($exchange, $offer);
}

function ces_komunitin_api_social_offers_update($exchange, $offer, $body) {
  $offer = _ces_komunitin_api_social_offersneeds_update($exchange, $offer, $body, 'offer');
  return ces_komunitin_api_social_offers_read($exchange, $offer);
}

function ces_komunitin_api_social_offers_delete($exchange, $offer) {
  return _ces_komunitin_api_social_offersneeds_delete($exchange, $offer);
}

function _ces_komunitin_api_social_offerneeds_load_collection($exchange, $filters, $sorts, $type, $pageAfter, $pageSize) {
  $type = ($type == 'offer') ? 'offer' : 'want';

  // Get the list of object ids.
  $query = db_select('ces_offerwant', 'o')
    ->fields('o', array('id'))
    ->condition('o.type', $type);
  // Filter by exchange.
  $categories = ces_offerswants_get_categories_ids(NULL, $exchange['id']);
  $query->condition('o.category', $categories, 'IN');

  // Expired
  $expired = ['false'];
  if (isset($filters['expired'])) {
    $expired = $filters['expired'];
    unset($filters['expired']);
  }
  if (count($expired) == 2 && in_array('true', $expired) && in_array('false', $expired)) {
      // no need to add filter
  } else if (in_array('true', $expired)) {
    $query->condition('o.expire', REQUEST_TIME, '<=');
  } else if (in_array('false', $expired)) {
    $query->condition('o.expire', REQUEST_TIME, '>');
  }

  // State
  $state = ['published'];
  if (isset($filters['state'])) {
    $state = $filters['state'];
    unset($filters['state']);
  }
  if (count($state) == 2 && in_array('published', $state) && in_array('hidden', $state)) {
    // no need to add filter
  } else if (in_array('published', $state)) {
    $query->condition('o.state', 1);
  } else if (in_array('hidden', $state)) {
    $query->condition('o.state', 0);
  }

  // Apply API filtering.
  foreach ($filters as $field => $values) {
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
          $query->condition('o.user', $uid);
        } else {
          // This member is related to an account that is not the default account
          // for any user, so it doesn't have related resources.
          return [];
        }
      } else {
        ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Member not found.");
      }
    } else if ($field == 'search') {
      $keys = explode(' ', reset($values));
      foreach ($keys as $key) {
        $escaped = db_like($key);
        $query->condition(db_or()
          ->condition('o.title', '%' . $escaped . '%', 'LIKE')
          ->condition('o.keywords', '%' . $escaped . '%', 'LIKE')
          ->condition('o.body', '%' . $escaped . '%', 'LIKE')
        );
      }
    } else {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED, "Filtering other than 'member' or 'search' is not supported.");
    }
  }
  // Default order by
  $query->orderBy('o.modified', 'DESC');

  // Pagination
  $query->range($pageAfter, $pageSize);

  $ids = $query->execute()->fetchCol();
  $offers = ces_offerwant_load_multiple($ids);
  return $offers;
}

function _ces_komunitin_api_social_offerneeds_load($code, $type = 'offer') {
  $type = ($type == 'offer') ? 'offer' : 'want';
  if (ces_komunitin_api_social_is_uuid($code)) {
    // Load by ID.
    $id = ces_komunitin_api_social_get_id($code);
    $offer = ces_offerwant_load($id->id);
    if (!$offer || $offer->type != $type) {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
    }
    return $offer;
  } else {
    // Load by code.
    $offers = ces_offerwant_load_multiple([], ['code' => $code, 'type' => $type]);
    if (!$offers) {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
    }
    return reset($offers);
  }
}

function ces_komunitin_api_social_members_count($exchange) {
  $count = db_select('ces_account')
    ->fields(NULL, array('id'))
    ->condition('exchange', $exchange['id'])
    ->condition('kind', 4, '<')
    ->condition('state', 1)
    ->countQuery()
    ->execute()
    ->fetchField();
  return intval($count);
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
/**
 * Counts the number of offers or needs (not expired nor hidden) for a given category.
 */
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
  return intval($count);
}
/**
 * Counts the number of offers or needs (not expired nor hidden) for a given exchange.
 */
function _ces_komunitin_api_social_offersneeds_count($exchange, $type) {
  $type = $type == 'offer' ? 'offer' : 'want';
  $query = db_select('ces_offerwant', 'o');
  $query->join('ces_category', 'c', 'o.category = c.id');
  $count = $query->fields('o', array('id'))
    ->condition('c.exchange', $exchange['id'])
    ->condition('o.type', $type)
    ->condition('o.state', 1)
    ->condition('o.expire', REQUEST_TIME, '>')
    ->countQuery()
    ->execute()
    ->fetchField();
  return intval($count);
}
/**
 * Counts the number of offers or needs (not expired nor hidden) for a given account.
 */
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
    return intval($count);
  } else {
    return 0;
  }
}
/**
 * Trim content to create a title for the content, using the first words and
 * never exceeding given character count.
 */
function ces_komunitin_api_social_create_title($text, $size = 50) {
  $text = trim($text, " .,");

  if (drupal_strlen($text) <= $size) {
    return $text;
  }

  $punctuation = [".", ",", ";", ":", "(", ")", "[", "]"];
  $all = implode("", $punctuation);

  foreach ($punctuation as $p) {
    if (($pos = strpos($text, $p)) !== FALSE) {
      $text = substr($text, 0, $pos);
      $text = trim($text, $all);
    }
    if (drupal_strlen($text) <= $size) {
      return $text;
    }
  }

  $text = substr($text, 0, $size);
  $pos = strrpos($text, " ");
  if ($pos !== FALSE) {
    $text = substr($text, 0, $pos);
  }
  return $text;
}

