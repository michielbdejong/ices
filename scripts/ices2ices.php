<?php

/**
@file
@brief Exportar ecoxarxa.

Ficheros finales:

- users
- announcements
- exchanges
- adverts
- transactions
- balances

 */

$ECOXARXA_ID=$_GET['exchange'];
$USER_DB="root";
$PASS_DB="kkkk";
$NAME_DB="integralcesservidor";

$DEBUG=FALSE;

$enlace = mysqli_connect("127.0.0.1", $USER_DB, $PASS_DB, $NAME_DB);

if (!$enlace) {
    echo "Error: No se pudo conectar a MySQL." . PHP_EOL;
    echo "errno de depuración: " . mysqli_connect_errno() . PHP_EOL;
    echo "error de depuración: " . mysqli_connect_error() . PHP_EOL;
    exit;
}

/* cambiar el conjunto de caracteres a utf8 */
if (!$enlace->set_charset("utf8")) {
    printf("Error cargando el conjunto de caracteres utf8: %s\n", $mysqli->error);
    exit();
}

/**
 * settings.csv
 * @todo CredLim, DebLim.
 */
$SQLS['settings']['heads'] = array(
    'ExchangeID', 'ExchangeTitle', 'ExchangeName', 'ExchangeType',
    'ExchangeDescr', 'Password', 'Town', 'Logo', 'Administrator', 'Addr1',
    'Addr2', 'Addr3', 'Postcode', 'Province', 'CountryCode', 'CountryName',
    'Tel1', 'Tel2', 'Fax', 'TelCode', 'Email', 'InternetMessaging', 'AdminTel',
    'AdminEmail', 'MemSec', 'MemSecEmail', 'MemSecEmailAlt', 'MemSecPsw',
    'MemSecTel', 'LevyRate', 'CurName', 'CurNamePlural', 'CurLet',
    'ConCurName', 'ConCurLet', 'MapAddress', 'WebAddress', 'ReDir', 'Hidden',
    'Active', 'TimeBased', 'TimeUnit', 'DateAdded', 'DateModified', 'CredLim',
    'DebLim', 'TimeDiff', 'DaylightSavingOn', 'DaylightSavingOff', 'Language',
    'DefaultExchanges', 'Cell', 'SubscriptionExchange', 'WelcomeLetter',
    'InviteLetter', 'InviteLetterHead', 'DoMoney', 'ConRedeemRate', 'HidePsw',
    'NoDetails', 'BudRate'
    );

$SQLS['settings']['sql'] = "SELECT
    id AS 'ExchangeID',
    shortname AS 'ExchangeTitle',
    name AS 'ExchangeName',
    '' AS 'ExchangeType',
    '' AS 'ExchangeDescr',
    '' AS 'Password',
    town AS 'Town',
    '' AS 'Logo',
    admin AS 'Administrator',
    region AS 'Addr1',
    '' AS 'Addr2',
    '' AS 'Addr3',
    '' AS 'Postcode',
    '' AS 'Province',
    country AS 'CountryCode',
    '' AS 'CountryName',
    '' AS 'Tel1',
    '' AS 'Tel2',
    '' AS 'Fax',
    '' AS 'TelCode',
    '' AS 'Email',
    '' AS 'InternetMessaging',
    '' AS 'AdminTel',
    '' AS 'AdminEmail',
    '' AS 'MemSec',
    '' AS 'MemSecEmail',
    '' AS 'MemSecEmailAlt',
    '' AS 'MemSecPsw',
    '' AS 'MemSecTel',
    '' AS 'LevyRate',
    '' AS 'CurName',
    '' AS 'CurNamePlural',
    '' AS 'CurLet',
    '' AS 'ConCurName',
    '' AS 'ConCurLet',
    map AS 'MapAddress',
    website AS 'WebAddress',
    '' AS 'ReDir',
    '' AS 'Hidden',
    state AS 'Active',
    '' AS 'TimeBased',
    '' AS 'TimeUnit',
    created AS 'DateAdded',
    modified AS 'DateModified',
    '' AS 'CredLim',
    '' AS 'DebLim',
    '' AS 'TimeDiff',
    '' AS 'DaylightSavingOn',
    '' AS 'DaylightSavingOff',
    '' AS 'Language',
    '' AS 'DefaultExchanges',
    '' AS 'Cell',
    '' AS 'SubscriptionExchange',
    '' AS 'WelcomeLetter',
    '' AS 'InviteLetter',
    '' AS 'InviteLetterHead',
    '' AS 'DoMoney',
    '' AS 'ConRedeemRate',
    '' AS 'HidePsw',
    '' AS 'NoDetails',
    '' AS 'BudRate'
    FROM ces_exchange
    WHERE id = " . $ECOXARXA_ID . "
    ";

