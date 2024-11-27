<?php

use Neomerx\JsonApi\Contracts\Schema\ContextInterface;
use Neomerx\JsonApi\Schema\BaseSchema;

/**
 * Currency settings are not implemented in the IntegralCES version of the
 * accounting API, but in the Komunitin version. We define here a static set of
 * IntegralCES-compatible settings so the app does not try to call not
 * implemented actions.
 */
class CurrencySettings {
  public $currency;
  function __construct(Currency $currency) {
    $this->currency = $currency;
  }

}

class CurrencySettingsSchema extends BaseSchema {
  public function getType(): string {
    return 'currency-settings';
  }

  public function getId($currencySettings): ?string
  {
    assert($currencySettings instanceof CurrencySettings);
    return (string) $currencySettings->currency->id;
  }

  /**
   * @param CurrencySettings $currencySettings
   */
  public function getAttributes($currencySettings, ContextInterface $context): iterable
  {
    assert($currencySettings instanceof CurrencySettings);
    return [
      'defaultInitialCreditLimit' => 0,
      'defaultInitialMaximumBalance' => false,
      'defaultAllowPayments' => true,
      'defaultAllowPaymentRequests' => true,
      'defaultAcceptPaymentsAutomatically' => true,
      'defaultAcceptPaymentsWhitelist' => [],
      'defaultAllowSimplePayments' => false,
      'defaultAllowSimplePaymentRequests' => true,
      'defaultAllowQrPayments' => false,
      'defaultAllowQrPaymentRequests' => false,
      'defaultAllowMultiplePayments' => false,
      'defaultAllowMultiplePaymentRequests' => false,
      'defaultAllowTagPayments' => false,
      'defaultAllowTagPaymentRequests' => false,
      'defaultAcceptPaymentsAfter' => 0,
      'defaultOnPaymentCreditLimit' => 0,
      'enableExternalPayments' => false,
      'enableExternalPaymentRequests' => false,
      'defaultAllowExternalPayments' => false,
      'defaultAllowExternalPaymentRequests' => false,
      'defaultAcceptExternalPaymentsAutomatically' => false,
      'externalTraderCreditLimit' => 0,
      'externalTraderMaximumBalance' => 0
    ];
  }

  public function getRelationships($currencySettings, ContextInterface $context): iterable
  {
    assert($currencySettings instanceof CurrencySettings);
    return [];
  }
}
