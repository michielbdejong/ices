<?php

class ResourceTypes {
  const USER = "user";
  const USER_SETTINGS = "user-settings";
  const GROUP = "group";
  const GROUP_SETTINGS = "group-settings";
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

  const ALL_TYPES = [self::USER, self::USER_SETTINGS, self::GROUP, self::GROUP_SETTINGS,
    self::MEMBER, self::NEED, self::OFFER, self::CATEGORY, self::FILE,
    self::EMAIL, self::PHONE, self::WEBSITE, self::WHATSAPP, self::TELEGRAM,
    self::INSTAGRAM, self::FACEBOOK, self::TWITTER];
}

function _ces_komunitin_api_social_type_code($type) {
  $type = substr($type, 0, 6);
  while (strlen($type) < 6) {
    $type .= ' ';
  }
  return $type;
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
  $type = _ces_komunitin_api_social_type_code($type);
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
    $type_code = substr($uuid, 0, 8) . substr($uuid, 9, 4);
    $type_code = hex2bin($type_code);
    foreach (ResourceTypes::ALL_TYPES as $item) {
      if (_ces_komunitin_api_social_type_code($item) == $type_code) {
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

  $query = db_select('ces_exchange', 'e')->fields('e');
  $only_active = true;

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
    } else if ($field == 'code') {
      $code = reset($values);
      $query->condition('e.code', $code);
      $only_active = false;
    }
  }

  if ($only_active) {
    $query->condition('e.state', 1);
  }

  $query->orderBy('e.name', 'ASC')
    ->range($pageAfter, $pageSize);

  $exchanges = $query->execute()->fetchAll(PDO::FETCH_ASSOC);

  // Maybe there's a more elegant way to overcome this by calling a load function in integralces.
  foreach ($exchanges as &$exchange) {
    $exchange['data'] = unserialize($exchange['data']);
  }
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

/**
 * Validate group create/update
 */
function _ces_komunitin_api_social_groups_validate($body, $new) {
  $edit = [];
  if (!isset($body->data)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $data = $body->data;
  if (!empty($data->attributes)) {
    $attrs = $data->attributes;
    // Name.
    if ($new && !isset($attrs->name)) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Name is required");
    }
    if (isset($attrs->name)) {
      if (strlen($attrs->name) < 3) {
        ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Name too short");
      }
      $edit['name'] = $attrs->name;
    }

    // Code.
    if ($new && !isset($attrs->code)) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Code is required");
    }
    if (isset($attrs->code)) {
      if (strlen($attrs->code) !== 4 || !preg_match('/^[a-zA-Z0-9]+$/', $attrs->code)) {
        ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Invalid code");
      }
      $edit['code'] = $attrs->code;
    }

    // Description.
    if ($new && !isset($attrs->description)) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Description is required");
    }
    if (isset($attrs->description)) {
      $edit['description'] = $attrs->description;
    }

    // Position
    if (isset($attrs->location) && !empty($attrs->location->coordinates)) {
      $edit['lng'] = $attrs->location->coordinates[0];
      $edit['lat'] = $attrs->location->coordinates[1];
    }

    // Address
    if (isset($attrs->address)) {
      $edit['town'] = $attrs->address->addressLocality;
      $edit['region'] = $attrs->address->addressRegion;
      $edit['country'] = $attrs->address->addressCountry;
    }

    // Image
    if (!empty($attrs->image)) {
      $files = _ces_komunitin_api_social_validate_files([$attrs->image], variable_get('ces_komunitin_picture_path', 'ces_komunitin_pictures'));
      $edit['image'] = $files['files'][0];
      $edit['uploaded'] = $files['uploaded'];
    }
  }

  return $edit;
}

