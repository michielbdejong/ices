<?php
/**
 * @file
 * @brief Views to ces rest
 *
 * @defgroup ces_rest_views_default Ces Rest Views defaults
 * @ingroup ces_rest
 * @{
 * View rest.
 */

/**
 * Implements hook_views_default_views().
 */
function ces_rest_views_default_views() {
  $export = array();

  $view = new view();
  $view->name = 'cr_accounts';
  $view->description = '';
  $view->tag = 'default';
  $view->base_table = 'users';
  $view->human_name = 'Ces Resr Accounts';
  $view->core = 7;
  $view->api_version = '3.0';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

  /* Display: Master */
  $handler = $view->new_display('default', 'Master', 'default');
  $handler->display->display_options['title'] = 'ces_accounts';
  $handler->display->display_options['use_more_always'] = FALSE;
  $handler->display->display_options['access']['type'] = 'none';
  $handler->display->display_options['cache']['type'] = 'none';
  $handler->display->display_options['query']['type'] = 'views_query';
  $handler->display->display_options['exposed_form']['type'] = 'basic';
  $handler->display->display_options['exposed_form']['options']['reset_button_label'] = 'Reinicialitza';
  $handler->display->display_options['pager']['type'] = 'some';
  $handler->display->display_options['pager']['options']['items_per_page'] = '5';
  $handler->display->display_options['style_plugin'] = 'default';
  $handler->display->display_options['row_plugin'] = 'fields';
  /* Relationship: Ces Exchange: Ces exchange message relationship */
  $handler->display->display_options['relationships']['id']['id'] = 'id';
  $handler->display->display_options['relationships']['id']['table'] = 'ces_exchange';
  $handler->display->display_options['relationships']['id']['field'] = 'id';
  $handler->display->display_options['relationships']['id']['required'] = TRUE;
  /* Camp: Usuari: Uid */
  $handler->display->display_options['fields']['uid']['id'] = 'uid';
  $handler->display->display_options['fields']['uid']['table'] = 'users';
  $handler->display->display_options['fields']['uid']['field'] = 'uid';
  /* Camp: Usuari: Nom */
  $handler->display->display_options['fields']['name_1']['id'] = 'name_1';
  $handler->display->display_options['fields']['name_1']['table'] = 'users';
  $handler->display->display_options['fields']['name_1']['field'] = 'name';
  $handler->display->display_options['fields']['name_1']['link_to_user'] = FALSE;
  /* Camp: Ces Exchange: Ces exchange Id */
  $handler->display->display_options['fields']['id']['id'] = 'id';
  $handler->display->display_options['fields']['id']['table'] = 'ces_exchange';
  $handler->display->display_options['fields']['id']['field'] = 'id';
  /* Camp: Ces Exchange: Exchange Shortname */
  $handler->display->display_options['fields']['shortname']['id'] = 'shortname';
  $handler->display->display_options['fields']['shortname']['table'] = 'ces_exchange';
  $handler->display->display_options['fields']['shortname']['field'] = 'shortname';
  /* Sort criterion: Usuari: Created date */
  $handler->display->display_options['sorts']['created']['id'] = 'created';
  $handler->display->display_options['sorts']['created']['table'] = 'users';
  $handler->display->display_options['sorts']['created']['field'] = 'created';
  $handler->display->display_options['sorts']['created']['order'] = 'DESC';
  /* Filter criterion: Usuari: Activa */
  $handler->display->display_options['filters']['status']['id'] = 'status';
  $handler->display->display_options['filters']['status']['table'] = 'users';
  $handler->display->display_options['filters']['status']['field'] = 'status';
  $handler->display->display_options['filters']['status']['value'] = '1';
  $handler->display->display_options['filters']['status']['group'] = 1;
  $handler->display->display_options['filters']['status']['expose']['operator'] = FALSE;

  /* Display: Services */
  $handler = $view->new_display('services', 'Services', 'services_1');
  $handler->display->display_options['path'] = 'ces_users';
  $translatables['ces_accounts'] = array(
    t('Master'),
    t('ces_accounts'),
    t('more'),
    t('Apply'),
    t('Reinicialitza'),
    t('Sort by'),
    t('Asc'),
    t('Desc'),
    t('Ces exchange message relationship'),
    t('Uid'),
    t('Nom'),
    t('Ces exchange Id'),
    t('.'),
    t(','),
    t('Exchange Shortname'),
    t('Services'),
  );


  $export['ces_users_accounts'] = $view;

  return $export;
}
/**
 * @}
 */
