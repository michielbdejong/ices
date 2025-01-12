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
echo "Installing site ${ICES_SITE_NAME:-IntegralCES}."
docker compose exec -T integralces drush si standard --db-url="mysql://integralces:${ICES_MYSQL_PASSWORD:-integralces}@db-integralces/integralces" --account-name="admin" --account-pass="${ICES_ADMIN_PASSWORD:-integralces}" --site-name="${ICES_SITE_NAME:-IntegralCES}" -y
# Download modules with specific versions to avoid prompts.
docker compose exec -T integralces drush dl -y \
  xautoload-7.x-5.9 ctools-7.x-1.21 entityreference-7.x-1.9 entity-7.x-1.11 
# Enable modules
docker compose exec -T integralces drush en -y \
  oauth2_server services image views \
  token libraries services_views \
  cors login_emailusername smtp bounce geolocation \
  locale
docker compose exec -T integralces drush en -y \
  ices ces_bank ces_blog ces_interop ces_message ces_offerswants ces_qr ces_rest ces_statistics ces_summaryblock ces_user ces_komunitin \
  greences

# Language 
docker compose exec -T integralces drush dl -y drush_language-7.x-1.6-rc3
docker compose exec -T integralces drush en -y drush_language
docker compose exec -T integralces drush language-add ca
docker compose exec -T integralces drush language-add es

# Configurations
docker compose exec -T integralces drush vset theme_default greences 
docker compose exec -T integralces drush role-add-perm 'anonymous user' 'use oauth2 server' 
docker compose exec -T integralces drush role-add-perm 'authenticated user' 'use oauth2 server'
docker compose exec -T integralces drush ev "variable_set('cors_domains', array('*'=>'<mirror>|GET,POST,PATCH,DELETE,OPTIONS|Content-Type,Authorization|true'));"

# Configure $base_url in settings.php
docker compose exec -T integralces sh -c "echo \"\n\\\$base_url = '$BASE_URL';\n\" >> sites/default/settings.php"


if [ "$DEMO" = "TRUE" ]; then
  docker compose exec -T integralces drush dl -y maillog-7.x-1.0-rc1
  docker compose exec -T integralces drush en -y ces_develop simpletest maillog  
  docker compose exec -T integralces drush vset maillog_send 0
  docker compose exec -T integralces drush php-script sites/all/modules/ices/ces_develop/demo.php
  docker compose exec -T integralces chown www-data:www-data -R sites/default
fi

if [ "$DEV" = "TRUE" ]; then
  # Install XDebug extension with PECL
  docker compose exec -T integralces pecl install xdebug-3.1.6
  # Configure XDebug
  docker compose exec -T integralces sh -c "echo \"\n[XDebug]\n\
  xdebug.mode = debug\n\
  xdebug.client_host = host.docker.internal\n\
  xdebug.start_with_request = yes\n\
  \" >> /usr/local/etc/php/conf.d/docker-php-ext-xdebug.ini"

  # Enable XDebug
  docker compose exec -T integralces docker-php-ext-enable xdebug
  # Restart web server
  docker compose restart integralces
fi

echo "IntegralCES installed!"
