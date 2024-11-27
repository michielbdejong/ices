<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

function gcd($a, $b) {
  return ($b == 0) ? $a : gcd($b, $a % $b);
}

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
  public $rate_n;
  public $rate_d;

  // Relationships
  public $settings;


  function __construct($exchange) {
    $this->id = $exchange['uuid_currency'];
    $this->code = $exchange['code'];
    $this->name = $exchange['currencyname'];
    $this->namePlural = $exchange['currenciesname'];
    $this->symbol = $exchange['currencysymbol'];
    $this->decimals = intval($exchange['currencyscale']);
    $this->scale = 6;
    $this->value = intval(round(pow(10, 6) * $exchange['currencyvalue']));

    // This does not work for denominators that are not multiple of a power of 10.
    $d = pow(10, 6);
    $gcd = gcd($this->value, $d);

    $this->rate_n = $this->value / $gcd;
    $this->rate_d = $d / $gcd;

    $this->settings = new CurrencySettings($this);

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
      'rate' => [
        'n' => $currency->rate_n,
        'd' => $currency->rate_d,
      ],
    ];
    return $attributes;
  }

  /**
   * @param Currency $currency
   */
  public function getRelationships($currency, ContextInterface $context): iterable
  {
    assert($currency instanceof Currency);
    return [
      'settings' => [
        self::RELATIONSHIP_DATA => $currency->settings,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ]
    ];
  }
  /**
   * Overwrite self url since it doesn't follow the general schema.
   */
  protected function getSelfSubUrl($resource): string
  {
    return '/' . $resource->code . '/currency';
  }

}
