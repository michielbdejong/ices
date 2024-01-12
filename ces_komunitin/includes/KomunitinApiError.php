<?php

use Neomerx\JsonApi\Schema\Error;

class KomunitinApiError {
  public const NOT_IMPLEMENTED = 'NotImplemented';
  public const UNAUTHORIZED = 'Unauthorized';
  public const FORBIDDEN = 'Forbidden';
  public const NOT_FOUND = 'NotFound';

  public const METHOD_NOT_ALLOWED = 'MethodNotAllowed';
  public const BAD_REQUEST = 'BadRequest';
  public const INTERNAL_SERVER_ERROR = 'InternalServerError';
  public const DUPLICATED_ID = 'DuplicatedId';
  public const INVALID_ID = 'InvalidId';
  public const BAD_PAYER = 'BadPayer';
  public const BAD_PAYEE = 'BadPayee';
  public const BAD_AMOUNT = 'BadAmount';
  public const BAD_TRANSFER_STATE = 'BadTransferState';
  public const TRANSFER_ERROR = 'TransferError';
  public const INVALID_PASSWORD = 'InvalidPassword';

  public static $errors = array(
    self::NOT_IMPLEMENTED => array('Not implemented', 501),
    self::UNAUTHORIZED => array('Unauthorized', 401),
    self::FORBIDDEN => array('Forbidden', 403),
    self::NOT_FOUND => array('Not Found', 404),
    self::METHOD_NOT_ALLOWED => array('Method Not Allowed', 405),
    self::BAD_REQUEST => array('Bad Request', 400),
    self::INTERNAL_SERVER_ERROR => array('Internal Server Error', 500),
    self::DUPLICATED_ID => array('Duplicated identifier', 409),
    self::INVALID_ID => array('Invalid id', 400),
    self::BAD_PAYER => array('Invalid payer account', 400),
    self::BAD_PAYEE => array('Invalid payee account', 400),
    self::BAD_AMOUNT => array('Invalid amount', 400),
    self::BAD_TRANSFER_STATE => array('Invalid transaction state', 400),
    self::TRANSFER_ERROR => array('Error operating transaction', 400),
    self::INVALID_PASSWORD => array('Invalid password', 400),
  );

  // API error code.
  protected $code;

  // Extra info.
  protected $details = null;

  function __construct($code, $details = null) {
    if (!isset(self::$errors[$code])) {
      throw 'Invalid Komunitin Api Error Code';
    }
    $this->code = $code;
    $this->details = $details;
  }

  function getMessage() {
    return self::$errors[$this->code][0];
  }

  function getStatusCode() {
    return self::$errors[$this->code][1];
  }

  function getJsonApiError() {
    return new Error(null, null, null, $this->getStatusCode(), $this->code, $this->getMessage(), $this->details);
  }
}
