<?php

use Neomerx\JsonApi\Schema\Error;

class KomunitinApiError {
  public const NOT_IMPLEMENTED = '1001';
  public const UNAUTHORIZED = '1002';
  public const FORBIDDEN = '1003';
  public const NOT_FOUND = '1004';
  public const METHOD_NOT_ALLOWED = '1005';
  public const BAD_REQUEST = '1006';
  public const INTERNAL_SERVER_ERROR = '1007';
  public const DUPLICATED_ID = '1008';
  public const INVALID_ID = '1009';
  public const BAD_PAYER = '1010';
  public const BAD_PAYEE = '1011';
  public const BAD_AMOUNT = '1012';
  public const BAD_TRANSACTION_STATE = '1013';
  public const TRANSACTION_ERROR = '1014';

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
    self::BAD_TRANSACTION_STATE => array('Invalid transaction state', 400),
    self::TRANSACTION_ERROR => array('Error operating transaction', 400),
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
