<?php

/**
 * @file
 * Spanish new account template.
 *
 * @ingroup ces_message_template
 */

$subject = '[[exchange:shortname]] solicitud de cuenta nueva [account:name]';

$body = 'Una nueva solicitud de cuenta en [exchange:name]. Puedes gestionar la solicitud desde [site:url]/ces/admin/account y activarla o rechazarla.

Detalles de la nueva cuenta:

Exchange: [exchange:code]
Account name: [account:name]
Account type: [account:kind]
User: [account:user:fullname]

--
IntegralCES
[site:mail]
';
