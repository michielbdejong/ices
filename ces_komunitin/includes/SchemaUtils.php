<?php

class SchemaUtils {
  public static function encodeDate($timestamp) {
    return DateTime::createFromFormat('U', $timestamp)
      ->setTimezone(new \DateTimeZone('UTC'))
      ->format('Y-m-d\TH:i:s\Z');
      // Note that the ATOM constant produces a format that is not allowed by
      // the JSON API parser at notifications service (Go library).
  }
}
