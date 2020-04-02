FROM ubuntu:latest

# Define env variables to avoid interactive console prompts in 
# apt install of php tzdata module.
ENV TZ Europe/Madrid
ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y \
  git curl unzip \
  apache2 \
  mysql-server \
  php libapache2-mod-php php-cli php-mbstring php-mysql php-gd php-curl php-ssh2 php-xml php-xdebug

# Configure apache: enable mod_rewrite.
RUN a2enmod rewrite && \
  sed -i '/<Directory \/var\/www\/>/,/<\/Directory>/ s/AllowOverride None/AllowOverride All/' /etc/apache2/apache2.conf

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

# Download theme
RUN git clone --branch 7.x-1.x https://git.drupalcode.org/sandbox/esteve-1866046.git sites/all/themes/greences

# Install modules, theme and add demo data.
RUN service mysql start && sleep 2 && \
  drush dl -y ices services && \
  drush en -y \
  oauth2_server services image views \
  token libraries services_views \
  cors login_emailusername \
  ices ces_bank ces_blog ces_interop ces_message ces_offerswants ces_qr ces_rest ces_statistics ces_summaryblock \
  greences && \
  drush vset theme_default greences

# install development modules
RUN service mysql start && sleep 2 && \
  drush dl -y devel && \
  drush en -y devel ces_develop simpletest maillog && \
  drush vset maillog_send 0 && \
  drush php-script sites/all/modules/ices/ces_develop/demo.php

EXPOSE 80

# Set ices modules dir as a volume for development
VOLUME /var/www/html/sites/all/modules/ices

CMD service mysql start && /usr/sbin/apache2ctl -D FOREGROUND