function ces_komunitin_api_social_groups_update($exchange, $body) {
  if (!ces_bank_access('edit', 'exchange', $exchange['id'])) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }

  $edit = _ces_komunitin_api_social_groups_validate($body, FALSE);

  // Check code
  if (isset($edit['code']) && $edit['code'] != $exchange['code']) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN. "Code cannot be changed");
  }

  // main fields
  if (isset($edit['name'])) {
    $exchange['name'] = $edit['name'];
    $exchange['shortname'] = $edit['name'];
  }

  if (isset($edit['lng'])) {
    $exchange['lng'] = $edit['lng'];
  }
  if (isset($edit['lat'])) {
    $exchange['lat'] = $edit['lat'];
  }
  if (isset($edit['country'])) {
    $exchange['country'] = $edit['country'];
  }
  if (isset($edit['region'])) {
    $exchange['region'] = $edit['region'];
  }
  if (isset($edit['town'])) {
    $exchange['town'] = $edit['town'];
  }
  // fields saved in "data" column
  $data = $exchange['data'];
  if (isset($edit['image'])) {
    $data['image'] = $edit['image'];
  }
  if (isset($edit['description'])) {
    $data['description'] = $edit['description'];
  }

  $admin = user_load($exchange['admin']);
  $contacts = _ces_komunitin_api_social_get_included_contacts($body, $admin->email);
  if ($contacts !== null && count($contacts) > 0) {
    $data['contacts'] = $contacts;
  }

  $exchange['data'] = $data;

  $bank = new CesBank();
  $bank->updateExchange($exchange);

  // Reload exchange to reflect what actually was saved.
  $exchange = ces_bank_get_exchange($exchange['id']);
  return ces_komunitin_api_social_groups_read($exchange);
}

function ces_komunitin_api_social_groups_create($body) {
  // This is a public endpoint.
  $edit = _ces_komunitin_api_social_groups_validate($body, TRUE);
  // Check code
  if (isset($edit['code'])) {
    $existing = ces_bank_get_exchange_by_name($edit['code']);
    if ($existing) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Code already in use");
    }
  }

  $adminUuid = $body->data->relationships->admins->data[0]->id;
  $adminId = ces_komunitin_api_social_get_id($adminUuid)->id;

  $admin = user_load($adminId);
  $lang = user_preferred_language($admin)->language;

  $currency = _ces_komunitin_api_social_get_included_resource($body, 'currency');
  $contacts = _ces_komunitin_api_social_get_included_contacts($body, $admin->mail);

  $exchange = [
    'name' => $edit['name'],
    'shortname' => $edit['name'],
    'code' => $edit['code'],
    'description' => $edit['description'],
    'lng' => $edit['lng'],
    'lat' => $edit['lat'],
    'country' => $edit['country'],
    'region' => $edit['region'],
    'town' => $edit['town'],
    'admin' => $adminId,
    'currencysymbol' => $currency->attributes->symbol,
    'currencyname' => $currency->attributes->name,
    'currenciesname' => $currency->attributes->namePlural,
    'currencyvalue' => $currency->attributes->rate->n / $currency->attributes->rate->d,
    'currencyscale' => $currency->attributes->decimals,

    'data' => [
      'image' => isset($edit['image']) ? $edit['image'] : null,
      'description' => $edit['description'],
      'default_lang' => $lang,
      'komunitin_redirect' => TRUE,
      'komunitin_accounting' => TRUE,
      'komunitin_allow_anonymous_member_list' => FALSE,
      'contacts' => $contacts,
    ],
  ];

  if (isset($contacts[Contact::TYPE_WEBSITE])) {
    $exchange['website'] = $contacts[Contact::TYPE_WEBSITE];
    unset($contacts[Contact::TYPE_WEBSITE]);
  } else {
    $exchange['website'] = '';
  }

  $bank = new CesBank();
  $bank->createExchange($exchange);

  // Reload exchange.
  $exchange = ces_bank_get_exchange($exchange['id']);
  return ces_komunitin_api_social_groups_read($exchange);
}


function ces_komunitin_api_social_groups_settings_read($exchange) {
  // Public endpoint. Should we restrict it to members?
  return new GroupSettings($exchange);
}

