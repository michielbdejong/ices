<?php

/**
 * @file
 * English alert offerwant expired template.
 *
 * @ingroup ces_message_template
 */

$subject = '[[exchange:shortname]] Your [offer:type] is expired: [offer:title]';

$body = 'Dear [offer:username],

Your [offer:type] is expired:
[offer:title]

To update your [offer:type], please visit:
[site:url]ces/bank/account/my[offer:type]s/[offer:id]/edit

Happy trading!

[exchange:shortname] administrator:
[exchange:admin:fullname]
[exchange:admin:mail]
[exchange:admin:mainphone]
';
