<?php
/**
 * @file
 * Forms definition.
 *
 * Forms definition, submit and validation functions for account users
 * in ces_bank module.
 */

/**
 * @defgroup ces_bank_forms_account_users Forms from account users in Ces Bank
 * @ingroup ces_bank
 * @{
 * Forms definition.
 */

/**
 * Form for add an user account.
 *
 * @param array $form
 *   Form.
 * @param array $form_state
 *   State form.
 * @param string $user
 *   Usuer.
 * @param string $account
 *   Account.
 * @param bool $administrator
 *   Exchange administrator or user account.
 */
function ces_bank_account_user_form($form, &$form_state, $user = NULL, $account = NULL, $administrator = FALSE) {
  $bank = new CesBank();
  if ($account == NULL) {
    $account = ces_bank_get_current_account();
  }

  if (!is_array($account)) {
    $account = $bank->getAccount($account);
  }
  // $user puede venir como objeto o solo el id.
  if (is_numeric($user)) {
    $user = user_load($user);
  }
  if (is_array($user)) {
    $user = user_load($user['username']);
  }

  $t_account = t('Account');
  drupal_set_title($t_account . ' ' . $account['name']);

  $admin = ces_bank_access('edit', 'account', $account['id']);

  if (!$admin) {
    drupal_set_message(t('Operation not permitted'));
    return FALSE;
  }

  if ($user && $account['users'][$user->uid]) {
    // Editing existing user in account
    $t_edit = t('Edit user in account');
    $form['titular'] = array(
      '#markup' => '<h2>' . $t_edit . ' ' . $account['name'] . '</h2>',
      '#weight' => -10,
    );
    $accuser = $account['users'][$user->uid];
    $privilege_default = ($accuser['privilege'] == 0);
    $form['id'] = array (
      '#type' => 'value',
      '#value' => $accuser['id'],
    );
    $username_default = $user->name;
  }
  else {
    // Adding a new user.
    $t_add = t('Add user in account');
    $form['titular'] = array(
      '#markup' => '<h2>' . $t_add . ' ' . $account['name'] . '</h2>',
      '#weight' => -10,
    );
    $privilege_default = 0;
    $username_default = '';
  }

  $form['administrator'] = array(
    '#type' => 'value',
    '#value' => ($administrator) ? 1 : 0,
  );

  $form['account_id'] = array(
    '#type' => 'value',
    '#value' => $account['id'],
  );

  $form['username'] = array(
    '#title' => t('Username'),
    '#description' => t('The name of the user.'),
    '#type' => 'textfield',
    '#autocomplete_path' => 'ces/bank/user/autocomplete',
    '#default_value' => $username_default,
    '#weight' => 1,
  );

  $form['privilege'] = array(
    '#type' => 'checkbox',
    '#title' => t('Privilege'),
    '#description' => t("Add administrator privileges."),
    '#default_value' => $privilege_default,
    '#weight' => 8,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 200,
  );
  $form['cancel'] = array(
    '#markup' => l(t('Cancel'), 'ces/bank/account/' . $account['id'] . '/users'),
    '#weight' => 201,
  );
  return $form;
}
/**
 * Form validate function for ces_bank_account_user_form.
 */
function ces_bank_account_user_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $name_user = $values['username'];
  if (empty($name_user)) {
    form_set_error('username', t('The user is required'));
  }
  if (!user_load_by_name($name_user)) {
    form_set_error('username', t('The user not exist'));
  }
}
/**
 * Submit add account user form.
 *
 * @param array $form
 *   Form.
 * @param array $form_state
 *   State form.
 */
