<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

class Account {
  public $id;

  // Attributes
  public $code;
  public $balance;
  public $creditLimit;
  public $debitLimit;

  // Relatinships
  //public $currency;

  function __construct($account, $exchange) {
    // Identifier.
    $this->id = $account['uuid'];
    // Account number ABCD0123
    $this->code = $account['name'];
    // Balance
    $decimals = $exchange['currencyscale'];
    $this->balance = round(pow(10, $decimals) * $account['balance']);
    // Limits
    $this->creditLimit = - 1;
    $this->debitLimit = -1;
    foreach($account['limits'] as $limit) {
      if ($limit['block']) { // Don't take in count soft limits.
        if ($limit['classname'] == 'CesBankAbsoluteDebitLimit') {
          $this->debitLimit = max($this->debitLimit, $limit['value']);
        } else if ($limit['classname'] == 'CesBankAbsoluteCreditLimit') {
          $this->creditLimit = max($this->creditLimit, $limit['value']);
        }
      }
    }
  }
}
class AccountSchema extends BaseSchema {
  public function getType(): string {
    return 'accounts';
  }

  public function getId($account): ?string
  {
    assert($account instanceof Account);
    return (string) $account->id;
  }

  /**
   * @param Account $account
   */
  public function getAttributes($account, ContextInterface $context): iterable {
    assert($account instanceof Account);
    $attributes = [
      'code' => $account->code,
      'balance' => $account->balance,
      'creditLimit' => $account->creditLimit,
      'debitLimit' => $account->debitLimit
    ];
    return $attributes;
  }

  public function getRelationships($account, ContextInterface $context): iterable
  {
    assert($account instanceof Account);
    return [];
  }

  /**
   * @param mixed $resource
   *
   * @return string
   */
  protected function getSelfSubUrl($resource): string
  {
    return $this->getResourcesSubUrl() . '/' . $resource->code;
  }

}
