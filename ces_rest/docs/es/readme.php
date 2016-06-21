<?php
/**
@file
Documentación de ces_rest.

@defgroup ces_rest Ces Rest
@ingroup ices

@todo Paginación para listado de transacciones.
@todo Filtrar listado de transacciones por fechas, usuario, ...

@{


## ICES API Admin.

Ces Rest es una API REST que nos permite administrar una ecored desde otra 
aplicación.

Para utilizar el servicio de un cliente debe ser concedida con un client_id y
client_key que debe ser proporcionada por el administrador del sistema de la
instancia ICES. Por favor póngase en contacto con http://www.integralces.net
webmaster@integralces.net.

El cliente que conecte con la API deberá hacerlo con las credenciales de un 
administrador de una ecored.

Nota:

> Para que funcione los usuarios autentificados deben tener permisos para utilizar  
> oauth2 server.


### Recursos

El servicio de los recursos es un simple servicio web JSON para acceder a
algunos de los objetos en el ICES. La autorización de este servicio es
[OAuth2](http://oauth.net/2/).

El criterio de valoración es la autorización

https://[site name]/oauth2/authorize

Se requiere que el atributo scope y debe ajustarse a los recursos. El punto final token es

https://[site name]/oauth2/token

Hay cuatro recursos disponibles:

- https://[site name]/cesrest/users/X
- https://[site name]/cesrest/accounts/X
- https://[site name]/cesrest/transactions/X
- https://[site name]/cesrest/exchange/X

X: Es el código de la ecored, ejemplo: EXEM.

#### users

https://[site name]/cesrest/users/

Acciones posibles:

- GET: [exchange_code]

  Devuelve objeto JSON con el listado de todos los usuarios de la red.

- GET: [exchange_code]/[uid]

  Devuelve objeto JSON con detalle de usuario.

- POST: create/[exchange_code]

  Crear nuevo usuario.

  Devuelve objeto JSON con la información del nuevo usuario.

  En caso de error un mensaje con el error en formato JSON.

- POST: update/[exchange_code]/[uid]
  
  Modificar usuario.

  Devuelve objeto JSON de uusario con los datos modificados.

  En caso de error un mensaje con el error en formato JSON.

Ejemplo:

  https://[site name]/cesrest/users/NET2/3

  Nos devuelve un objeto JSON con toda la información de perfil del usuario 3


#### accounts

https://[site name]/cesrest/accounts/

Acciones posibles:

- GET: [exchange_code]

  Devuelve objeto JSON con el listado de todos las cuentas de la red.

- GET: [exchange_code]/[account_id]

  Devuelve objeto JSON con detalle de cuenta.

- POST: create/[exchange_code]

  Crear nueva cuenta.

  Devuelve objeto JSON con la información de la nueva cuenta.

  En caso de error un mensaje con el error en formato JSON.

- POST: update/[exchange_code]/[account_id]
  
  Modificar cuenta.

  Devuelve objeto JSON de la cuenta con los datos modificados.

  En caso de error un mensaje con el error en formato JSON.

#### transactions

https://[site name]/cesrest/transactions/

Acciones posibles:

- GET: [exchange_code]

  Devuelve objeto JSON con el listado de todas las transacciones de la red.

- GET: [exchange_code]/[transaction_id]

  Devuelve objeto JSON con detalle de la transacción.

Para generar transacciones se utilizara la API CES Interop.

#### exchange

https://[site name]/cesrest/exchange/

Acciones posibles:

- GET: [exchange_code]

  Devuelve objeto JSON con la información de la ecored.

@section example_client Cliente de ejemplo.

Salida de la ayuda de test_rest.sh

@code
$ ./ces_rest/scripts/test_rest.sh -h

Uso: test_rest.sh 

Options:

-d                     Debuger.
-l [option]            TRUE/FALSE/LOGOUT Default FALSE.
-at [token]            Authentication oauth2 token.
-ar [token]            Authentication oauth2 refresh.
-h                     This help.
-c [file conf]         File with another configuration.     
-a [action] [params]   Name scope and params.
                       Acciones prefijadas: users/NET1 accounts/NET1 accounts/NET1/NET10001 users/NET1/3 users/create/NET1


Para poder testear con este script es necesario crear un cliente en el servidor
con los datos de la configuración.

http://test.integralces.net/testices/admin/structure/oauth2-servers/manage/cesrest/clients/add

Etiqueta: Test Client
Client ID: test_client
Client secret: test_secret
Redirect URIs: http://localhost/headers.php (Opcional)

Conceder permisos a los usuarios registrados para utilizar oauth2

Use OAuth2 Server 

Uso del script:

Obtener un token del usuario Riemann con la extensión RESTClient
de firefox (www.restclient.net).

Para ello primero hay que logearse en integralces y pedir el token con la 
extensión restclient.

Desde restclient, vamos a Authentication / Oauth2

Rellenamos formulario con los siguientes datos:

- Response type: code
- Client identifier: test_client
- Client secret: test_secret
- Authorization endpoint: http://test.integralces.net/testices/oauth2/authorize
- Token endpoint: http://test.integralces.net/testices/oauth2/token
- Access token request method: POST
- Redirection endpoint: http://localhost/headers.php (Opcional)
- Access token scope: cesrest
- State: xyz

Enviar token al script:

./ces_rest/scripts/test_rest.sh -at [token]

El token será guardado en un archivo temporal.

A partir de aquí se puede hacer pruebas con las diferentes acciones posibles.
@endcode

@}
*/
