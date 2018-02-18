#!/bin/bash

echo "--"
echo "-- Migrar ecoxarxa de ICES a ICES."
echo "--"

ECOXARXA=$1
DATABASE=${2:-"integralcesservidor"}

echo "--"
echo "-- ECOXARXA: $ECOXARXA / DATABASE: $DATABASE"
echo "--"

if [ "$ECOXARXA" == "" ] ; then

  echo
  echo "Uso: $0 [ID_ECOXARXA] [DATABSE]"
  echo
  exit

fi

mysqldump -w "exchange=$ECOXARXA" \
--no-create-db \
--skip-add-drop-table \
--skip-extended-insert \
--no-create-info \
$DATABASE ces_account
