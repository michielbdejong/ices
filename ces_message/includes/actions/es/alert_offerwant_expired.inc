<?php

/**
 * @file
 * Spanish alert offerwant expired template.
 *
 * @ingroup ces_message_template
 */

$subject = '[[exchange:shortname]] Tu oferta/demanda ha caducado: [offer:title]';

$body = 'Estimado/a [offer:username],

Tu oferta/demanda ha caducado:
[offer:title]

Para actualizarla, puedes visitar:
[site:url]ces/bank/account/my[offer:type]s/[offer:id]/edit

Feliz intercambio!

[exchange:shortname] administrador:
[exchange:admin:fullname]
[exchange:admin:mail]
[exchange:admin:mainphone]
';