function ces_bank_account_user_form_submit($form, &$form_state) {
  $record = &$form_state['values'];
  $administrator = (isset($record['administrator'])
   && $record['administrator'] == 1) ? TRUE : FALSE;

  $user = user_load_by_name($record['username']);
  $accountuser = array(
    'user' => $user->uid,
    'account' => $record['account_id'],
    'privilege' => $record['privilege'] ? 0 : 1,
  );

  $bank = new CesBank();

  try {
    if (isset($record['id'])) {
      // Update.
      $accountuser['id'] = $record['id'];
      $bank->updateAccountUser($accountuser);
    }
    else {
      $bank->createAccountUser($accountuser);
    }
    drupal_set_message(t('Users account successfully update.'));
    if ($administrator) {
      drupal_goto('ces/admin/account/' . $record['account_id'] . '/listusers');
    }
    else {
      drupal_goto('ces/bank/account/' . $record['account_id'] . '/users');
    }
  }
  catch (Exception $e) {
    if ($e->errorInfo[1] == 1062) {
      drupal_set_message(t('Duplicate entry'));
    }
    else {
      drupal_set_message(t('An error occurred while saving the user account record. Details: %msg', array('%msg' => $e->getMessage())), 'error');
    }
  }
}
/**
 * Form for deleting an user account.
 *
 * @param array $form
 *   Form.
 * @param array $form_state
 *   State form.
 * @param array $user
 *   Usuer.
 * @param array $account
 *   Account.
 * @param bool $administrator
 *   Exchange administrator or user account.
 */
function ces_bank_account_user_delete_form($form, &$form_state, $user = NULL, $account = NULL, $administrator = FALSE) {
  $drupal_user = FALSE;
  $t_delete = t('Delete account user');
  drupal_set_title($t_delete . ' ' . $account['name']);
  $bank = new CesBank();
  if ($account == NULL) {
    $account = ces_bank_get_current_account();
  }
  if (!is_array($account)) {
    $account = $bank->getAccount($account);
  }
  // $user It may come as object or the identification.
  if (is_numeric($user)) {
    $drupal_user = user_load($user);
  }
  if (is_array($user)) {
    $drupal_user = user_load($user['username']);
  }
  if (is_object($user)) {
    $drupal_user = $user;
  }
  // We allow decoupling nonexistent users.
  if (!$drupal_user) {
    $uid = $user;
    $username = t('Nonexistent user');
  }
  else {
    $uid = $drupal_user->uid;
    $username = $drupal_user->name;
  }

  // Get the accountuser record.
  $accountuserid = NULL;
  foreach ($account['users'] as $accountuser) {
    if ($accountuser['user'] == $uid && $accountuser['account'] == $account['id']) {
      $accountuserid = $accountuser['id'];
    }
  }

  $form['bankaccount'] = array();
  $form['bankaccount']['id'] = array(
    '#type' => 'value',
    '#value' => $accountuserid,
  );

  $form['bankaccount']['administrator'] = array(
    '#type' => 'value',
    '#value' => ($administrator) ? 1 : 0,
  );

  $form['bankaccount']['account_id'] = array(
    '#type' => 'value',
    '#value' => $account['id'],
  );

  $t_delete_user = t('Delete user');
  $t_account = t('account');
  $t_of = t('of');
  $form['bankaccount']['title'] = array(
    '#type' => 'item',
    '#title' => check_plain($t_delete_user . ' ' . $username . ' ' . $t_of . ' ' . $t_account),
    '#markup' => t('Confirm delete user of account.'),
  );
  $form['bankaccount']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Confirm'),
  );
  $form['cancel'] = array(
    '#markup' => l(t('Cancel'), 'ces/bank/account/' . $account['id'] . '/users'),
  );
  return $form;
}

/**
 * Submit bank account form.
 *
 * @param array $form
 *   Form.
 * @param array $form_state
 *   State form.
 */
function ces_bank_account_user_delete_form_submit($form, &$form_state) {

  $record = &$form_state['values'];
  $administrator = (isset($record['administrator'])
   && $record['administrator'] == 1) ? TRUE : FALSE;
  $bank = new CesBank();
  try {
    // Delete user from account.
    $bank->deleteAccountUser($record['id']);

    drupal_set_message(t('Users account successfully delete.'));
    if ($administrator) {
      drupal_goto('ces/admin/account/' . $record['account_id'] . '/listusers');
    }
    else {
      drupal_goto('ces/bank/account/' . $record['account_id'] . '/users');
    }
  }
  catch (Exception $e) {
    drupal_set_message(t('An error occurred while saving the user account record. Details: %msg', array('%msg' => $e->getMessage())), 'error');
  }
}
/**
 * @}
 */
