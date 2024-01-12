# This is a debian with php 8.0 and apache and drupal files.
FROM php:7.4-apache

# Install some basic utilities
RUN apt-get update && apt-get install -y \
  git curl unzip \
  libfreetype6-dev libjpeg-dev libpng-dev libpq-dev libwebp-dev libzip-dev libc-client-dev libkrb5-dev\
  default-mysql-client
  
# Install PHP extensions
RUN docker-php-ext-configure gd \
		--with-freetype \
		--with-jpeg=/usr \
		--with-webp
RUN docker-php-ext-install -j$(nproc) gd opcache pdo_mysql mysqli zip \
  && docker-php-ext-enable mysqli
RUN docker-php-ext-configure imap --with-kerberos --with-imap-ssl && \
    docker-php-ext-install -j$(nproc) imap

# Configure apache: enable mod_rewrite and change port from 80 to 2029.
# We ned to change the port so fomr inise the container the url localhost:2029 is accessible 
# and hence drupal can access himself.
RUN a2enmod rewrite && \
  sed -i '/<Directory \/var\/www\/>/,/<\/Directory>/ s/AllowOverride None/AllowOverride All/' /etc/apache2/apache2.conf && \
  sed -i 's/Listen 80/Listen 2029/' /etc/apache2/ports.conf && \
  sed -i 's/<VirtualHost \*:80>/<VirtualHost \*:2029>/' /etc/apache2/sites-available/000-default.conf

# Install composer
RUN curl -sS https://getcomposer.org/installer | php -- --install-dir=/usr/local/bin --filename=composer
ENV PATH="/root/.composer/vendor/bin:${PATH}"

# Install drush
RUN composer global require drush/drush:^8 && drush version

# Download drupal 7
WORKDIR /var/www/
RUN rm -Rf html && drush dl drupal-7 --drupal-project-rename && mv drupal html
WORKDIR /var/www/html

# Download modules
RUN drush dl -y \
  oauth2_server services image views \
  token libraries services_views \
  cors login_emailusername smtp bounce geolocation \
  ices
# Download libraries
RUN curl https://codeload.github.com/bshaffer/oauth2-server-php/zip/refs/tags/v1.13.0 --output oauth2-server-php.zip && \
  unzip oauth2-server-php.zip && mv oauth2-server-php-1.13.0 sites/all/libraries/oauth2-server-php && rm oauth2-server-php.zip && \
  git clone --branch master  --depth 1 https://github.com/neomerx/json-api.git sites/all/libraries/json-api && \
  git clone --branch master  --depth 1 https://github.com/thephpleague/html-to-markdown.git sites/all/libraries/html-to-markdown

#Download theme
RUN git clone --branch 7.x-1.x https://git.drupalcode.org/sandbox/esteve-1866046.git sites/all/themes/greences

# Set permissions to certain files.
RUN chown www-data:www-data -R sites/default

EXPOSE 2029
