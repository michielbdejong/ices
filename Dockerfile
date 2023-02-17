FROM ubuntu:bionic as integralces-demo

# Define env variables to avoid interactive console prompts in 
# apt install of php tzdata module.
ENV TZ Europe/Madrid
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
  git curl unzip \
  apache2 \
  mysql-server \
  php libapache2-mod-php php-cli php-mbstring php-mysql php-gd php-curl php-ssh2 php-xml php-imap php-xdebug

# Configure apache: enable mod_rewrite and change port from 80 to 2029.
# We ned to change the port so fomr inise the container the url localhost:2029 is accessible 
# and hence drupal can access himself.
RUN a2enmod rewrite && \
  sed -i '/<Directory \/var\/www\/>/,/<\/Directory>/ s/AllowOverride None/AllowOverride All/' /etc/apache2/apache2.conf && \
  sed -i 's/Listen 80/Listen 2029/' /etc/apache2/ports.conf && \
  sed -i 's/<VirtualHost \*:80>/<VirtualHost \*:2029>/' /etc/apache2/sites-available/000-default.conf

# Install composer and drush
RUN curl -sS https://getcomposer.org/installer | php -- --install-dir=/usr/local/bin --filename=composer
RUN composer global require drush/drush:8
ENV PATH="/root/.composer/vendor/bin:${PATH}"

# Create database "ices" with user and passowrd "ices".
RUN service mysql start && sleep 2 && \
  mysql -u root -e "create database ices;\ngrant usage on *.* to ices@localhost identified by 'ices';\ngrant all privileges on ices.* to ices@localhost;\n"

# Install Drupal 7
WORKDIR /var/www/

RUN rm -Rf html && drush dl drupal-7 --drupal-project-rename && mv drupal html
WORKDIR /var/www/html

RUN service mysql start && sleep 2 && \
  drush si standard --db-url=mysql://ices:ices@localhost/ices --account-name=admin --account-pass=adminpass --site-name=IntegralCES -y
RUN chown -R www-data sites/default/files

# Copy ices from workspace.
COPY . sites/all/modules/ices

# Download theme
RUN git clone --branch 7.x-1.x https://git.drupalcode.org/sandbox/esteve-1866046.git sites/all/themes/greences

# download libraries.
RUN git clone --branch main --depth 1 https://github.com/bshaffer/oauth2-server-php.git sites/all/libraries/oauth2-server-php && \
  git clone --branch master  --depth 1 https://github.com/neomerx/json-api.git sites/all/libraries/json-api && \
  git clone --branch master  --depth 1 https://github.com/thephpleague/html-to-markdown.git sites/all/libraries/html-to-markdown

# Install modules and theme
RUN service mysql start && sleep 2 && \
  drush dl -y services && \
  drush en -y \
  oauth2_server services image views \
  token libraries services_views \
  cors login_emailusername smtp bounce && \
  drush en -y ices ces_bank ces_blog ces_interop ces_message ces_offerswants ces_qr ces_rest ces_statistics ces_summaryblock ces_user ces_komunitin \
  greences && \
  drush vset theme_default greences && \
  drush role-add-perm 'anonymous user' 'use oauth2 server' && \
  drush role-add-perm 'authenticated user' 'use oauth2 server' && \
  drush ev "variable_set('cors_domains', array('*'=>'<mirror>|GET,POST,OPTIONS|Content-Type,Authorization|true'));" && \
  service mysql stop



EXPOSE 2029

# Set ices modules dir as a volume for development
VOLUME /var/www/html/sites/all/modules/ices

CMD service mysql start && /usr/sbin/apache2ctl -D FOREGROUND

FROM integralces-demo as integralces-test

# Configure xdebug.
RUN printf "\n[XDebug]\n\
xdebug.remote_enable = 1\n\
xdebug.remote_host = host.docker.internal\n\
xdebug.remote_autostart = 1\n\
xdebug.remote_port = 9029\n" >> /etc/php/7.2/apache2/php.ini

# install development modules and add demo data.
RUN service mysql start && sleep 5 && \
  drush dl -y devel && \
  drush en -y devel ces_develop simpletest maillog && \
  drush vset maillog_send 0 && \
  drush php-script sites/all/modules/ices/ces_develop/demo.php

# install phpmyadmin
# RUN apt install -y phpmyadmin php-gettext
