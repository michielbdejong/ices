<?php

class SchemaUtils {
  public static function encodeDate($timestamp) {
    return DateTime::createFromFormat('U', $timestamp)->format(DateTime::ATOM);
  }
}