function ces_komunitin_api_social_groups_settings_update($exchange, $body) {
  if (!ces_bank_access('edit', 'exchange', $exchange['id'])) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }

  if (!isset($body->data) || !isset($body->data->attributes)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }

  $attrs = $body->data->attributes;
  $data = $exchange['data'];

  /*
    requireAcceptTerms: boolean;
    terms: string;
    minOffers: number;
    minNeeds: number;
  */

  if (isset($attrs->requireAcceptTerms)) {
    $data['require_terms'] = $attrs->requireAcceptTerms === true;
  }

  if (isset($attrs->terms)) {
    $data['terms_text']['value'] = $attrs->terms;
    $data['terms_text']['format'] = 'plain_text';
  }

  if (isset($attrs->minOffers)) {
    $data['registration_offers'] = $attrs->minOffers;
  }

  if (isset($attrs->minNeeds)) {
    $data['registration_wants'] = $attrs->minNeeds;
  }

  if (isset($attrs->allowAnonymousMemberList)) {
    $data['komunitin_allow_anonymous_member_list'] = $attrs->allowAnonymousMemberList === true;
  }

  $exchange['data'] = $data; //?

  $bank = new CesBank();
  $bank->updateExchange($exchange);

  return ces_komunitin_api_social_groups_settings_read($exchange);

}

function ces_komunitin_api_social_users_load($exchange, $id) {
  if ($id === 'me') {
    global $user;
    return $user;
  } else {
    $result = ces_komunitin_api_social_get_id($id);
    if ($result === FALSE || $result->type != ResourceTypes::USER) {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
    }
    $user = user_load($result->id);
    if ($user == NULL) {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
    }
    return $user;
  }
}

// Read the current user
function ces_komunitin_api_social_users_read($record) {
  global $user;
  // Users can only be accessed by themselves or by the notifications service.
  $exchange = ces_bank_get_current_exchange($record);
  if (ces_komunitin_api_client_access('komunitin_social_read_all') || $user->uid === $record->uid || $user->uid === $exchange['admin']) {
    return new User($record, !empty($exchange) ? new Group($exchange) : NULL);
  } else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_users_load_collection($exchange = FALSE, $filters, $sorts, $pageAfter, $pageSize) {
  // User list must be filtered by member.
  if (!isset($filters['members'])) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Members filter is required');
  }

  $membersIds = $filters['members'];
  $members = [];
  foreach ($membersIds as $id) {
    $members[] = ces_komunitin_api_social_members_load(FALSE, $id);
  }
  $users = array_map(function($member) {
    return $member['user'];
  }, $members);

  // Ignore other filters, sorts and pagination

  return $users;
}

/*
 * Read some users. By now this is only used by the notifications service and
 * not by the app.
 */
function ces_komunitin_api_social_users_read_collection($users) {
  global $user; // Logged in user.
  $canReadAll = ces_komunitin_api_client_access('komunitin_social_read_all');
  foreach ($users as $record) {
    $exchange = ces_bank_get_current_exchange($record);
    if ($canReadAll || $user->uid === $exchange['admin'] || $user->uid === $record->uid) {
      $result[] = new User($record, new Group($exchange));
    }
  }
  return $result;
}

function ces_komunitin_api_social_users_update($userAccount, $body) {
  require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
  global $user;
  // Allow only the user to update itself.
  if ($user->uid !== $userAccount->uid) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }

  if (!isset($body->data)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $data = $body->data;
  if (isset($data->attributes)) {
    // Providing password is required for updating email or password.
    $auth_access = ces_komunitin_api_client_access('komunitin_auth');
    if (!$auth_access) {
      if (empty($data->attributes->password)) {
        ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "'komunitin_auth' scope or password is required for updating email or password.");
      }
      $password = $data->attributes->password;
      if (!user_check_password($password, $user)) {
        ces_komunitin_api_send_error(KomunitinApiError::INVALID_PASSWORD);
      }
    }
    if (!empty($data->attributes->email) && !valid_email_address($data->attributes->email)) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Invalid email");
    }

    $edit = [];
    // Update email
    if (!empty($data->attributes->email)) {
      $edit['mail'] = $data->attributes->email;
    }
    // Update password
    if (!empty($data->attributes->newPassword)) {
      $edit['pass'] = $data->attributes->newPassword;
    }
    // Update user in database.
    if (!empty($edit)) {
      $user = user_save($user, $edit);
    }
  }
  return ces_komunitin_api_social_users_read($user);
}

