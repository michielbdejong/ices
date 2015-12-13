<?php

/**
 * @file
 * Catalan alert offerwant expired template.
 *
 * @ingroup ces_message_template
 */

$subject = '[[exchange:shortname]] La teva oferta/demanda ha caducat: [offer:title]';

$body = 'Benvolgut/da [offer:username],

La teva oferta/demanda ha caducat:
[offer:title]

Per a actualitzar-la, pots visitar:
[site:url]ces/bank/account/my[offer:type]s/[offer:id]/edit

Bons intercanvis!

[exchange:shortname] administrador:
[exchange:admin:fullname]
[exchange:admin:mail]
[exchange:admin:mainphone]
';
