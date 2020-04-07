<?php

/**
 * This file contains the code to export and format in HTML a book with all offers in an exchange.
 *
 * This is based on a previous script that used to run on exported CSV directly from the DB.
 * That's why it doesn't use regular load mechanisms for offers. Further, it is probably
 * more efficient to just do a big SQL query exactly with the data we need.
 */

/**
 * Implements hook_form().
 */
function ces_offerswants_book_form($form, &$form_state) {
  $form['expire_date'] = array(
    '#type' => 'date',
    '#title' => t('Expire after'),
    '#description' => t('This feature allows to include expired offers in the export file.')
  );
  $form['download'] = array(
    '#type' => 'submit',
    '#value' => t('Download'),
    '#description' => t('Export offers to HTML.')
  );
  return $form;
}
/**
 * Implements hook_form_submit().
 */
function ces_offerswants_book_form_submit($form, &$form_state) {
  // Compute timestamp.
  $date = $form_state['values']['expire_date'];
  $expire = mktime(24, 0, 0, $date['month'], $date['day'], $date['year']);

  $exchange = ces_offerswants_get_current_exchage_id();

  $sql = "SELECT c.title AS 'category', o.title, o.body, o.modified, a.name AS 'account',
    f.ces_firstname_value AS 'firstname', s.ces_surname_value AS 'lastname', u.mail,
    pm.ces_phonemobile_value AS 'phonemobile', pw.ces_phonework_value AS 'phonework',
    ph.ces_phonehome_value AS 'phonehome', t.ces_town_value AS 'town',
    p.ces_postcode_value AS 'postcode'
    FROM {ces_category} c, {ces_offerwant} o, {ces_accountuser} au, {ces_account} a, {users} u
    LEFT JOIN {field_data_ces_firstname} f
    ON (u.uid = f.entity_id)
    LEFT JOIN {field_data_ces_surname} s
    ON (u.uid = s.entity_id)
    LEFT JOIN {field_data_ces_phonemobile} pm
    ON (u.uid = pm.entity_id)
    LEFT JOIN {field_data_ces_phonework} pw
    ON (u.uid = pw.entity_id)
    LEFT JOIN {field_data_ces_phonehome} ph
    ON (u.uid = ph.entity_id)
    LEFT JOIN {field_data_ces_town} t
    ON (u.uid = t.entity_id)
    LEFT JOIN {field_data_ces_postcode} p
    ON (u.uid = p.entity_id)
    WHERE au.user = u.uid AND au.account = a.id AND a.exchange = :exchange AND o.category = c.id
    AND c.exchange = :exchange AND o.user = u.uid AND o.type = 'offer' AND o.state = 1
    AND a.kind < 5 AND (o.expire > :expire)";

  // Get records.
  $result = db_query($sql, array(':exchange' => $exchange, ':expire' => $expire));
  $records = array();
  // Format some data.
  foreach ($result as $record) {
    $records[] = format_record($record);
  }

  // Split into categories.
  $categories = split_categories($records);

  // Group offers by user.
  foreach ($categories as $key => $category) {
    $category->records = split_users($category->records);
  }

  // Print result.
  $html = print_offerbook_data($categories);

  // Return result as a download.
  drupal_add_http_header('Content-Type', 'text/html; charset=utf-8');
  drupal_add_http_header('Content-Length', strlen($html));
  drupal_add_http_header('Content-Disposition', 'attachment; filename="offerbook.html"');
  echo $html;
  drupal_exit();
}

function format_record($record) {
  $record->title = format_text($record->title, 60);
  if (ctype_upper($record->title)) {
    $record->title = substr($record->title, 0, 1) . strtolower(substr($record->title, 1));
  }
  $record->body = format_text($record->body, 300);
  $record->town = format_text($record->town, 50);
  return $record;
}

function format_text($title, $len) {
  $title = strip_tags($title);
  $title = html_entity_decode($title, ENT_COMPAT, 'UTF-8');
  $title = trim($title);
  $title = str_replace("\n", " ", $title);
  $title = preg_replace("/\\s\\s+/", ' ', $title);
  $title = mb_strtoupper(mb_substr($title, 0, 1)) . mb_substr($title, 1);
  $title = str_replace(' I ', ' i ', $title);
  if (mb_strlen($title) > $len) {
    $title = mb_substr($title, 0, $len);
    $n = mb_strrpos($title, ' ');
    if ($n < $len - 15) {
      $n = $len - 3;
    }
    $title = mb_substr($title, 0, $len) . '...';
  }
  return $title;
}

/**
 * @param array $data Is an array of objects. Each object is a record.
 *  */
function split_categories($data) {
  $splitted = array();
  foreach ($data as $record) {
    if (!isset($splitted[$record->category])) {
      $category = new stdClass();
      $category->category = $record->category;
      $category->records = array();
      $splitted[$record->category] = $category;
    }
    $splitted[$record->category]->records[] = $record;
    ksort($splitted);
  }
  return $splitted;
}


/**
 * @param array $data
 *  */
function split_users($data) {
  $users = array();
  foreach ($data as $record) {
    $key = $record->postcode . ' - ' . $record->account;
    if (!isset($users[$key])) {
      $account = new StdClass();
      $account->account = $record->account;
      $account->fullname = fullname($record->firstname, $record->lastname);
      $account->phone = phone($record->phonemobile, $record->phonework, $record->phonehome);
      $account->mail = $record->mail;
      $account->town = $record->town;
      $account->offers = array();
      $users[$key] = $account;
    }
    $users[$key]->offers[$record->title] = $record;
  }
  ksort($users);
  foreach ($users as $account) {
    ksort($account->offers);
  }
  return $users;
}

function fullname($firstname, $lastname) {
  return format_text($firstname . ' ' . $lastname, 1000);
}

function phone($p1, $p2, $p3) {
  $phone = trim($p1);
  if (empty($phone)) {
    $phone = trim($p2);
    if (empty($phone)) {
      $phone = trim($p3);
    }
  }
  $phone = preg_replace('/[^0-9]/', '', $phone);
  return $phone;
}

function print_offerbook_data($data) {
  $out = '';
  foreach ($data as $category) {
    $out .= '<h2>' . $category->category . '</h2>' . "\n";
    $out .= '<table>' . "\n";
    foreach ($category->records as $user) {
      $first = true;
      foreach ($user->offers as $offer) {
        $out .= '<tr>' . "\n";
        $out .= '<td style="width: 70%; border-top: solid, 1px #555; vertical-align: top;" valign="top">' . print_offer($offer) . '</td>' . "\n";
        if ($first) {
          $out .= '<td rowspan="' . count($user->offers) . '" style="width: 30%; border-top: solid, 1px #555; vertical-align: top;"  valign="top">' . "\n";
          $out .= print_user($user) . "\n";
          $out .= '</td>' . "\n";
          // rowspan.
          $first = false;
        }
        $out .= '</tr>';
      }
    }
    $out .= '</table>';
  }
  return $out;
}

function print_user($user) {
  $out = '<h5>' . htmlspecialchars($user->fullname) . '</h5>' . "\n";
  $out .= '<p>' . htmlspecialchars($user->mail) . '<br/>' . "\n";
  $out .= $user->phone . '<br/>' . "\n";
  $out .= $user->town . '</p>' . "\n";
  return $out;
}

function print_offer($offer) {
  $out = '<h6>' . htmlspecialchars($offer->title) . '</h6>' . "\n";
  $out .= '<p>' . htmlspecialchars($offer->body) . '</p>' . "\n";
  return $out;
}
