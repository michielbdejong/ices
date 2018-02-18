SELECT 

  ca.name
  , FORMAT(ca.balance,2) AS Balance
  , u.name
  , u.mail

  , CASE ca.state
    WHEN 0 THEN 'Oculta'
    WHEN 1 THEN 'Activada'
    WHEN 2 THEN 'Bloqueada'
    WHEN 3 THEN 'Cerrada'
    ELSE ca.state
  END AS 'Estado cuenta'

  , CASE ca.kind
    WHEN 0 THEN 'Individual'
    WHEN 1 THEN 'Shared'
    WHEN 2 THEN 'Organización'
    WHEN 3 THEN 'Compañia'
    WHEN 4 THEN 'Publica'
    WHEN 5 THEN 'Virtual'
    ELSE ca.kind
  END AS 'Tipo cuenta'

  ,IF (u.status=1, 'Activo', 'Bloqueado') AS 'Estado Usuario'
  , IF (u.login, DATE_FORMAT(from_unixtime(u.login), '%Y-%m-%d'), 'NEVER') AS 'Last Login'
  , ( SELECT DATE_FORMAT(from_unixtime(MAX(trans.created)), '%Y-%m-%d') 
    FROM ces_transaction AS trans WHERE 
    ( trans.fromaccount = ca.id OR trans.toaccount = ca.id ) 
    ) AS 'Last Transaction'
  , fdfn.ces_firstname_value AS 'Nombre'
  , fdsn.ces_surname_value AS 'Apellido'
  , fdph.ces_phonehome_value AS 'Tel. Casa'
  , fdpm.ces_phonemobile_value AS 'Tel. Movil'
  , fdpw.ces_phonework_value AS 'Tel. Trabajo'
  , fdcp.ces_postcode_value AS CP
  , fdct.ces_town_value AS 'Población'
  , ( SELECT COUNT(id) FROM ces_offerwant 
    WHERE ces_offerwant.user = cau.user AND ces_offerwant.type = 'offer' ) 
    AS Ofertas
  , ( SELECT COUNT(id) FROM ces_offerwant 
    WHERE ces_offerwant.user = cau.user AND ces_offerwant.type = 'want' ) 
    AS Demandas

FROM users u
LEFT JOIN ces_accountuser cau ON cau.user = u.uid
LEFT JOIN ces_account ca ON ca.id = cau.account
LEFT JOIN field_data_ces_postcode fdcp ON fdcp.entity_id = u.uid
LEFT JOIN field_data_ces_town fdct ON fdct.entity_id = u.uid
LEFT JOIN field_data_ces_firstname fdfn ON fdfn.entity_id = u.uid
LEFT JOIN field_data_ces_surname fdsn ON fdsn.entity_id = u.uid
LEFT JOIN field_data_ces_phonehome fdph ON fdph.entity_id = u.uid
LEFT JOIN field_data_ces_phonemobile fdpm ON fdpm.entity_id = u.uid
LEFT JOIN field_data_ces_phonework fdpw ON fdpw.entity_id = u.uid

WHERE ca.exchange =__ECOXARXA__
-- AND ca.state = 1
AND ca.kind != 5

