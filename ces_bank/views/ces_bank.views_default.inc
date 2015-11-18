<?php
/**
 * @file
 * @brief Views to ces bank
 *
 * @defgroup ces_bank_views_default Ces Bank Views defaults
 * @ingroup ces_bank
 * @{
 * View from ces_bank.
 */

/**
 * Implements hook_views_default_views().
 */
function ces_bank_views_default_views() {
$views = array();

$view = new view();
$view->name = 'ces_bank_accounts_list';
$view->description = 'Accounts\' list of an exchange.';
$view->tag = 'ices';
$view->base_table = 'ces_account';
$view->human_name = 'Accounts list';
$view->core = 7;
$view->api_version = '3.0';
$view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

/* Display: Master */
$handler = $view->new_display('default', 'Master', 'default');
$handler->display->display_options['use_ajax'] = TRUE;
$handler->display->display_options['use_more_always'] = FALSE;
$handler->display->display_options['access']['type'] = 'role';
$handler->display->display_options['access']['role'] = array(
  3 => '3',
  2 => '2',
);
$handler->display->display_options['cache']['type'] = 'none';
$handler->display->display_options['query']['type'] = 'views_query';
$handler->display->display_options['exposed_form']['type'] = 'basic';
$handler->display->display_options['pager']['type'] = 'full';
$handler->display->display_options['pager']['options']['items_per_page'] = '100';
$handler->display->display_options['style_plugin'] = 'table';
$handler->display->display_options['style_options']['columns'] = array(
  'id' => 'id',
  'name' => 'name',
  'name_1' => 'name_1',
  'ces_firstname' => 'ces_firstname',
  'ces_surname' => 'ces_surname',
  'mail' => 'mail',
  'balance' => 'balance',
);
$handler->display->display_options['style_options']['default'] = 'name';
$handler->display->display_options['style_options']['info'] = array(
  'id' => array(
    'align' => '',
    'separator' => '',
    'empty_column' => 0,
  ),
  'name' => array(
    'sortable' => 1,
    'default_sort_order' => 'asc',
    'align' => '',
    'separator' => '',
    'empty_column' => 0,
  ),
  'name_1' => array(
    'sortable' => 1,
    'default_sort_order' => 'asc',
    'align' => '',
    'separator' => ': ',
    'empty_column' => 0,
  ),
  'ces_firstname' => array(
    'sortable' => 0,
    'default_sort_order' => 'asc',
    'align' => '',
    'separator' => '',
    'empty_column' => 0,
  ),
  'ces_surname' => array(
    'sortable' => 0,
    'default_sort_order' => 'asc',
    'align' => '',
    'separator' => '',
    'empty_column' => 0,
  ),
  'mail' => array(
    'sortable' => 1,
    'default_sort_order' => 'asc',
    'align' => '',
    'separator' => '',
    'empty_column' => 0,
  ),
  'balance' => array(
    'sortable' => 1,
    'default_sort_order' => 'asc',
    'align' => '',
    'separator' => '',
    'empty_column' => 0,
  ),
);
/* Relationship: Ces Account: Ces accountuser relationship */
$handler->display->display_options['relationships']['id']['id'] = 'id';
$handler->display->display_options['relationships']['id']['table'] = 'ces_account';
$handler->display->display_options['relationships']['id']['field'] = 'id';
/* Relationship: Ces Account-Users: Ces user relationship */
$handler->display->display_options['relationships']['user']['id'] = 'user';
$handler->display->display_options['relationships']['user']['table'] = 'ces_accountuser';
$handler->display->display_options['relationships']['user']['field'] = 'user';
$handler->display->display_options['relationships']['user']['relationship'] = 'id';
/* Field: Ces Account: Ces account Id */
$handler->display->display_options['fields']['id']['id'] = 'id';
$handler->display->display_options['fields']['id']['table'] = 'ces_account';
$handler->display->display_options['fields']['id']['field'] = 'id';
$handler->display->display_options['fields']['id']['exclude'] = TRUE;
/* Field: Ces Account: Account name */
$handler->display->display_options['fields']['name']['id'] = 'name';
$handler->display->display_options['fields']['name']['table'] = 'ces_account';
$handler->display->display_options['fields']['name']['field'] = 'name';
$handler->display->display_options['fields']['name']['label'] = 'Account';
$handler->display->display_options['fields']['name']['element_label_colon'] = FALSE;
/* Field: User: Name */
$handler->display->display_options['fields']['name_1']['id'] = 'name_1';
$handler->display->display_options['fields']['name_1']['table'] = 'users';
$handler->display->display_options['fields']['name_1']['field'] = 'name';
$handler->display->display_options['fields']['name_1']['relationship'] = 'user';
$handler->display->display_options['fields']['name_1']['label'] = 'User';
$handler->display->display_options['fields']['name_1']['element_label_colon'] = FALSE;
$handler->display->display_options['fields']['name_1']['link_to_user'] = FALSE;
/* Field: User: First Name */
$handler->display->display_options['fields']['ces_firstname']['id'] = 'ces_firstname';
$handler->display->display_options['fields']['ces_firstname']['table'] = 'field_data_ces_firstname';
$handler->display->display_options['fields']['ces_firstname']['field'] = 'ces_firstname';
$handler->display->display_options['fields']['ces_firstname']['relationship'] = 'user';
$handler->display->display_options['fields']['ces_firstname']['exclude'] = TRUE;
/* Field: User: Surname */
$handler->display->display_options['fields']['ces_surname']['id'] = 'ces_surname';
$handler->display->display_options['fields']['ces_surname']['table'] = 'field_data_ces_surname';
$handler->display->display_options['fields']['ces_surname']['field'] = 'ces_surname';
$handler->display->display_options['fields']['ces_surname']['relationship'] = 'user';
$handler->display->display_options['fields']['ces_surname']['exclude'] = TRUE;
$handler->display->display_options['fields']['ces_surname']['element_label_colon'] = FALSE;
/* Field: User: E-mail */
$handler->display->display_options['fields']['mail']['id'] = 'mail';
$handler->display->display_options['fields']['mail']['table'] = 'users';
$handler->display->display_options['fields']['mail']['field'] = 'mail';
$handler->display->display_options['fields']['mail']['relationship'] = 'user';
$handler->display->display_options['fields']['mail']['link_to_user'] = '0';
/* Field: Ces Account: Account balance */
$handler->display->display_options['fields']['balance']['id'] = 'balance';
$handler->display->display_options['fields']['balance']['table'] = 'ces_account';
$handler->display->display_options['fields']['balance']['field'] = 'balance';
$handler->display->display_options['fields']['balance']['label'] = 'Balance';
$handler->display->display_options['fields']['balance']['element_label_colon'] = FALSE;
/* Contextual filter: Ces Account: Account exchange */
$handler->display->display_options['arguments']['exchange']['id'] = 'exchange';
$handler->display->display_options['arguments']['exchange']['table'] = 'ces_account';
$handler->display->display_options['arguments']['exchange']['field'] = 'exchange';
$handler->display->display_options['arguments']['exchange']['default_action'] = 'not found';
$handler->display->display_options['arguments']['exchange']['default_argument_type'] = 'fixed';
$handler->display->display_options['arguments']['exchange']['summary']['number_of_records'] = '0';
$handler->display->display_options['arguments']['exchange']['summary']['format'] = 'default_summary';
$handler->display->display_options['arguments']['exchange']['summary_options']['items_per_page'] = '25';
$handler->display->display_options['arguments']['exchange']['specify_validation'] = TRUE;
$handler->display->display_options['arguments']['exchange']['validate']['type'] = 'numeric';
$handler->display->display_options['arguments']['exchange']['limit'] = '0';
/* Filter criterion: Ces Account: Account exchange */
$handler->display->display_options['filters']['exchange']['id'] = 'exchange';
$handler->display->display_options['filters']['exchange']['table'] = 'ces_account';
$handler->display->display_options['filters']['exchange']['field'] = 'exchange';
$handler->display->display_options['filters']['exchange']['label'] = 'Exchange';
$handler->display->display_options['filters']['exchange']['element_label_colon'] = FALSE;
/* Filter criterion: Global: Combine fields filter */
$handler->display->display_options['filters']['combine']['id'] = 'combine';
$handler->display->display_options['filters']['combine']['table'] = 'views';
$handler->display->display_options['filters']['combine']['field'] = 'combine';
$handler->display->display_options['filters']['combine']['operator'] = 'contains';
$handler->display->display_options['filters']['combine']['exposed'] = TRUE;
$handler->display->display_options['filters']['combine']['expose']['operator_id'] = 'combine_op';
$handler->display->display_options['filters']['combine']['expose']['label'] = 'Search for account, user or email';
$handler->display->display_options['filters']['combine']['expose']['operator'] = 'combine_op';
$handler->display->display_options['filters']['combine']['expose']['identifier'] = 'combine';
$handler->display->display_options['filters']['combine']['expose']['remember_roles'] = array(
  2 => '2',
  1 => 0,
  3 => 0,
);
$handler->display->display_options['filters']['combine']['fields'] = array(
  'name' => 'name',
  'name_1' => 'name_1',
  'ces_firstname' => 'ces_firstname',
  'ces_surname' => 'ces_surname',
  'mail' => 'mail',
);
/* Filter criterion: Ces Account: Account state */
$handler->display->display_options['filters']['state']['id'] = 'state';
$handler->display->display_options['filters']['state']['table'] = 'ces_account';
$handler->display->display_options['filters']['state']['field'] = 'state';
$handler->display->display_options['filters']['state']['value']['value'] = '1';
/* Filter criterion: Ces Account: Account kind */
$handler->display->display_options['filters']['kind']['id'] = 'kind';
$handler->display->display_options['filters']['kind']['table'] = 'ces_account';
$handler->display->display_options['filters']['kind']['field'] = 'kind';
$handler->display->display_options['filters']['kind']['operator'] = '!=';
$handler->display->display_options['filters']['kind']['value']['value'] = '5';

/* Display: Accounts  */
$handler = $view->new_display('block', 'Accounts ', 'block');
$handler->display->display_options['display_description'] = 'Accounts list, display for auth-users.';
$translatables['ces_bank_accounts_list'] = array(
  t('Master'),
  t('more'),
  t('Apply'),
  t('Reset'),
  t('Sort by'),
  t('Asc'),
  t('Desc'),
  t('Items per page'),
  t('- All -'),
  t('Offset'),
  t('« first'),
  t('‹ previous'),
  t('next ›'),
  t('last »'),
  t('Ces accountuser relationship'),
  t('Ces user relationship'),
  t('Ces account Id'),
  t('.'),
  t(','),
  t('Account'),
  t('User'),
  t('First Name'),
  t('Surname'),
  t('E-mail'),
  t('Balance'),
  t('All'),
  t('Exchange'),
  t('Search for account, user or email'),
  t('Accounts '),
  t('Accounts list, display for auth-users.'),
);

$views['accounts_list'] = $view;

  return $views;
}
/**
 * @}
 */