/**
 * Create a new user. This endpoint is called in the signup process and hence
 * it is not protected.
 */
function ces_komunitin_api_social_users_create($body) {
  // Validate request. It must have email, password and a member with name
  // associated to an exchange.
  if (!isset($body->data) || !isset($body->data->attributes)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $attributes = $body->data->attributes;
  if (!isset($attributes->email) || !isset($attributes->password)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  if (!valid_email_address($attributes->email)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Invalid email");
  }
  if (strlen($attributes->password) < 8) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Password too short");
  }
  $user = null;
  if ($user = user_load_by_mail($attributes->email)) {
    // Email already in use.
    // There are 2 options here
    // 1. The user is active. In this case we return an error and the client will
    // need to recover their password.
    if ($user->status == 1) {
      ces_komunitin_api_send_error(KomunitinApiError::DUPLICATED_EMAIL, "Email already in use");
    }
    // 2.  The user is inactive. In this case we'll update this user and resend
    // the validation email.
  }

  // Member included resource.
  $language = language_default();

  $has_member = isset($body->data->relationships) && isset($body->data->relationships->members) && isset($body->data->relationships->members->data) && count($body->data->relationships->members->data) === 1;
  if ($has_member) {
    $members = _ces_komunitin_api_social_get_included_resources($body, "members");
    $member = reset($members);

    if (empty($member->attributes->name)) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Missing member name");
    }
    if (!isset($member->relationships) || !isset($member->relationships->group) || !isset($member->relationships->group->data) || !isset($member->relationships->group->data->id)) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Missing member group");
    }
    $exchange_id = ces_komunitin_api_social_get_id($member->relationships->group->data->id);
    $exchange = ces_bank_get_exchange($exchange_id->id);
    if ($exchange == NULL) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Invalid member group");
    }
    if (isset($exchange['data']['default_lang'])) {
      $language = $exchange['data']['default_lang'];
    }
  }

  $has_settings = isset($body->data->relationships) && !empty($body->data->relationships->settings);
  if ($has_settings) {
    $settings = _ces_komunitin_api_social_get_included_resource($body, "settings");
    if (isset($settings->attributes->language)) {
      $language = $settings->attributes->language;
    }
  }

  // All right! Create the user.
  require_once __DIR__ . '/ces_komunitin.api.auth.inc';

  // The name is in the member resource for regular users, and in the attributes
  // for the admin user before creating the associated member and group.
  $name = $has_member ? $member->attributes->name : $attributes->name;
  // 1. Get free username.
  $username = ces_offerswants_slugify(substr($name, 0, 20));
  $error = user_validate_name($username);
  if ($error) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Invalid member name");
  }
  if (($existing = user_load_by_name($username)) && (empty($user) || $existing->uid != $user->uid)) {
    $number = 1;
    do {
      $new_username = $username . $number;
      $number++;
    } while (user_load_by_name($new_username));
    $username = $new_username;
  }

  // 2. Create user.
  $user = user_save($user, [
    'name' => $username,
    'mail' => trim($attributes->email),
    'init' => trim($attributes->email),
    'pass' => $attributes->password,
    'language' => $language,
    'status' => 0,
    'ces_firstname' => [LANGUAGE_NONE => [['value' => $name]]],
    'ces_surname' => [LANGUAGE_NONE => [['value' => ""]]],
  ]);

  if (!$user) {
    ces_komunitin_api_send_error(KomunitinApiError::INTERNAL_SERVER_ERROR, "User not created");
  }

  // 3. Create account (if not exists).
  if ($has_member) {
    $bank = new CesBank();
    $accounts = $bank->getUserAccountsInExchange($user->uid, $exchange['id']);
    if (empty($accounts)) {
      // Create account.
      $account = $bank->getDefaultAccount($exchange['id']);
      $account['users'][0]['user'] = $user->uid;
      $bank->createAccount($account, FALSE); // Do not send default email.
    }
  }

  // Send validation email.
  ces_komunitin_send_validation_email($user->uid, $has_member ? $exchange['code'] : NULL);

  // Return newly created user.
  return new User($user, $has_member ? new Group($exchange) : NULL);
}