/**
 * users.csv
 *
 * 'CredLimit', // @todo Pendiente.
 * 'DebLimit',  // @todo Pendiente.
 */

$SQLS['users']['heads'] = array(
  'UID', 'Password', 'UserType', 'Firstname', 'Surname', 'OrgName', 'Address1',
  'Address2', 'Address3', 'Postcode', 'SubArea', 'DefaultSub', 'PhoneH',
  'PhoneW', 'PhoneF', 'PhoneM', 'Email', 'IM', 'WebSite', 'DOB', 'NoEmail1',
  'NoEmail2', 'NoEmail3', 'NoEmail4', 'Hidden', 'Created', 'LastAccess',
  'LastEdited', 'EditedBy', 'InvNo', 'OrdNo', 'Coord', 'CredLimit', 'DebLimit',
  'LocalOnly', 'Notes', 'Lang', 'Photo', 'HideAddr1', 'HideAddr2', 'HideAddr3',
  'HideArea', 'HideCode', 'HidePhoneH', 'HidePhoneW', 'HidePhoneF',
  'HidePhoneM', 'HideEmail', 'IdNo', 'LoginCount', 'SubsDue', 'Closed',
  'DateClosed', 'Translate', 'Locked', 'Buddy'
);
$SQLS['users']['sql'] = "SELECT
  ca.name     AS 'UID',
  'pasword'   AS 'Password',
  CASE ca.kind
    WHEN 0 THEN 'adm'
    WHEN 1 THEN 'fam'
    WHEN 2 THEN 'org'
    WHEN 3 THEN 'com'
    WHEN 4 THEN 'pub'
    WHEN 5 THEN 'vir'
    ELSE ca.kind
  END AS 'UserType',
  fdfn.ces_firstname_value AS 'Firstname',
  fdsn.ces_surname_value   AS 'Surname',
  '' AS 'OrgName',
  fdcadd.ces_address_value AS 'Address1',
  fdct.ces_town_value      AS 'Address2',
  ''                       AS 'Address3',
  fdcp.ces_postcode_value  AS 'Postcode',
  '' AS 'SubArea',
  0  AS 'DefaultSub',
  fdph.ces_phonehome_value   AS 'PhoneH',
  fdpw.ces_phonework_value   AS 'PhoneW',
  ''                         AS 'PhoneF',
  fdpm.ces_phonemobile_value AS 'PhoneM',
  u.mail AS 'Email',
  '' AS 'IM',
  '' AS 'WebSite',
  '' AS 'DOB',
  0  AS 'NoEmail1',
  0  AS 'NoEmail2',
  0  AS 'NoEmail3',
  0  AS 'NoEmail4',
  CASE ca.state
    WHEN 0 THEN 1
    WHEN 1 THEN 0
    WHEN 2 THEN 1
    WHEN 3 THEN 1
    ELSE ca.state
  END AS 'Hidden',
  u.created AS 'Created',
  u.login   AS 'LastAccess',
  u.access AS 'LastEdited',
  '' AS 'EditedBy',
  0 AS 'InvNo',
  0 AS 'OrdNo',
  0 AS 'Coord',
  0 AS 'CredLimit',
  0 AS 'DebLimit',
  0 AS 'LocalOnly',
  0 AS 'Notes',
  0 AS 'Lang',
  0 AS 'Photo',
  0 AS 'HideAddr1',
  0 AS 'HideAddr2',
  0 AS 'HideAddr3',
  0 AS 'HideArea',
  0 AS 'HideCode',
  0 AS 'HidePhoneH',
  0 AS 'HidePhoneW',
  0 AS 'HidePhoneF',
  0 AS 'HidePhoneM',
  0 AS 'HideEmail',
  '' AS 'IdNo',
  '' AS 'LoginCount',
  '' AS 'SubsDue',
  CASE ca.state
    WHEN 0 THEN 0
    WHEN 1 THEN 0
    WHEN 2 THEN 0
    WHEN 3 THEN 1
	ELSE ca.state
  END AS 'Closed',
  '' AS 'DateClosed',
  0 AS 'Translate',
  CASE ca.state
    WHEN 0 THEN 0
    WHEN 1 THEN 0
    WHEN 2 THEN 1
    WHEN 3 THEN 0
    ELSE ca.state
  END AS 'Locked',
  '' AS 'Buddy'

  FROM users u
  LEFT JOIN ces_accountuser cau ON cau.user = u.uid
  LEFT JOIN ces_account ca ON ca.id = cau.account
  LEFT JOIN field_data_ces_address fdcadd ON fdcadd.entity_id = u.uid
  LEFT JOIN field_data_ces_postcode fdcp ON fdcp.entity_id = u.uid
  LEFT JOIN field_data_ces_town fdct ON fdct.entity_id = u.uid
  LEFT JOIN field_data_ces_firstname fdfn ON fdfn.entity_id = u.uid
  LEFT JOIN field_data_ces_surname fdsn ON fdsn.entity_id = u.uid
  LEFT JOIN field_data_ces_phonehome fdph ON fdph.entity_id = u.uid
  LEFT JOIN field_data_ces_phonemobile fdpm ON fdpm.entity_id = u.uid
  LEFT JOIN field_data_ces_phonework fdpw ON fdpw.entity_id = u.uid

	WHERE ca.exchange = " . $ECOXARXA_ID .

	" ORDER BY ca.name";

