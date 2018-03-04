<?php

/**
@file
@brief Exportar ecoxarxa.

Ficheros finales:

├── announce.csv
├── balances.csv
├── groups.csv
├── levy.csv
├── offers.csv
├── recommend.csv
├── remote.csv
├── settings.csv
├── settings-old.csv
├── trades.csv
├── users.csv
├── users-old.csv
└── wants.csv
 */

$ECOXARXA_ID=31;
$USER_DB="root";
$PASS_DB="kkkk";
$NAME_DB="integralcesservidor";

$DEBUG=True;

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
==> announce.csv <==
ID,Owner,Title,Description,DateAdded, DateEvent,DateExpiry,Keep

SQL:
SELECT * FROM node JOIN field_data_ces_blog_exchange fdcbe ON fdcbe.entity_id = node.nid WHERE fdcbe.ces_blog_exchange_value = 31 
*/

$SQLS['announce']['heads'] = array(
	'ID','Owner','Title','Description','DateAdded', 'DateEvent','DateExpiry','Keep'
);
$SQLS['announce']['sql'] = "SELECT
		node.nid AS 'ID',
		node.uid AS 'Owner',
		node.title AS 'Title',
		fdb.body_value AS 'Description',
		node.created AS 'DateAdded',
		node.changed AS 'DateEvent',
		node.changed AS 'DateEvent',
		0 AS 'Keep'
	FROM node 
	JOIN field_data_ces_blog_exchange fdcbe ON fdcbe.entity_id = node.nid 
	JOIN field_data_body fdb ON fdb.entity_id = node.nid
	WHERE fdcbe.ces_blog_exchange_value = " . $ECOXARXA_ID;

//
// ==> balances.csv <==
// UID,Sales,Income,Purchases,Expenditure,Levy,Balance
//
// ==> groups.csv <==
// GID,GroupTitle,GroupDescr,GroupCat,GroupStarted,GroupModified,NewsUpdated,GroupAccess,GroupOwner,GroupImage,GroupNews,GroupAccount,MemCount,Address1,Address2,Address3,Postcode,Phone,Email,IM,Website,AllMail,AllPhotos,EnableDiscuss,EnableNotice,Active
//
// ==> levy.csv <==
// Revenue,LevyCount,Transferred,TransferCount,Balance
//
// ==> offers.csv <==
// ID,UID,Remote,Category,Subcat,Title,Description,Image,Keys,Rate,ConRate,DateAdded,DateExpires,Hidden
//
// ==> recommend.csv <==
// ID,Recommender,Recommendee,Title,Recommendation,DateAdded,DateEdited,DateExpiry
//
// ==> remote.csv <==
// ID,UID,RXID,Category,Subcategory,Title,Description,Rate,ConRate, DateAdded,DateExpires,Hide
//
// ==> settings.csv <==
// ExchangeID,ExchangeTitle,ExchangeName,ExchangeType,ExchangeDescr,Password,Town,Logo,Administrator,Addr1,Addr2,Addr3,Postcode,Province,CountryCode,CountryName,Tel1,Tel2,Fax,TelCode,Email,InternetMessaging,AdminTel,AdminEmail,MemSec,MemSecEmail,MemSecEmailAlt,MemSecPsw,MemSecTel,LevyRate,CurName,CurNamePlural,CurLet,ConCurName,ConCurLet,MapAddress,WebAddress,ReDir,Hidden,Active,TimeBased,TimeUnit,DateAdded,DateModified,CredLim,DebLim,TimeDiff,DaylightSavingOn,DaylightSavingOff,Language,DefaultExchanges,Cell,SubscriptionExchange,WelcomeLetter,InviteLetter,InviteLetterHead,DoMoney,ConRedeemRate,HidePsw,NoDetails,BudRate,,
//
// ==> settings-old.csv <==
// ExchangeID,ExchangeTitle,ExchangeName,ExchangeType,ExchangeDescr,Password,Town,Logo,Administrator,Addr1,Addr2,Addr3,Postcode,Province,CountryCode,CountryName,Tel1,Tel2,Fax,TelCode,Email,InternetMessaging,AdminTel,AdminEmail,MemSec,MemSecEmail,MemSecEmailAlt,MemSecPsw,MemSecTel,LevyRate,CurName,CurNamePlural,CurLet,ConCurName,ConCurLet,MapAddress,WebAddress,ReDir,Hidden,Active,TimeBased,TimeUnit,DateAdded,DateModified,CredLim,DebLim,TimeDiff,DaylightSavingOn,DaylightSavingOff,Language,DefaultExchanges,Cell,SubscriptionExchange,WelcomeLetter,InviteLetter,InviteLetterHead,DoMoney,ConRedeemRate,HidePsw,NoDetails,BudRate
//
// ==> trades.csv <==
// ID,Seller,Buyer,RemoteExchange,RemoteBuyer,RecordID,DateEntered,EnteredBy,Amount,Levy,LevyRate,Description
//
// ==> users.csv <==
// UID,Password,UserType,Firstname,Surname,OrgName,Address1,Address2,Address3,Postcode,SubArea,DefaultSub,PhoneH,PhoneW,PhoneF,PhoneM,Email,IM,WebSite,DOB,NoEmail1,NoEmail2,NoEmail3,NoEmail4,Hidden,Created,LastAccess,LastEdited,EditedBy,InvNo,OrdNo,Coord,CredLimit,DebLimit,LocalOnly,Notes,Lang,Photo,HideAddr1,HideAddr2,HideAddr3,HideArea,HideCode,HidePhoneH,HidePhoneW,HidePhoneF,HidePhoneM,HideEmail,IdNo,LoginCount,SubsDue,Closed,DateClosed,Translate,Locked,Buddy
//
// ==> users-old.csv <==
// UID,Password,UserType,Firstname,Surname,OrgName,Address1,Address2,Address3,Postcode,SubArea,DefaultSub,PhoneH,PhoneW,PhoneF,PhoneM,Email,IM,WebSite,DOB,NoEmail1,NoEmail2,NoEmail3,NoEmail4,Hidden,Created,LastAccess,LastEdited,EditedBy,InvNo,OrdNo,Coord,CredLimit,DebLimit,LocalOnly,Notes,Lang,Photo,HideAddr1,HideAddr2,HideAddr3,HideArea,HideCode,HidePhoneH,HidePhoneW,HidePhoneF,HidePhoneM,HideEmail,IdNo,LoginCount,SubsDue,Closed,DateClosed,Translate,Locked,Buddy
//
// ==> wants.csv <==
// ID,UID,Keep,DateAdded,Title,Description

foreach ($SQLS as $CSV => $SQL) {

	$FILENAME = "$CSV.csv";
	$HEADS = $SQL['heads'];
	$SQL = $SQL['sql'];

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

	$result = $enlace->query($SQL);
	if (!$result) die('Couldn\'t fetch records');
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

mysqli_close($enlace);