function ces_komunitin_api_social_users_settings_read($record) {
  global $user;
  $exchange = ces_bank_get_current_exchange($record);
  if (ces_komunitin_api_client_access('komunitin_social_read_all') || $user->uid === $record->uid || $user->uid === $exchange['admin']) {
    return new UserSettings($record);
  } else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_users_settings_update($record, $body) {
  global $user;
  $exchange = ces_bank_get_current_exchange($record);

  // Own user or exchange admin.
  if (!($user->uid === $record->uid || $user->uid === $exchange['admin'])) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }

  module_load_include('module', 'ces_user');
  $obj = $body->data->attributes;
  $settings = [];
  if (isset($obj->language)) {
    $settings['language'] = $obj->language;
  }
  if (isset($obj->notifications)) {
    $settings['notifications'] = (array) $obj->notifications;
  }
  if (isset($obj->emails)) {
    $settings['emails'] = (array) $obj->emails;
  }
  ces_user_set_settings($record, $settings);
  return new UserSettings($record);
}

function ces_komunitin_api_social_members_load($exchange, $code) {
  // Load by id.
  $bank = new CesBank();
  if (ces_komunitin_api_social_is_uuid($code)) {
    $id = ces_komunitin_api_social_get_id($code);
    if ($id->type != ResourceTypes::MEMBER) {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
    }
    $account = $bank->getAccount($id->id);
  } else {
    // Load by account name.
    $account = $bank->getAccountByName($code);

  }
  if (empty($account)) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
  }
  $uid = _ces_komunitin_api_social_account_user_id($account['id']);
  $account['user'] = user_load($uid);
  return $account;
}