/**
 * announce.csv
 */

$SQLS['announce']['heads'] = array(
	'ID','Owner','Title','Description','DateAdded', 'DateEvent','DateExpiry','Keep'
);
$SQLS['announce']['sql'] = "SELECT
		node.nid       AS 'ID',
		node.uid       AS 'Owner',
		node.title     AS 'Title',
		fdb.body_value AS 'Description',
		node.created   AS 'DateAdded',
		node.changed   AS 'DateEvent',
		node.changed   AS 'DateEvent',
		0              AS 'Keep'
	FROM node
	JOIN field_data_ces_blog_exchange fdcbe ON fdcbe.entity_id = node.nid
	JOIN field_data_body fdb ON fdb.entity_id = node.nid
	WHERE fdcbe.ces_blog_exchange_value = " . $ECOXARXA_ID;

/**
 * balances.csv
 * UID,Sales,Income,Purchases,Expenditure,Levy,Balance
 */
$SQLS['balances']['heads'] = array(
 'UID','Sales','Income','Purchases','Expenditure','Levy','Balance'
);
$SQLS['balances']['sql'] = "SELECT

		ca.name AS 'UID',
		( SELECT count(id) 
				FROM ces_transaction WHERE ces_transaction.toaccount = ca.id 
		) AS 'Sales',
		( SELECT SUM(amount) 
				FROM ces_transaction WHERE ces_transaction.toaccount = ca.id 
		) AS 'Income',
		( SELECT count(id) 
			FROM ces_transaction WHERE ces_transaction.fromaccount = ca.id 
		) AS 'Purchases',
		( SELECT SUM(amount) 
			FROM ces_transaction WHERE ces_transaction.fromaccount = ca.id 
		) AS 'Expenditure',
		0 AS 'Levy',
		FORMAT(ca.balance,2) AS 'Balance'

	FROM ces_account ca
	LEFT JOIN ces_accountuser cau ON cau.account = ca.id
	LEFT JOIN users u ON u.uid = cau.user
	LEFT JOIN ces_offerwant o ON o.user = cau.user AND o.type = 'offer'
	LEFT JOIN ces_offerwant w ON w.user = cau.user AND w.type = 'want'

	WHERE ca.exchange = " . $ECOXARXA_ID .
	" ORDER BY ca.name";

/**
 * trades.csv
 * ID,Seller,Buyer,RemoteExchange,RemoteBuyer,RecordID,DateEntered,EnteredBy,Amount,Levy,LevyRate,Description
 */
$SQLS['trades']['heads'] = array(
	'ID','Seller','Buyer','RemoteExchange','RemoteBuyer','RecordID','DateEntered',
	'EnteredBy','Amount','Levy','LevyRate','Description'
);
$SQLS['trades']['sql'] = "
	SELECT
		ct.id AS 'ID',
		af.name AS 'Seller',
		at.name AS 'Buyer',
		'' AS 'RemoteExchange',
		'' AS 'RemoteBuyer',
		'' AS 'RecordID',
		ct.created AS 'DateEntered',
		au.name AS 'EnteredBy',
		ct.amount AS 'Amount',
		'' AS 'Levy',
		'' AS 'LevyRate',
		ct.concept AS 'Description'

	FROM ces_transaction ct
    
	INNER JOIN users u ON ct.user = u.uid
	INNER JOIN ces_accountuser cau ON cau.user = u.uid

	INNER JOIN ces_account af ON af.id = ct.toaccount
	INNER JOIN ces_account at ON at.id = ct.fromaccount
	INNER JOIN ces_account au ON au.id = cau.account


	WHERE af.exchange = ". $ECOXARXA_ID . "
		OR at.exchange = " . $ECOXARXA_ID;

/**
 * offers.csv
 * ID,UID,Remote,Category,Subcat,Title,Description,Image,Keys,Rate,ConRate,DateAdded,DateExpires,Hidden
 */
