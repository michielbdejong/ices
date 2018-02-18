#!/bin/bash

## Los datos que necesitaríamos serian:
## - si tiene oferta i/o pedido introducidas.

## Ecoxarxe COOP = 24 / EXEM = 31
ecoxarxa=$1

csvfile="/tmp/listado_cuentas_ices-${ecoxarxa}-`date +'%s'`.csv"
DIR_SCRIPTS="$(dirname "$BASH_SOURCE")/"

SQL="
(SELECT 

	'_Cuenta',
	'Balance',
	'Usuario',
  'e-mail',
  'Estado cuenta',
  'Tipo cuenta',
  'Estado usuario',
  'Last Login',
  'Last Transaction',
  'Nombre',
  'Apellido',
  'Tel. Casa',
  'Tel. Movil',
  'Tel. Trabajo',
  'CP',
  'Población',
  'Ofertas',
  'Demandas'

	) UNION  (

`cat ${DIR_SCRIPTS}listado_cuentas_ices.sql | sed "s/__ECOXARXA__/$ecoxarxa/g"`

INTO OUTFILE '$csvfile'
FIELDS TERMINATED BY '\t'
LINES TERMINATED BY '\r\n'

)
ORDER BY _Cuenta ASC
;"

echo
echo -e "$SQL"
echo

mysql integralcesservidor -e "$SQL"

if [ $? == 0 ] ; then

  echo
  echo Informe generado: $csvfile
  echo

  xdg-open "$csvfile" &

else

  echo
  echo Error al ejecutar SQL.
  echo
  exit 1

fi