function ces_komunitin_api_social_members_load_collection($exchange, $filters, $sorts, $pageAfter, $pageSize) {
  $query = db_select('ces_account', 'a')
    ->fields('a')
    ->condition('a.exchange', $exchange['id'])
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

  // Filter out inactive accounts by default
  if (!isset($filters['state'])) {
    $filters['state'] = ['active'];
  }

  $addCondition = function($query, $field, $values) {
    if (count($values) == 1) {
      $query->condition($field, $values[0]);
    }
    else  {
      $query->condition($field, $values, 'IN');
    }
  };

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
      $addCondition($query, 'a.uuid', $values);
    }
    else if ($field == 'code') {
      $addCondition($query, 'a.name', $values);
    }
    else if ($field == 'state') {
      $statesTranslation = [
        Member::STATE_PENDING => CesBankLocalAccount::STATE_HIDDEN,
        Member::STATE_ACTIVE => CesBankLocalAccount::STATE_ACTIVE,
        Member::STATE_SUSPENDED => CesBankLocalAccount::STATE_LOCKED,
        Member::STATE_DELETED => CesBankLocalAccount::STATE_CLOSED,
      ];
      $values = array_map(function($state) use ($statesTranslation) {
        return $statesTranslation[$state];
      }, $values);
      $addCondition($query, 'a.state', $values);
    }
    else {
      ces_komunitin_api_send_error(KomunitinApiError::NOT_IMPLEMENTED, "Filtering other than ['code', 'state', 'account', 'search'] is not supported.");
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
  global $user;

  $client_access = ces_komunitin_api_client_access('komunitin_social_read_all');
  $user_access = ces_bank_access('view', 'exchange accounts', $exchange['id']);
  $anonymous_access = !$user_access && !empty($exchange['data']['komunitin_allow_anonymous_member_list']);

  if ($client_access || $anonymous_access || $user_access) {
    $group = new Group($exchange);
    $result = [];
    foreach ($members as $member) {
      if ($anonymous_access) {
        $result[] = new MinimalMember($member, $group);
      } else {
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

    // Check user activation.
    if (isset($attributes->state)) {
      $state = $attributes->state;
      if ($state == Member::STATE_ACTIVE && $member['state'] == CesBankLocalAccount::STATE_HIDDEN) {
        // Activate user.
        $bank = new CesBank();
        $bank->activateAccount($member);
      }
    }
  }

  $contacts = _ces_komunitin_api_social_get_included_contacts($body);
  if ($contacts !== NULL) {
    // Update contacts.
    ces_user_set_contacts($user, $contacts);
  }
  // Update user object inside $member so we return an updated resource.
  $member['user'] = user_load($user->uid);

  return ces_komunitin_api_social_members_read($exchange, $member);
}

/**
 * return null if no contacts relationship is present. Otherwise return an
 * associative array type=>value with the contacts from the included array
 * defined in the relationship.
 */
function _ces_komunitin_api_social_get_included_contacts($body, $excludeEmail = NULL) {
  $resources = _ces_komunitin_api_social_get_included_resources($body, "contacts", "contacts");
  if ($resources === NULL) {
    return NULL;
  }
  $contacts = [];
  foreach ($resources as $contact) {
    if ($excludeEmail !== NULL && $contact->attributes->type === ResourceTypes::EMAIL && $contact->attributes->name === $excludeEmail) {
      continue;
    }
    $contacts[$contact->attributes->type] = $contact->attributes->name;
  }
  return $contacts;
}

function _ces_komunitin_api_social_get_included_resource($body, $relationship) {
  $data = $body->data;
  if (empty($data->relationships) || empty($data->relationships->$relationship)) {
    return NULL;
  }
  if (!isset($body->included) || !isset($data->relationships->$relationship->data)
    || !isset($data->relationships->$relationship->data->id) || !isset($data->relationships->$relationship->data->type)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Missing included or related $relationship.");
  }

  $included = $body->included;
  $related = $data->relationships->$relationship->data;

  $id = $related->id;
  $type = $related->type;

  // Find the id in the included array.
  foreach ($included as $item) {
    if ($item->type == $type &&  $item->id == $id) {
      return $item;
    }
  }
  ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "$type $id not found in included array.");
}

function _ces_komunitin_api_social_get_included_resources($body, $relationship) {
  $data = $body->data;
  if (empty($data->relationships) || empty($data->relationships->$relationship)) {
    return NULL;
  }
  if (!isset($body->included) || !isset($data->relationships->$relationship->data)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Missing included or related $relationship.");
  }
  $included = $body->included;
  $resources = [];
  foreach ($data->relationships->$relationship->data as $related) {
    // Get the id of the related resource.
    $id = $related->id;
    $type = $related->type;
    // Find the id in the included array.
    $resource = NULL;
    foreach ($included as $item) {
      if ($item->type == $type &&  $item->id == $id) {
        $resource = $item;
        break;
      }
    }
    // We throw error when the resource is not found in the included array. We
    // could support not modified resources that are present in the relationship
    // but not in the included array, but it is not necessary for the actual
    // app usage of the API.
    if ($resource == NULL) {
      ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "$type $id not found in included array.");
    }
    $resources[] = $resource;
  }
  return $resources;
}

