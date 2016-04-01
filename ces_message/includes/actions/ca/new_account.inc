<?php

/**
 * @file
 * Catalan new account template.
 *
 * @ingroup ces_message_template
 */

$subject = '[[exchange:shortname]] New account request [account:name]';

$body = 'Una nova sol·licitud de compte en [exchange:name]. Pots gestionar la sol·licitud anant a [site:url]/ces/admin/account i activar-la o rebutjar-la.

Detalls del nou compte:

Exchange: [exchange:code]
Account name: [account:name]
Account type: [account:kind]
User: [account:user:fullname]

--
IntegralCES
[site:mail]
';
