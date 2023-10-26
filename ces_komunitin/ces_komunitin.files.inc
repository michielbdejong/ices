<?php
require_once __DIR__ . "/ces_komunitin.api.inc";
require_once __DIR__ . "/includes/File.php";

define('MAX_FILE_SIZE', 1000000);
/**
 * Handler for /ces/files endpoint
 */
function ces_komunitin_files() {
  $token = oauth2_server_check_access('komunitin', 'komunitin_social');
  if (($token instanceof OAuth2\Response) || !$token['user_id']) {
    ces_komunitin_api_send_error(KomunitinApiError::UNAUTHORIZED);
  }
  $user = user_load($token['user_id']);
  // Check POST method
  if ($_SERVER['REQUEST_METHOD'] != 'POST') {
    ces_komunitin_api_send_error(KomunitinApiError::METHOD_NOT_ALLOWED);
  }
  try {
    // Use drupal API to save a temporary file. It will be automatically
    // cleaned up if not used.

    $validators = array('file_validate_is_image' => array());
    $file = file_save_upload('file', $validators);
    if (!$file) {
      throw new RuntimeException('Failed to move uploaded file.');
    }
    $file->url = file_create_url($file->uri);
    $result = new File($file);

    // Return file info.
    global $base_url;
    $urlprefix = $base_url . "/ces";
    $encoder = CustomFactory::newEncoder(array(
      File::class => FileSchema::class,
    ))
      ->withEncodeOptions(JSON_PRETTY_PRINT)
      ->withUrlPrefix($urlprefix);

    $content = $encoder->encodeData($result);
    $status = 201; // Created

    ces_komunitin_api_send_response($status, $content);

  } catch (RuntimeException $e) {
    ces_komunitin_api_send_error(KomunitinApiError::INTERNAL_SERVER_ERROR, $e->getMessage());
  }
}
