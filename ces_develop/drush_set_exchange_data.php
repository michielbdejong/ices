#!/usr/bin/env drush
<?php
/**
 * This script is to be called using drush scr command.
 *
 * It takes arguments of the form of
 * --code=XYZW komunitin_accounting_api_url
 *
 * And saves them to the data field of the exchange with the given code.
 */

 // Function to get all command-line arguments in key-value format
$code = drush_get_option('code', NULL);

if (!$code) {
  echo "Please provide the exchange code using --code=XYZW\n";
  exit(1);
}

$options = [
  'registration_offers' => intval(drush_get_option('registration_offers', 0)),
  'registration_wants' => intval(drush_get_option('registration_wants', 0)),
  'komunitin_accounting_api_url' => drush_get_option('komunitin_accounting_api_url', ""),
  'komunitin_app_url' => drush_get_option('komunitin_app_url', ""),
];

print_r($options);

// Serialize options
$serialized_options = serialize($options);

// Save serialized options to DB field
db_query("UPDATE {ces_exchange} SET data = :data WHERE code = :code", array(':data' => $serialized_options, ':code' => $code));

echo "Exchange $code data updated.\n";

