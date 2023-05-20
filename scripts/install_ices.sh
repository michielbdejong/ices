#!/bin/bash

# Configuración
#
# Puedes guardar la configuración en tu directorio personal 
# ~/puesta_punto_drupal.conf o definirla aquí directamente.

file_config="${HOME}/.puesta_punto_drupal.conf"

if [ -f "$file_config" ] ; then
  source "$file_config"
else
  NOMBRE_PROYECTO=ices
  USUARIO_MYSQL=root
  PASS_MYSQL=''
  USUARIO_DRUPAL=''
  USUARIO_PROYECTO=''
  PASS_PROYECTO=''
  EMAIL=""
  DIR_SERVIDOR=''
  USUARIO_APACHE=www-data
fi

if [ -z $USUARIO_MYSQL    ] || \
   [ -z $PASS_MYSQL       ] || \
   [ -z $USUARIO_DRUPAL   ] || \
   [ -z $USUARIO_PROYECTO ] || \
   [ -z $PASS_PROYECTO    ] || \
   [ -z $EMAIL            ] || \
   [ -z $DIR_SERVIDOR     ] ; then

  echo
  echo Falta la configuración, mira el código fuente para realizarla
  echo
  exit

fi


if [ -z $PASS_MYSQL ] ; then

   echo
   echo Es necesario configurar el script para ejecutarlo
   echo
   exit

fi

echo
echo Cambiamos a directorio del servidor
echo -----------------------------------
echo
cd "$DIR_SERVIDOR"
[[ $? != 0 ]] && exit;


echo
echo Bajamos drupal
echo --------------
echo
drush -v dl drupal --drupal-project-rename=$NOMBRE_PROYECTO
[[ $? != 0 ]] && exit;

echo
echo Cambiamos a directorio de proyecto
echo ----------------------------------
echo
cd $NOMBRE_PROYECTO
[[ $? != 0 ]] && exit;

echo
echo Bajamos archivos de idioma ca, es
echo ---------------------------------
echo
wget http://ftp.drupal.org/files/translations/7.x/drupal/drupal-7.x.ca.po
wget http://ftp.drupal.org/files/translations/7.x/drupal/drupal-7.x.es.po

echo
echo Movemos archivos ubicación final
echo --------------------------------
echo
mv  *.po profiles/standard/translations/
[[ $? != 0 ]] && exit;

echo
echo Instalamos drupal standar
echo -------------------------
echo
drush -v --yes site-install standard --account-name=$USUARIO_PROYECTO --account-pass=$PASS_PROYECTO --account-mail=$EMAIL \
  --db-url=mysql://$USUARIO_MYSQL:$PASS_MYSQL@localhost/$NOMBRE_PROYECTO --locale=ca
[[ $? != 0 ]] && exit;

echo
echo Bajamos librerías necesarias
echo ----------------------------

[[ -d sites/all/libraries ]] || mkdir sites/all/libraries
cd sites/all/libraries 

# wget --no-check-certificate http://bitbucket.org/cleonello/jqplot/get/1.0.8.zip -O jqplot.zip
wget --no-check-certificate https://bitbucket.org/cleonello/jqplot/downloads/jquery.jqplot.1.0.8r1250.zip -O jqplot.zip
unzip jqplot.zip
mv dist jquery.jqplot
rm jqplot.zip
git clone --branch master https://github.com/bshaffer/oauth2-server-php.git
cd oauth2-server-php
git checkout v1.7.0
cd ..

cd ../../../

echo
echo Cambiamos permisos en sites/default/files
echo -----------------------------------------
echo
sudo chown -R $USER:$USUARIO_APACHE sites/default/files
sudo chmod -R 775 sites/default/files
[[ $? != 0 ]] && exit;

echo
echo Nos movemos al directorio de los temas
echo --------------------------------------
echo
cd sites/all/themes/
[[ $? != 0 ]] && exit;

echo
echo Bajamos tema greences para desarrollo
echo -------------------------------------
echo
git clone --branch 7.x-1.x $USUARIO_DRUPAL@git.drupal.org:sandbox/esteve/1866046.git greences
[[ $? != 0 ]] && exit;

echo
echo Vamos a directorio proyecto
echo ---------------------------
echo
cd ../../../
[[ $? != 0 ]] && exit;

echo
echo Activamos greences
echo ------------------
echo
drush -v --yes en greences
[[ $? != 0 ]] && exit;

echo
echo Greences como tema por defecto
echo ------------------------------
echo
drush vset theme_default greences
[[ $? != 0 ]] && exit;

echo
echo Cambiamos a directorio de módulos
echo ---------------------------------
echo
cd sites/all/modules/
[[ $? != 0 ]] && exit;

echo
echo Bajamos proyecto para desarrollo
echo --------------------------------
echo
git clone --branch 7.x-1.x $USUARIO_DRUPAL@git.drupal.org:project/ices.git
[[ $? != 0 ]] && exit;

echo
echo Cambiamos a directorio del módulo
echo ---------------------------------
echo
cd ices
[[ $? != 0 ]] && exit;

echo
echo Configuramos git
echo ----------------
echo
git config user.email "$EMAIL"
git config user.name "$USUARIO_DRUPAL"
[[ $? != 0 ]] && exit;

echo
echo Cambiamos a directorio de proyecto
echo ----------------------------------
echo
cd ../../../../
[[ $? != 0 ]] && exit;

echo
echo Activamos módulos ICES
echo ----------------------
echo
modulos="$(for m in `find sites/all/modules/ -name '*.module'` ; do basename $m .module ; done)"
drush -y en ces_bank ces_message ces_develop ces_import4ces ces_statistics ces_summaryblock
drush -y en ${modulos[*]}
[[ $? != 0 ]] && exit;

echo
echo Activamos módulo simpletest
echo ---------------------------
echo
drush -v --yes en simpletest
[[ $? != 0 ]] && exit;

echo
echo Añadimos datos demo a la aplicación
echo -----------------------------------
echo
drush -v php-script sites/all/modules/ices/ces_develop/demo.php
[[ $? != 0 ]] && exit;


echo
echo Módulos desarrollo
echo ------------------
echo

read -p "Deseas instalar los módulos para desarrollo (s/n): " OPCION

if [ "$OPCION" = "s" ] ; then
  drush --yes dl coder devel admin_menu advanced_help potx module_filter coffee
  drush --yes en coder coder_review devel devel_generate admin_devel admin_menu_toolbar \
    admin_menu advanced_help views_ui potx module_filter coffee maillog
  drush --yes dis toolbar
  ## Maillog nos permite ver la salida de los correos electrónicos enviados sin hacerlo.
  ## De esta manera evitamos accidentes con los envíos masivos a usuarios de 
  ## prueba y poder depurar los correos.
  drush vset maillog_send 0
fi

echo
echo ==================
echo Proceso completado
echo ==================
echo
echo
echo Pagina web del proyecto: 
echo
echo http://localhost/$NOMBRE_PROYECTO/
echo
echo Ejecución de test:
echo
echo http://localhost/$NOMBRE_PROYECTO/admin/config/development/testing
echo 

## Otras opciones

# Los test desde drush dan errores que desde web no salen

# echo
# echo Ejecutamos test para CES
# echo ------------------------
# echo
# drush --uri=http://localhost/$NOMBRE_PROYECTO test-run --xml ICES
# [[ $? != 0 ]] && exit;

# echo
# echo Borramos cache
# echo --------------
# echo
# drush cc all
# [[ $? != 0 ]] && exit;