function ces_komunitin_api_social_members_delete($exchange, $member) {
  if (!ces_bank_access('admin', 'account', $member['id'])) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }

  if ($member['state'] == CesBankLocalAccount::STATE_CLOSED) {
    // Already deleted.
    return;
  }

  // If this exchange is configured to use the Komunitin accounting API,
  // delete the account in Komunitin.
  if ($member['state'] != CesBankLocalAccount::STATE_HIDDEN) {
    if (!empty($exchange['data']['komunitin_accounting'])) {
      module_load_include('inc', 'ces_komunitin', 'ces_komunitin.client');
      ces_komunitin_client_delete_account($member);
    }
  }

  // Delete the member. THis is using legacy code that is not very clean,
  // but anyway this service will all be replaced hopefully sooner than later.

  $bank = new CesBank();
  $checks = $bank->deleteAccountsCheck([$member['id']]);
  $check = $checks['accounts'][0];

  // Account.
  if ($check['num_transactions'] > 0) {
    // Can't delete the account as it has transactions, just mark it as closed.
    $member['state'] = CesBankLocalAccount::STATE_CLOSED;
    $bank->updateAccount($member);
  } else {
    // Completely delete the account.
    $bank->deleteAccount($member['id']);
  }

  // User(s)
  $users_cancel = array();
  foreach ($check['account_users'] as $user) {
    if ($user['user_action'] == 'delete' || $user['user_action'] == 'cancel') {
      $users_cancel[] = array(
        'uid' => $user['uid'],
        'action' => $user['user_action'],
      );
    }
  }
  if (!empty($users_cancel)) {
    $bank->deleteUsers($users_cancel);
  }
}

function _ces_komunitin_api_social_categories_load_extras($category) {
  $data = json_decode($category->description);

  $category->description = $data !== null ? $data->description : '';
  $category->icon = $data !== null ? $data->icon : null;
}
function _ces_komunitin_api_social_categories_save_extras($category) {
  $data = new StdClass();
  $data->description = $category->description;
  $data->icon = $category->icon;
  $category->description = json_encode($data);
}

function ces_komunitin_api_social_categories_load($exchange, $id) {
  $parsed = ces_komunitin_api_social_get_id($id);
  if ($parsed->type != ResourceTypes::CATEGORY) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
  }
  $category = ces_category_load($parsed->id);
  _ces_komunitin_api_social_categories_load_extras($category);
  if ($category == NULL) {
    ces_komunitin_api_send_error(KomunitinApiError::NOT_FOUND);
  }
  return $category;
}

function ces_komunitin_api_social_categories_read($exchange, $category) {
  if (ces_bank_access('view', 'exchange details', $exchange['id'])) {
    $group = new Group($exchange);
    return new Category($category, $group);
  } else {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
}

function ces_komunitin_api_social_categories_create($exchange, $body) {
  if (!ces_bank_access('edit', 'exchange details', $exchange['id'])) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
  if (!isset($body->data) || !isset($body->data->attributes)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $attributes = $body->data->attributes;
  if (empty(trim($attributes->name))) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Name is required");
  }
  if (empty(trim($attributes->icon))) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, "Icon is required");
  }
  $category = (object) [
    'title' => trim($attributes->name),
    'description' => isset($attributes->description) ? trim($attributes->description) : '',
    'icon' => $attributes->icon,
    'exchange' => $exchange['id'],
    'context' => 3, // 3 means categories for both offers and wants.
    'parent' => 0 // Flat hierarchy for komunitin categories.
  ];
  _ces_komunitin_api_social_categories_save_extras($category);

  $category = ces_category_save($category);

  return ces_komunitin_api_social_categories_read($exchange, $category);
}

function ces_komunitin_api_social_categories_update($exchange, $category, $body) {
  if (!ces_bank_access('edit', 'exchange details', $exchange['id'])) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
  if (!isset($body->data) || !isset($body->data->attributes)) {
    ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST);
  }
  $attributes = $body->data->attributes;
  if (isset($attributes->name)) {
    $category->title = trim($attributes->name);
  }
  if (isset($attributes->description)) {
    $category->description = trim($attributes->description);
  }
  if (isset($attributes->icon)) {
    $category->icon = $attributes->icon;
  }
  _ces_komunitin_api_social_categories_save_extras($category);
  ces_category_save($category);

  return ces_komunitin_api_social_categories_read($exchange, $category);
}

