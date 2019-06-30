<?php
/**
 * @file
 * All page callback functions for bank module.
 */

/**
 * @defgroup ces_bank_pages_export Pages from Ces Bank
 * @ingroup ces_bank
 * @{
 * All page callback functions for bank module.
 */

function generate_sql($exchange_id, $csv) {

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
        WHERE id = " . $exchange_id . "
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

        WHERE ca.exchange = " . $exchange_id .

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
        WHERE fdcbe.ces_blog_exchange_value = " . $exchange_id;

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

        WHERE ca.exchange = " . $exchange_id .
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


        WHERE af.exchange = ". $exchange_id . "
            OR at.exchange = " . $exchange_id;

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

    WHERE cow.type = 'offer' AND au.exchange = " . $exchange_id;

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

    WHERE cow.type = 'want' AND au.exchange = " . $exchange_id;

    return $SQLS[$csv];
}

/**
 * List the limitchains for the given exchange.
 */
function ces_bank_export_exchange_page() {

  $page = array();
  $page['table'] = array(
    '#theme' => 'table',
    '#attributes' => array(
      'class' => array('ces-table'),
    ),
    '#header' => array(
      array(
        'data' => t('Export'),
        'field' => 'name',
        'sort' => 'asc',
      ),
    ),
  );
  $rows = array();
  $CSVS = array();
  $CSVS[] = 'settings';
  $CSVS[] = 'users';
  $CSVS[] = 'announce';
  $CSVS[] = 'balances';
  $CSVS[] = 'trades';
  $CSVS[] = 'offers';
  $CSVS[] = 'wants';

  foreach ($CSVS as $CSV ) {

    $row    = array() ;
    if (ces_bank_access(CesBankPermission::PERMISSION_ADMIN, 'exchange', CES_BANK_CURRENT_EXCHANGE)) {
      $row[] = l(t($CSV), 'ces/admin/export/' . $CSV . '/csv');
    }
    $rows[] = $row    ;
  }
  $page['table']['#rows'] = $rows;
  return $page;
}

function ces_bank_export_csv($csv) {

    $exchange = ces_bank_get_current_exchange();
    if ($exchange === FALSE) {
        return array(
            'message' => array(
                '#markup' => '<p>' . t('Invalid current exchange') . '</p>',
            ),
        );
    }

    // Generamos fichero csv.
    csv_file($csv);
}


function csv_file($csv, $enlace = FALSE) {

    $exchange = ces_bank_get_current_exchange();
    if ($exchange === FALSE) {
        return array(
            'message' => array(
                '#markup' => '<p>' . t('Invalid current exchange') . '</p>',
            ),
        );
    }

    $QUERY    = generate_sql($exchange['id'], $csv);
	$FILENAME = "$csv.csv";
	$HEADS    = $QUERY['heads'];
	$SQL      = $QUERY['sql'];

    $USER_DB = $GLOBALS['databases']['default']['default']['username'];
    $PASS_DB = $GLOBALS['databases']['default']['default']['password'];
    $NAME_DB = $GLOBALS['databases']['default']['default']['database'];

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

	$result = $enlace->query($SQL);
	if (!$result) {
		print_r(mysqli_error($enlace));
		exit();
	}
	$num_fields = mysqli_num_fields($result);
	$headers = $HEADS;
	// for ($i = 0; $i < $num_fields; $i++) {
	// 		$headers[] = mysqli_field_name($result , $i);
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

/**
 *  @}
 */