$SQLS['offers']['heads'] = array(
 'ID','UID','Remote','Category','Subcat','Title','Description','Image','Keys','Rate','ConRate','DateAdded','DateExpires','Hidden'
);
$SQLS['offers']['sql'] = "
SELECT
	cow.id AS 'ID',
	au.name AS 'UID',
	'' AS 'Remote',
	cc.title AS 'Category',
	'' AS 'Subcat',
	cow.title AS 'Title',
	cow.body AS 'Description',
	'' AS 'Image',
	cow.keywords AS 'Keys',
	fdcor.ces_offer_rate_value AS 'Rate',
	'' AS 'ConRate',
	cow.created AS 'DateAdded',
	cow.expire AS 'DateExpires',
    CASE cow.state
    WHEN 0 THEN 1
    WHEN 1 THEN 0
    ELSE 1
  END AS 'Hidden'

FROM ces_offerwant cow
	INNER JOIN users u ON cow.user = u.uid
	INNER JOIN ces_accountuser cau ON cau.user = u.uid
	INNER JOIN ces_account au ON au.id = cau.account
	INNER JOIN ces_category cc ON cc.id = cow.category
	JOIN field_data_ces_offer_rate fdcor ON fdcor.entity_id = cow.id

WHERE cow.type = 'offer' AND au.exchange = " . $ECOXARXA_ID;

/**
 * wants.csv
 * ID,UID,Keep,DateAdded,Title,Description
 */
$SQLS['wants']['heads'] = array(
 'ID','UID','Keep','DateAdded','Title','Description'
);
$SQLS['wants']['sql'] = "
SELECT

	cow.id AS 'ID',
	au.name AS 'UID',
	cow.state AS 'Keep',
	cow.created AS 'DateAdded',
	cow.title AS 'Title',
	cow.body AS 'Description'

FROM ces_offerwant cow
	INNER JOIN users u ON cow.user = u.uid
	INNER JOIN ces_accountuser cau ON cau.user = u.uid
	INNER JOIN ces_account au ON au.id = cau.account

WHERE cow.type = 'want' AND au.exchange = " . $ECOXARXA_ID;



function csv_file($SQLS, $csv, $enlace = FALSE) {

	$FILENAME = "$csv.csv";
	$HEADS = $SQLS[$csv]['heads'];
	$SQL = $SQLS[$csv]['sql'];

	// echo "<p>";
	// echo "FILENAME: $FILENAME";
	// echo "<br/>";
	// echo "HEADS: ";
	// echo "<br/>";
    // print_r($HEADS);
	// echo "<br/>";
	// echo "SQL: ";
	// echo "<br/>";
 	// echo $SQL;
	// echo "</p>";
	// exit();

	$result = $enlace->query($SQL);
	if (!$result) {
		print_r(mysqli_error($enlace));
		exit();
	}
	$num_fields = mysql_num_fields($result);
	$headers = $HEADS;
	// for ($i = 0; $i < $num_fields; $i++) {
	// 		$headers[] = mysql_field_name($result , $i);
	// }
	$fp = fopen('php://output', 'w');
	if ($fp && $result) {
			header('Content-Type: charset=UTF-8; text/csv');
			header('Content-Disposition: attachment; filename="' . $FILENAME. '"');
			header('Pragma: no-cache');
			header('Expires: 0');
			fputcsv($fp, $headers);
			while ($row = $result->fetch_array(MYSQLI_NUM)) {
					fputcsv($fp, array_values($row));
			}
			die;
	}

}

// Seleccionar ecoxarxa.
if ( ! isset($_GET['exchange']) ) {
	$SQL_EXCHANGES="SELECT id, code, state, shortname, name FROM ces_exchange";
	$result = $enlace->query($SQL_EXCHANGES);
	if (!$result) {
		print_r(mysqli_error($enlace));
		exit();
	}
  while ($row=mysqli_fetch_row($result)) {
		printf (
			"<br/><a href='?exchange=%s'>%s</a> (%s) %s %s $s\n",
			$row[0],$row[0],$row[1],$row[2],$row[3],$row[4]
			);
    }
  mysqli_free_result($result);
	exit();
	}


if ( isset($_GET['csv']) ) {
	csv_file($SQLS, $_GET['csv'], $enlace);
	exit();
}

echo '<br/>';
echo 'Ecoxarxa: ' . $ECOXARXA_ID;
echo '<br/>';

$URL_BASE = $_SERVER['SCRIPT_NAME'];
foreach ($SQLS as $CSV => $SQL) {
	$URL = $URL_BASE . '?exchange=' . $ECOXARXA_ID . '&csv=' . $CSV;
	echo '<br/><a href="' . $URL . '">' . $URL . '</a>';
	if ( $DEBUG ) {
		echo "<pre>";
		print_r($SQL['sql']);
		echo "<pre/>";
	}
}



mysqli_close($enlace);
