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
  public $currency;
  public $settings;

  function __construct($account, Currency $currency) {
    // Identifier.
    $this->id = $account['uuid'];
    // Account number ABCD0123
    $this->code = $account['name'];
    // Balance
    $decimals = $currency->decimals;
    $this->balance = round(pow(10, $decimals) * $account['balance']);

    // Limits. We need to retrieve the info since it doesn't come with account record.
    $bank = new CesBank();
    $limitchain = $bank->getLimitChain($account['limitchain']);
    $this->creditLimit = - 1;
    $this->debitLimit = -1;
    foreach($limitchain['limits'] as $limit) {
      if ($limit['block']) { // Don't take in count soft limits.
        if ($limit['classname'] == 'CesBankAbsoluteDebitLimit') {
          $this->debitLimit = max($this->debitLimit, $limit['value']);
        } else if ($limit['classname'] == 'CesBankAbsoluteCreditLimit') {
          $this->creditLimit = max($this->creditLimit, $limit['value']);
        }
      }
    }

    $this->currency = $currency;
    $this->settings = new AccountSettings($account, $currency);
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
    return [
      'currency' =>  [
        self::RELATIONSHIP_DATA => $account->currency,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ],
      'settings' => [
        self::RELATIONSHIP_DATA => $account->settings,
        self::RELATIONSHIP_LINKS_SELF => false,
        self::RELATIONSHIP_LINKS_RELATED => false
      ]
    ];
  }

  /**
   * @param mixed $resource
   *
   * @return string
   */
  protected function getSelfSubUrl($resource): string
  {
    return '/' . $resource->currency->code . $this->getResourcesSubUrl() . '/' . $resource->code;
  }

}
