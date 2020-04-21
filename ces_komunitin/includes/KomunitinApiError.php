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

  public static $errors = array(
    self::NOT_IMPLEMENTED => array('Not implemented', 501),
    self::UNAUTHORIZED => array('Unauthorized', 401),
    self::FORBIDDEN => array('Forbidden', 403),
    self::NOT_FOUND => array('Not Found', 404),
    self::METHOD_NOT_ALLOWED => array('Method Not Allowed', 405),
    self::BAD_REQUEST => array('Bad Request', 400),
    self::INTERNAL_SERVER_ERROR => array('Internal Server Error', 500),
    self::DUPLICATED_ID => array('Duplicated identifier', 403),
  );

  protected $code;

  function __construct($code) {
    if (!isset(self::$errors[$code])) {
      throw 'Invalid Komunitin Api Error Code';
    }
    $this->code = $code;
  }

  function getMessage() {
    return self::$errors[$this->code][0];
  }

  function getStatusCode() {
    return self::$errors[$this->code][1];
  }

  function getJsonApiError() {
    return new Error(null, null, null, $this->getStatusCode(), $this->code, $this->getMessage());
  }
}
