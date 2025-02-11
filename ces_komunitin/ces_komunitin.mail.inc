<?php

/**
 * @file
 * Functions to send Komunitin-branded emails.
 */

/**
 * Implements hook_mail().
 */
function ces_komunitin_mail($key, &$message, $params) {
  $langcode = $message['language']->language;
  $message['headers']['Content-Type'] = 'text/html; charset=UTF-8;';
  switch ($key) {
    case 'reset_password':
      $message['subject'] = t('Reset your Komunitin password', [], ['langcode' => $langcode]);
      $message['body'][] = _ces_komunitin_mail_html_template([
        t('We\'ve been requested to reset your password. Please click the following button to set a new password for your account.', [], ['langcode' => $langcode]),
        t('If you didn\'t request this, you can safely ignore this email.', [], ['langcode' => $langcode]),
      ], t('Set password'), $params['url'], $langcode);
      break;
    case 'signup_validate_email':
      $message['subject'] = t('Validate your email', [], ['langcode' => $langcode]);
      $message['body'][] = _ces_komunitin_mail_html_template([
        t('Welcome to Komunitin! Please click the following button to validate your email address and proceed with your registration.', [], ['langcode' => $langcode]),
      ], t('Validate email'), $params['url'], $langcode);
  }
}

function _ces_komunitin_mail_html_template($paragraphs, $action, $url, $langcode) {
  $app_url = variable_get('ces_komunitin_app_url', 'https://komunitin.org');

  $body = '<body style="background: #FAFAFA; padding: 20px;">'
  . '<table width="100%" border="0" cellspacing="20" cellpadding="0" style="background: #FFFFFF; max-width: 600px; margin: auto; border-radius: 10px;">';
  $body .= '<tr>'
  . '<td align="center" style="padding: 20px;">'
  . '<img src="' . $app_url . '/logos/logo-200.png" alt="Komunitin" style="width: 200px; height: auto;">'
  . '</td>'
  . '</tr>';

  foreach ($paragraphs as $paragraph) {
    $body .= '<tr>'
    . '<td align="center" style="padding: 10px font-size: 18px; font-family: Helvetica, Arial, sans-serif; color: #212121;">'
    . $paragraph
    . '</td>'
    . '</tr>';
  }
  $body .= '<tr>'
  . '<td align="center" style="padding: 20px;">'
  . '<table border="0" cellspacing="0" cellpadding="0">'
  . '<tr>'
  . '<td align="center" style="border-radius: 5px;" bgcolor="#72A310">'
  . '<a href="' . $url . '" target="_blank" style="font-size: 18px; font-family: Helvetica, Arial, sans-serif; color: #FFFFFF; text-decoration: none; border-radius: 5px; padding: 10px 20px; border: 1px solid #72A310; display: inline-block; font-weight: bold;">'
  . $action
  . '</a>'
  . '</td>'
  . '</tr>'
  . '</table>'
  . '</td>'
  . '</tr>'
  . '<tr>'
  . '<td align="center" style="padding: 10px; font-size: 16px; line-height: 22px; font-family: Helvetica, Arial, sans-serif; color: #212121;">'
  . t('The Komunitin system', [], ['langcode' => $langcode])
  . '</td>'
  . '</tr>'
  . '</table>'
  . '</body>';
  return $body;
}

function ces_komunitin_mail_alter(&$message) {
  // Don't send drupal system mails for komunitin users 
  // (unless mail was sent by the komunitin module).
  if ($message['module'] == 'ces_komunitin') {
    return;
  }
  
  $to = $message['to'];
  // Get the list of email addresses from the 'to' string.
  $addresses = [];
  preg_match_all('/[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}/', $to, $matches);
  if (!empty($matches[0])) {
    $addresses = $matches[0];
  }
  $newto = [];
  // Check which addresses are komunitin users.
  foreach ($addresses as $address) {
    $user = user_load_by_mail($address);
    $settings = ces_user_get_settings($user);
    if (empty($settings['komunitin'])) {
      $newto[] = $address;
    }
  }
  if (!empty($newto)) {
    $message['to'] = implode(', ', $newto);
  } else {
    $message['send'] = FALSE;
  }
}


module_load_include('inc', 'system', 'system.mail');

use MailerSend\MailerSend;
use MailerSend\Helpers\Builder\Recipient;
use MailerSend\Helpers\Builder\EmailParams;


class CesKomunitinMailSystem extends DefaultMailSystem {

  public function format(array $message) {
    $message['body'] = implode("\n\n", $message['body']);
    return $message;
  }

  public function mail(array $message) {
    $key = variable_get('ces_komunitin_mailersend_api_key', getenv('MAILERSEND_API_KEY'));
    $mailersend = new MailerSend(['api_key' => $key]);

    $recipients = [
      new Recipient($message['to'], $message['params']['to_name']),
    ];

    $from = variable_get('ces_komunitin_app_mail', 'app@komunitin.org');
    $from_name = variable_get('ces_komunitin_app_mail_name', 'Komunitin app');

    // Save html version of body before computing the text version.
    $html = $message['body'];
    // Get the text version.
    $message['body'] = [$message['body']];
    $message = parent::format($message);
    $text = $message['body'];

    $emailParams = (new EmailParams())
      ->setFrom($from)
      ->setFromName($from_name)
      ->setRecipients($recipients)
      ->setSubject($message['subject'])
      ->setHtml($html)
      ->setText($text);
    // TODO: Set reply-to to exchange admin email.

    $mailersend->email->send($emailParams);

    return true;

  }

}






