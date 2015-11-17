<?php
/**
 * @file
 * ces_bank view.
 */

/**
 * Views for CES Banks creates ces_exchange view.
 * @defgroup ces_bank_view Ces Bank View
 * @ingroup ces_bank
 * @{
 * View from bank.
 */
function ces_bank_views_data() {
  // Ces Exchange.
  $data['ces_exchange']['table']['group'] = t('Ces Exchange');
  $data['ces_exchange']['table']['base'] = array(
    'field' => 'id',
    'title' => t('Ces Exchange table'),
    'help' => t('Ces Exchange view.'),
    'weight' => -10,
  );
  $data['ces_exchange']['table']['join'] = array(
    'users' => array(
      'left_field' => 'uid',
      'field' => 'admin',
    ),
  );
  $data['ces_exchange']['id'] = array(
    'title' => t('Ces exchange Id'),
    'help' => t('Ces exchange id.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'relationship' => array(
      'base' => 'ces_message',
      'base field' => 'exchange',
      'handler' => 'views_handler_relationship',
      'label' => t('Ces exchange message relationship'),
      'title' => t('Ces exchange message relationship'),
      'help' => t('More information on this relationship'),
    ),
  );

  $data['ces_exchange']['shortname'] = array(
    'title' => t('Exchange Shortname'),
    'help' => t('Exchange Shortname.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['ces_account']['table']['group'] = t('Ces Account');
  $data['ces_account']['table']['base'] = array(
    'field' => 'id',
    'title' => t('Ces Account table'),
    'help' => t('Ces Account view.'),
    'weight' => -10,
  );
  $data['ces_account']['table']['join'] = array(
    'ces_accountuser' => array(
      'left_field' => 'account',
      'field' => 'id',
    ),
  );
  $data['ces_account']['id'] = array(
    'title' => t('Ces account Id'),
    'help' => t('Ces account id.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
    ),
    'relationship' => array(
      'base' => 'ces_accountuser',
      'base field' => 'account',
      'handler' => 'views_handler_relationship',
      'label' => t('Ces accountuser relationship'),
      'title' => t('Ces accountuser relationship'),
      'help' => t('More information on this relationship'),
    ),
  );
  $data['ces_account']['name'] = array(
    'title' => t('Account name'),
    'help' => t('Account name.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
  );
  $data['ces_account']['balance'] = array(
    'title' => t('Account balance'),
    'help' => t('Account balance.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
  );
  $data['ces_account']['exchange'] = array(
    'title' => t('Account exchange'),
    'help' => t('Account exchange.'),
    'field' => array(
      'handler' => 'views_handler_field',
    ),
    'filter' => array(
      'handler' => 'views_handler_field_numeric',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['ces_accountuser']['table']['group'] = t('Ces Account-Users');
  $data['ces_accountuser']['table']['base'] = array(
    'field' => 'id',
    'title' => t('Ces Account-User table'),
    'help' => t('Ces Account-User view.'),
    'weight' => -10,
  );
  $data['ces_accountuser']['table']['join'] = array(
    'users' => array(
      'left_field' => 'uid',
      'field' => 'user',
    ),
  );
  $data['ces_account']['table']['join'] = array(
    'ces_transactions' => array(
      'left_field' => 'toaccount',
      'field' => 'account',
    ),
  );
  $data['ces_accountuser']['user'] = array(
    'title' => t('Ces account user Id'),
    'help' => t('Ces account user Id.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'relationship' => array(
      'base' => 'users',
      'base field' => 'uid',
      'handler' => 'views_handler_relationship',
      'label' => t('Ces user relationship'),
      'title' => t('Ces user relationship'),
      'help' => t('More information on this relationship'),
    ),
  );
  $data['ces_accountuser']['account'] = array(
    'title' => t('Ces account(user) Id'),
    'help' => t('Ces account(user) Id.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'relationship' => array(
      'base' => 'ces_transaction',
      'base field' => 'toaccount',
      'handler' => 'views_handler_relationship',
      'label' => t('Ces transaction relationship'),
      'title' => t('Ces transaction relationship'),
      'help' => t('More information on this relationship'),
    ),
  );
  $data['ces_transaction']['table']['group'] = t('Ces Transactions');
  $data['ces_accountuser']['table']['base'] = array(
    'field' => 'id',
    'title' => t('Ces Transaction table'),
    'help' => t('Ces Transaction view.'),
    'weight' => -10,
  );
  $data['ces_transaction']['amount'] = array(
    'title' => t('Transactions amount'),
    'help' => t('Transactions amount.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
  );
  return $data;
}
/**
 * @} End of "defgroup ces_bank_view".
 */
