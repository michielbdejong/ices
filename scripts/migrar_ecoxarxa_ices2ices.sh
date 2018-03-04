#!/bin/bash

## @file
## @brief Exportar ecoxarxa.
##
## Ficheros finales:
##
## ├── announce.csv
## ├── balances.csv
## ├── groups.csv
## ├── levy.csv
## ├── offers.csv
## ├── recommend.csv
## ├── remote.csv
## ├── settings.csv
## ├── settings-old.csv
## ├── trades.csv
## ├── users.csv
## ├── users-old.csv
## └── wants.csv
##
## Cabeceras:
##
## ==> announce.csv <==
## ID,Owner,Title,Description,DateAdded, DateEvent,DateExpiry,Keep
##
## SQL:
## SELECT * FROM node JOIN field_data_ces_blog_exchange fdcbe ON fdcbe.entity_id = node.nid WHERE fdcbe.ces_blog_exchange_value = 31 
SQLS['announce']['heads'] = array(
	'ID','Owner','Title','Description','DateAdded', 'DateEvent','DateExpiry','Keep'
);
##
## ==> balances.csv <==
## UID,Sales,Income,Purchases,Expenditure,Levy,Balance
##
## ==> groups.csv <==
## GID,GroupTitle,GroupDescr,GroupCat,GroupStarted,GroupModified,NewsUpdated,GroupAccess,GroupOwner,GroupImage,GroupNews,GroupAccount,MemCount,Address1,Address2,Address3,Postcode,Phone,Email,IM,Website,AllMail,AllPhotos,EnableDiscuss,EnableNotice,Active
##
## ==> levy.csv <==
## Revenue,LevyCount,Transferred,TransferCount,Balance
##
## ==> offers.csv <==
## ID,UID,Remote,Category,Subcat,Title,Description,Image,Keys,Rate,ConRate,DateAdded,DateExpires,Hidden
##
## ==> recommend.csv <==
## ID,Recommender,Recommendee,Title,Recommendation,DateAdded,DateEdited,DateExpiry
##
## ==> remote.csv <==
## ID,UID,RXID,Category,Subcategory,Title,Description,Rate,ConRate, DateAdded,DateExpires,Hide
##
## ==> settings.csv <==
## ExchangeID,ExchangeTitle,ExchangeName,ExchangeType,ExchangeDescr,Password,Town,Logo,Administrator,Addr1,Addr2,Addr3,Postcode,Province,CountryCode,CountryName,Tel1,Tel2,Fax,TelCode,Email,InternetMessaging,AdminTel,AdminEmail,MemSec,MemSecEmail,MemSecEmailAlt,MemSecPsw,MemSecTel,LevyRate,CurName,CurNamePlural,CurLet,ConCurName,ConCurLet,MapAddress,WebAddress,ReDir,Hidden,Active,TimeBased,TimeUnit,DateAdded,DateModified,CredLim,DebLim,TimeDiff,DaylightSavingOn,DaylightSavingOff,Language,DefaultExchanges,Cell,SubscriptionExchange,WelcomeLetter,InviteLetter,InviteLetterHead,DoMoney,ConRedeemRate,HidePsw,NoDetails,BudRate,,
##
## ==> settings-old.csv <==
## ExchangeID,ExchangeTitle,ExchangeName,ExchangeType,ExchangeDescr,Password,Town,Logo,Administrator,Addr1,Addr2,Addr3,Postcode,Province,CountryCode,CountryName,Tel1,Tel2,Fax,TelCode,Email,InternetMessaging,AdminTel,AdminEmail,MemSec,MemSecEmail,MemSecEmailAlt,MemSecPsw,MemSecTel,LevyRate,CurName,CurNamePlural,CurLet,ConCurName,ConCurLet,MapAddress,WebAddress,ReDir,Hidden,Active,TimeBased,TimeUnit,DateAdded,DateModified,CredLim,DebLim,TimeDiff,DaylightSavingOn,DaylightSavingOff,Language,DefaultExchanges,Cell,SubscriptionExchange,WelcomeLetter,InviteLetter,InviteLetterHead,DoMoney,ConRedeemRate,HidePsw,NoDetails,BudRate
##
## ==> trades.csv <==
## ID,Seller,Buyer,RemoteExchange,RemoteBuyer,RecordID,DateEntered,EnteredBy,Amount,Levy,LevyRate,Description
##
## ==> users.csv <==
## UID,Password,UserType,Firstname,Surname,OrgName,Address1,Address2,Address3,Postcode,SubArea,DefaultSub,PhoneH,PhoneW,PhoneF,PhoneM,Email,IM,WebSite,DOB,NoEmail1,NoEmail2,NoEmail3,NoEmail4,Hidden,Created,LastAccess,LastEdited,EditedBy,InvNo,OrdNo,Coord,CredLimit,DebLimit,LocalOnly,Notes,Lang,Photo,HideAddr1,HideAddr2,HideAddr3,HideArea,HideCode,HidePhoneH,HidePhoneW,HidePhoneF,HidePhoneM,HideEmail,IdNo,LoginCount,SubsDue,Closed,DateClosed,Translate,Locked,Buddy
##
## ==> users-old.csv <==
## UID,Password,UserType,Firstname,Surname,OrgName,Address1,Address2,Address3,Postcode,SubArea,DefaultSub,PhoneH,PhoneW,PhoneF,PhoneM,Email,IM,WebSite,DOB,NoEmail1,NoEmail2,NoEmail3,NoEmail4,Hidden,Created,LastAccess,LastEdited,EditedBy,InvNo,OrdNo,Coord,CredLimit,DebLimit,LocalOnly,Notes,Lang,Photo,HideAddr1,HideAddr2,HideAddr3,HideArea,HideCode,HidePhoneH,HidePhoneW,HidePhoneF,HidePhoneM,HideEmail,IdNo,LoginCount,SubsDue,Closed,DateClosed,Translate,Locked,Buddy
##
## ==> wants.csv <==
## ID,UID,Keep,DateAdded,Title,Description

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

## Descartamos mysdump para hacer un sistema igual al CES.