function ces_komunitin_api_social_categories_delete($exchange, $category) {
  if (!ces_bank_access('edit', 'exchange details', $exchange['id'])) {
    ces_komunitin_api_send_error(KomunitinApiError::FORBIDDEN);
  }
  ces_category_delete($category->id);
}

function ces_komunitin_api_social_categories_load_collection($exchange, $filters, $sorts) {
  // 3 means categories for both offers and wants.
  $categories = ces_offerswants_get_categories(3, $exchange['id']);
  foreach ($categories as $category) {
    _ces_komunitin_api_social_categories_load_extras($category);
  }

  return $categories;
}

function ces_komunitin_api_social_categories_read_collection($exchange, $categories) {
  // Public access to categories endpoint (it is required to create offers and
  // wants at signup).
  $group = new Group($exchange);
  $return = array_map(function ($category) use ($group) {
    return new Category($category, $group);
  }, $categories);
  return $return;
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
function ces_komunitin_api_social_needs_read_collection($exchange, $needs) {
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
 * Validates and permanently saves uploaded images.
 *
 * @param $files array of temporary file urls.
 * @param $directory directory where to save the images.
 *
 * return [
 *  'files' => All file ids
 *  'uploaded' => Uploaded file objects
 * ]
 */
function _ces_komunitin_api_social_validate_files($urls, $directory) {
  $files = [];
  $uploaded = [];
  if (isset($urls) && is_array($urls)) {
    foreach ($urls as $url) {
      $file = _ces_komunitin_api_social_get_file_by_url($url);
      if ($file == NULL) {
        ces_komunitin_api_send_error(KomunitinApiError::BAD_REQUEST, 'Invalid image ' . $url);
      } else {
        if (!$file->status) {
          $info = image_get_info($file->uri);
          if ($info) {
            $directory = file_default_scheme() . '://' . $directory;
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
        $files[] = $file->fid;
      }
    }
  }
  return [
    'files' => $files,
    'uploaded' => $uploaded
  ];
}

/**
 * @param $oldFiles array of file ids from existing record or null for create operations.
 * @param $newFiles array of file ids given by _ces_komunitin_api_social_offersneeds_validate()
 * @param $uploaded array of file objects given by _ces_komunitin_api_social_offersneeds_validate()
 * @param $module module name using the file
 * @param $type type of the object using the file
 * @param $id id of the object using the file
 */
function _ces_komuntin_social_update_files($oldFiles, $newFiles, $uploaded, $module, $type, $id) {
  if (!empty($oldFiles)) {
    $deletedFiles = array_diff($oldFiles, $newFiles);
    foreach ($deletedFiles as $fid) {
      if (($file = file_load($fid)) !== FALSE) {
        file_usage_delete($file, $module, $type, $id);
        file_delete($file);
      }
    }
  }

  foreach ($uploaded as $file) {
    file_usage_add($file, $module, $type, $id);
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
  $files = _ces_komunitin_api_social_validate_files($attributes->images, variable_get('ces_offerswants_picture_path', 'ces_offerswants_pictures'));

  $validated['image'] = implode(',',$files['files']);
  $validated['uploaded'] = $files['uploaded'];

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
  $newFiles = explode(',', $data['image']);
  _ces_komuntin_social_update_files(null, $newFiles, $data['uploaded'], 'ces_offerswants', 'ces_offerwant', $o->id);

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
    // Otherwise don't update the images.
    $oldFiles = strlen($o->image) > 0 ? explode(',', $o->image) : [];
    $newFiles = strlen($data['image']) > 0 ? explode(',', $data['image']) : [];
    _ces_komuntin_social_update_files($oldFiles, $newFiles, $data['uploaded'], 'ces_offerswants', 'ces_offerwant', $o->id);
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



