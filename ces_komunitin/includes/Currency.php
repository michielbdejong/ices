<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

class Currency {
  public const CODE_TYPE_CEN = "CEN";

  public $id;

  // Attributes
  public $code;
  public $name;
  public $namePlural;
  public $symbol;
  public $decimals;
  public $scale;
  public $value;


  function __construct($exchange) {
    $this->id = $exchange['uuid_currency'];
    $this->code = $exchange['code'];
    $this->name = $exchange['currencyname'];
    $this->namePlural = $exchange['currenciesname'];
    $this->symbol = $exchange['currencysymbol'];
    $this->decimals = $exchange['currencyscale'];
    $this->scale = $exchange['currencyscale'];
    $this->value = round(pow(10, 6) * $exchange['currencyvalue']);
  }
}

class CurrencySchema extends BaseSchema {
  public function getType(): string {
    return 'currencies';
  }

  public function getId($currency): ?string
  {
    assert($currency instanceof Currency);
    return (string) $currency->id;
  }

  /**
   * @param Currency $currency
   */
  public function getAttributes($currency, ContextInterface $context): iterable
  {
    assert($currency instanceof Currency);
    $attributes = [
      'codeType'  => Currency::CODE_TYPE_CEN,
      'code' => $currency->code,
      'name' => $currency->name,
      'namePlural' => $currency->namePlural,
      'symbol' => $currency->symbol,
      'decimals'  => $currency->decimals,
      'scale' => $currency->scale,
      'value' => $currency->value,
    ];
    return $attributes;
  }

  /**
   * @param Currency $currency
   */
  public function getRelationships($currency, ContextInterface $context): iterable
  {
    assert($currency instanceof Currency);
    return [];
  }
  /**
   * Overwrite self url since it doesn't follow the general schema.
   */
  protected function getSelfSubUrl($resource): string
  {
    return '/' . $resource->code . '/currency';
  }

}
