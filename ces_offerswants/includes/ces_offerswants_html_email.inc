<?php
/**
 * @file
 * Extends DefaultMailSystem class for sending html emails.
 */

/**
 * We extend DefaultMailSystem class for sending html emails.
 */
class CesOfferswantsMailSystem extends DefaultMailSystem {
  public function format(array $message) {
    $message['body'] = implode("\n\n", $message['body']);
    $message['body'] = nl2br($message['body']);
    $message['body'] = drupal_wrap_mail($message['body']);
    return $message;
  }
}