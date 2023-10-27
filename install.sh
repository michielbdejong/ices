#!/bin/bash

while [ $# -gt 0 ]; do
  case $1 in
    --demo)
      DEMO="TRUE"
      shift
      ;;
    --dev)
      DEV="TRUE"
      shift
      ;;
    *)
      shift
      ;;
  esac
done
# Install Drupal
echo "Installing site ${ICES_SITE_NAME:-IntegralCES}, with admin password ${ICES_ADMIN_PASSWORD:-integralces}."
docker compose exec integralces drush si standard --db-url="mysql://integralces:${ICES_MYSQL_PASSWORD:-integralces}@db/integralces" --account-name="admin" --account-pass="${ICES_ADMIN_PASSWORD:-integralces}" --site-name="${ICES_SITE_NAME:-IntegralCES}" -y
# Enable modules
docker compose exec integralces drush en -y \
  oauth2_server services image views \
  token libraries services_views \
  cors login_emailusername smtp bounce geolocation 
docker compose exec integralces drush en -y \
  ices ces_bank ces_blog ces_interop ces_message ces_offerswants ces_qr ces_rest ces_statistics ces_summaryblock ces_user ces_komunitin \
  greences
# Configurations
docker compose exec integralces drush vset theme_default greences 
docker compose exec integralces drush role-add-perm 'anonymous user' 'use oauth2 server' 
docker compose exec integralces drush role-add-perm 'authenticated user' 'use oauth2 server'
docker compose exec integralces drush ev "variable_set('cors_domains', array('*'=>'<mirror>|GET,POST,PATCH,DELETE,OPTIONS|Content-Type,Authorization|true'));"

if [ "$DEMO" = "TRUE" ]; then
  docker compose exec integralces drush dl -y devel
  docker compose exec integralces drush en -y devel ces_develop simpletest maillog
  docker compose exec integralces drush vset maillog_send 0
  docker compose exec integralces drush php-script sites/all/modules/ices/ces_develop/demo.php
  docker compose exec integralces chown www-data:www-data -R sites/default
fi

if [ "$DEV" = "TRUE" ]; then
  # Install XDebug extension with PECL
  docker compose exec integralces pecl install xdebug-3.1.6
  # Configure XDebug
  docker compose exec integralces printf "\n[XDebug]\n\
  xdebug.mode = debug\n\
  xdebug.client_host = host.docker.internal\n\
  " >> /usr/local/etc/php/conf.d/docker-php-ext-xdebug.ini
  # Enable XDebug
  docker compose exec integralces docker-php-ext-enable xdebug
  # Restart web server
  docker compose restart integralces
fi

echo "IntegralCES installed!"
