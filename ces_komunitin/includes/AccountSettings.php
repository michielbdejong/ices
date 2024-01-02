<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

/**
 * Settings associated to the Account.
 */
class AccountSettings {
  public $id;
  public $settings;

  public $accountCode;
  public $currency;

  function __construct($account, Currency $currency) {
    // We're reusing the same UUID as the account just to put something, but the
    // account settings should never be used independently of their account.
    $this->id = $account['uuid'];
    $this->settings = [
      'acceptPaymentsAutomatically' => isset($account['data']['accept']['manual']) ? !$account['data']['accept']['manual'] : TRUE,
    ];
    $this->accountCode = $account['name'];
    $this->currency = $currency;
  }
}

class AccountSettingsSchema extends BaseSchema {

  public function getType(): string {
    return 'account-settings';
  }

  public function getId($settings): ?string {
    assert($settings instanceof AccountSettings);
    return (string) $settings->id;
  }

  public function getAttributes($settings, ContextInterface $context): iterable {
    assert($settings instanceof AccountSettings);
    return $settings->settings;
  }

  public function getRelationships($settings, ContextInterface $context): iterable {
    assert($settings instanceof AccountSettings);
    return [];
  }

  protected function getSelfSubUrl($settings): string
  {
    return '/' . $settings->currency->code . '/accounts/' . $settings->accountCode . '/settings';
  }
}
