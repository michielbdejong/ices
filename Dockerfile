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
RUN docker-php-ext-install -j$(nproc) gd opcache pdo_mysql zip
RUN docker-php-ext-configure imap --with-kerberos --with-imap-ssl && \
    docker-php-ext-install -j$(nproc) imap

# Enable rewrite apache module
RUN a2enmod rewrite

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
RUN git clone --branch main --depth 1 https://github.com/bshaffer/oauth2-server-php.git sites/all/libraries/oauth2-server-php && \
  git clone --branch master  --depth 1 https://github.com/neomerx/json-api.git sites/all/libraries/json-api && \
  git clone --branch master  --depth 1 https://github.com/thephpleague/html-to-markdown.git sites/all/libraries/html-to-markdown

#Download theme
RUN git clone --branch 7.x-1.x https://git.drupalcode.org/sandbox/esteve-1866046.git sites/all/themes/greences

# Set permissions to certain files.
RUN chown www-data:www-data -R sites/default
